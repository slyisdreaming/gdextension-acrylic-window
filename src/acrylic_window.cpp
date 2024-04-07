/**************************************************************************/
/*  acrylic_window.cpp                                                    */
/*  Acrylic and Mica styles for apps built in Godot.                      */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#include "acrylic_window.hpp"

#include "helpers.hpp"
#include "native_window.hpp"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/label.hpp>

#include <godot_cpp/classes/window.hpp>

namespace {
	const char PRINT_CATEGORY[] = "AcrylicWindow";
}

// Check that property has been modified and that node is ready.
#define PROPERTY_GUARD(property_name)					\
	if (property_name == p_##property_name)				\
		return;											\
	if (!is_node_ready()) {								\
		property_name = p_##property_name;				\
		return;											\
	}

// Check that can modify editor.
#define EDITOR_GUARD(property_name)						\
	if (!modify_editor && is_editor()) {				\
		property_name = p_##property_name;				\
		return;											\
	}

#define NATIVE_GUARD									\
	NativeWindow native(this);							\
	if (!native.is_valid()) {							\
		print_error("Failed to init native window.");	\
		return;											\
	}

namespace godot {

void AcrylicWindow::minimize() {
	NATIVE_GUARD;
	native.minimize();
}

void AcrylicWindow::maximize(bool toggle) {
	NATIVE_GUARD;
	native.maximize(toggle);
}

void AcrylicWindow::close() {
	NATIVE_GUARD;
	native.close();
}

void AcrylicWindow::dim(bool on) {
	if (dim_tween.is_valid())
		dim_tween->kill();

	if (!dim_rect) {
		print_error("dim_rect is null");
		return;
	}

	dim_tween = create_tween();

	bool should_dim = on && dim_strength > 0.0001 && !always_on_top;

	if (should_dim) {
		dim_tween->tween_property(dim_rect, "color:a", dim_strength, 0.4);

		NATIVE_GUARD;
		native.set_border_color(border_color.darkened(dim_strength));
	}
	else {
		dim_tween->tween_property(dim_rect, "color:a", 0.0, 0.2);

		NATIVE_GUARD;
		native.set_border_color(border_color);
	}
}

void AcrylicWindow::_ready() {
	// NOTE: This function is called twice in the editor: when opening a scene 
	// in the editor and when loading a scene in a game running in the editor.
	// 	
	// BUG? Godot complains that super._ready is not defined when called from
	// GDScript. That's why use NOTIFICATION_READY instead.
}



void AcrylicWindow::_notification(int p_what) {
	switch (p_what) {
	case NOTIFICATION_DRAW:
		on_draw();
		break;
	case NOTIFICATION_READY:
		on_ready();
		break;
	case NOTIFICATION_EXIT_TREE:
		on_exit_tree();
		break;
	}
}

void AcrylicWindow::_bind_methods() {
	BIND_ENUM_CONSTANT(FRAME_NONE);
	BIND_ENUM_CONSTANT(FRAME_CUSTOM);
	BIND_ENUM_CONSTANT(FRAME_DEFAULT);
	
	BIND_ENUM_CONSTANT(BACKDROP_SOLID);
	BIND_ENUM_CONSTANT(BACKDROP_TRANSPARENT);
	BIND_ENUM_CONSTANT(BACKDROP_ACRYLIC);
	BIND_ENUM_CONSTANT(BACKDROP_MICA);
	BIND_ENUM_CONSTANT(BACKDROP_TABBED);

	BIND_ENUM_CONSTANT(ACCENT_NEVER);
	BIND_ENUM_CONSTANT(ACCENT_ALWAYS);
	BIND_ENUM_CONSTANT(ACCENT_MOUSE_OVER);

	BIND_ENUM_CONSTANT(AUTOHIDE_NEVER);
	BIND_ENUM_CONSTANT(AUTOHIDE_ALWAYS);
	BIND_ENUM_CONSTANT(AUTOHIDE_MAXIMIZED);

	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, modify_editor);

	BIND_PROPERTY(AcrylicWindow, Variant::FLOAT, text_size);
	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, always_on_top);
	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, drag_by_content);
	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, drag_by_right_click);
	BIND_PROPERTY(AcrylicWindow, Variant::FLOAT, dim_strength);
	
	BIND_PROPERTY_ENUM(AcrylicWindow, Variant::INT, frame, "None, Default, Custom");
	BIND_PROPERTY_ENUM(AcrylicWindow, Variant::INT, backdrop, "Solid, Transparent, Acrylic, Mica, Tabbed");
	BIND_PROPERTY_ENUM(AcrylicWindow, Variant::INT, autohide_title_bar, "Never, Always, Maximized");
	BIND_PROPERTY_ENUM(AcrylicWindow, Variant::INT, accent_title_bar, "Never, Always, Mouse Over");

	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, auto_colors);
	BIND_PROPERTY(AcrylicWindow, Variant::COLOR, base_color);
	BIND_PROPERTY(AcrylicWindow, Variant::COLOR, border_color);
	BIND_PROPERTY(AcrylicWindow, Variant::COLOR, title_bar_color);
	BIND_PROPERTY(AcrylicWindow, Variant::COLOR, text_color);
	BIND_PROPERTY(AcrylicWindow, Variant::COLOR, clear_color);

	BIND_FUNCTION(AcrylicWindow, minimize);
	BIND_FUNCTION(AcrylicWindow, maximize);
	BIND_FUNCTION(AcrylicWindow, close);
}

#pragma region CALLBACKS

void AcrylicWindow::on_draw() {
	draw_rect(Rect2(Point2(), get_size()),
		backdrop == BACKDROP_SOLID ? Color(base_color.r, base_color.g, base_color.b) : base_color);

#if DEBUG_DWM
	{
		// Draw client area border.
		auto w = get_size().width;
		auto h = get_size().height;

		draw_rect(Rect2(0, 0, w, 1), Color(1, 0, 0));
		draw_rect(Rect2(w - 1, 0, 1, h), Color(0, 1, 0));
		draw_rect(Rect2(0, h - 1, w, 1), Color(0, 0, 1));
		draw_rect(Rect2(0, 0, 1, h), Color(1, 1, 0));
	}
#endif

}

void AcrylicWindow::on_ready() {
	if (is_editor())
		return;

	// on_ready must be called before apply_style
	// because apply_style relies on the native code.
	NATIVE_GUARD;
	native.on_ready();

	dim_rect = new ColorRect();
	dim_rect->set_name("DimRect");
	dim_rect->set_color(Color(0, 0, 0, 0));
	dim_rect->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	dim_rect->set_z_index(100);
	dim_rect->set_anchor_and_offset(SIDE_LEFT, 0, 0);
	dim_rect->set_anchor_and_offset(SIDE_RIGHT, 1, 0);
	dim_rect->set_anchor_and_offset(SIDE_TOP, 0, 0);
	dim_rect->set_anchor_and_offset(SIDE_BOTTOM, 1, 0);
	add_child(dim_rect);

	//dim(true);

	// Need this to drag by content.
	set_mouse_filter(MOUSE_FILTER_PASS);
	apply_style();
}

void AcrylicWindow::on_exit_tree() {
	if (is_editor())
		return;

	NATIVE_GUARD;
	native.on_exit_tree();
}

#pragma endregion

#pragma region PROPERTIES

DEFINE_PROPERTY_GET(AcrylicWindow, float, text_size)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, always_on_top)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, drag_by_content)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, drag_by_right_click)
DEFINE_PROPERTY_GET(AcrylicWindow, float, dim_strength)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, modify_editor)
DEFINE_PROPERTY_GET(AcrylicWindow, AcrylicWindow::Frame, frame)
DEFINE_PROPERTY_GET(AcrylicWindow, AcrylicWindow::Backdrop, backdrop)
DEFINE_PROPERTY_GET(AcrylicWindow, AcrylicWindow::Autohide, autohide_title_bar)
DEFINE_PROPERTY_GET(AcrylicWindow, AcrylicWindow::Accent, accent_title_bar)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, auto_colors)
DEFINE_PROPERTY_GET(AcrylicWindow, Color, base_color)
DEFINE_PROPERTY_GET(AcrylicWindow, Color, border_color)
DEFINE_PROPERTY_GET(AcrylicWindow, Color, title_bar_color)
DEFINE_PROPERTY_GET(AcrylicWindow, Color, text_color)
DEFINE_PROPERTY_GET(AcrylicWindow, Color, clear_color)

DEFINE_PROPERTY_SET(AcrylicWindow, bool, drag_by_content)
DEFINE_PROPERTY_SET(AcrylicWindow, bool, drag_by_right_click)
DEFINE_PROPERTY_SET(AcrylicWindow, float, dim_strength)

void AcrylicWindow::set_modify_editor(const bool p_modify_editor) {
	PROPERTY_GUARD(modify_editor);
	modify_editor = p_modify_editor;
	if (modify_editor)
		apply_style();
}

void AcrylicWindow::set_text_size(const float p_text_size) {
	PROPERTY_GUARD(text_size);
		
	if (is_editor()) {
		text_size = p_text_size;
		return;
	}

	NATIVE_GUARD;
	if (!native.set_text_size(p_text_size))
		return;

	text_size = p_text_size;
}

void AcrylicWindow::set_always_on_top(const bool p_always_on_top) {
	PROPERTY_GUARD(always_on_top);
	EDITOR_GUARD(always_on_top);

	NATIVE_GUARD;
	if (!native.set_always_on_top(p_always_on_top))
		return;

	always_on_top = p_always_on_top;

	// Can't use this code because it affects all the windows.
	// DisplayServer::get_singleton()->window_set_flag(DisplayServer::WINDOW_FLAG_ALWAYS_ON_TOP, always_on_top);
}

void AcrylicWindow::set_frame(const AcrylicWindow::Frame p_frame) {
	PROPERTY_GUARD(frame);
	EDITOR_GUARD(frame);

	// Need to set frame first because wndproc calls get_frame.
	Frame prev_frame = frame;
	frame = p_frame;

	NATIVE_GUARD;
	if (!native.set_frame(p_frame)) {
		frame = prev_frame;
		return;
	}

	frame = p_frame;
}

void AcrylicWindow::set_backdrop(const AcrylicWindow::Backdrop p_backdrop) {
	PROPERTY_GUARD(backdrop);
	EDITOR_GUARD(backdrop);

	NATIVE_GUARD;
	if (!native.set_backdrop(p_backdrop))
		return;

	backdrop = p_backdrop;
}

void AcrylicWindow::set_autohide_title_bar(const AcrylicWindow::Autohide p_autohide_title_bar) {
	PROPERTY_GUARD(autohide_title_bar);
	autohide_title_bar = p_autohide_title_bar;
}

void AcrylicWindow::set_accent_title_bar(const AcrylicWindow::Accent p_accent_title_bar) {
	PROPERTY_GUARD(accent_title_bar);
	accent_title_bar = p_accent_title_bar;
}

void AcrylicWindow::set_auto_colors(const bool p_auto_colors) {
	PROPERTY_GUARD(auto_colors);		
	auto_colors = p_auto_colors;
	if (auto_colors) {
		adjust_colors();

		EDITOR_GUARD(auto_colors);

		NATIVE_GUARD;
		native.set_base_color(base_color);
		native.set_border_color(border_color);
		native.set_title_bar_color(title_bar_color);
		native.set_text_color(text_color);
		native.set_clear_color(clear_color);
	}
}

void AcrylicWindow::set_base_color(const Color &p_base_color) {
	PROPERTY_GUARD(base_color);

	base_color = p_base_color;
	queue_redraw();

	if (auto_colors) {
		adjust_colors();

		EDITOR_GUARD(base_color);

		NATIVE_GUARD;
		native.set_base_color(base_color);
		native.set_border_color(border_color);
		native.set_title_bar_color(title_bar_color);
		native.set_text_color(text_color);
		native.set_clear_color(clear_color);
	}
}

void AcrylicWindow::set_border_color(const Color &p_border_color) {
	PROPERTY_GUARD(border_color);
	if (auto_colors)
		return;
	
	EDITOR_GUARD(border_color);

	NATIVE_GUARD;
	if (!native.set_border_color(p_border_color))
		return;
	
	border_color = p_border_color;
}

void AcrylicWindow::set_title_bar_color(const Color &p_title_bar_color) {
	PROPERTY_GUARD(title_bar_color);
	if (auto_colors)
		return;
	
	EDITOR_GUARD(title_bar_color);

	NATIVE_GUARD;
	if (!native.set_title_bar_color(p_title_bar_color))
		return;
	
	title_bar_color = p_title_bar_color;
}

void AcrylicWindow::set_text_color(const Color &p_text_color) {
	PROPERTY_GUARD(text_color);
	if (auto_colors)
		return;
	
	EDITOR_GUARD(text_color);

	NATIVE_GUARD;
	if (!native.set_text_color(p_text_color))
		return;

	text_color = p_text_color;
}

void AcrylicWindow::set_clear_color(const Color &p_clear_color) {
	PROPERTY_GUARD(clear_color);
	if (auto_colors)
		return;
	
	EDITOR_GUARD(clear_color);

	NATIVE_GUARD;
	if (!native.set_clear_color(p_clear_color))
		return;
	
	clear_color = p_clear_color;
}

void AcrylicWindow::adjust_colors() {
	float luminance = base_color.get_luminance();
	Color border_tint = Color(luminance, luminance, luminance) * 0.75;
	border_color = base_color.lerp(border_tint, 1 - base_color.a);
	title_bar_color = border_color;
	text_color = luminance < 0.65 ? Color(1, 1, 1) : Color(0, 0, 0);
	clear_color = base_color.darkened(0.85);
}

void AcrylicWindow::apply_style() {
	if (is_editor()) {
		if (!modify_editor)
			return;

		print_warning("The new style will be applied to the Editor. Uncheck Modify Editor to disable this.");
	}

	NATIVE_GUARD;
	native.set_text_size(text_size);
	native.set_always_on_top(always_on_top);
	native.set_backdrop(backdrop);
	native.set_border_color(border_color);
	native.set_title_bar_color(title_bar_color);
	native.set_text_color(text_color);
	native.set_clear_color(clear_color);
	native.set_frame(frame);
}

#pragma endregion

}
