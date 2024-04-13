/**************************************************************************/
/*  native_window_base.cpp                                                */
/*                                                                        */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#include "native_window_base.hpp"

#include "helpers.hpp"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

namespace {
	const char PRINT_CATEGORY[] = "AcrylicWindow";
}

namespace godot {

NativeWindowBase::NativeWindowBase(AcrylicWindow* acrylic_window)
	: acrylic_window(acrylic_window)
{
	window = acrylic_window->get_window();
	if (!window) {
		print_error("Failed to get window.");
		return;
	}
}

bool NativeWindowBase::is_valid() const {
	return window != nullptr;
}

void NativeWindowBase::on_ready()
{}

void NativeWindowBase::on_exit_tree()
{}

bool NativeWindowBase::minimize() {
	window->set_mode(Window::MODE_MINIMIZED);
	return true;
}

bool NativeWindowBase::maximize(bool toggle) {
	window->set_mode(Window::MODE_MINIMIZED);
	return true;
}

// TODO Need to check if this code closes jsut this window or the whole application.
bool NativeWindowBase::close() {
	SceneTree* scene_tree = window->get_tree();
	if (!scene_tree) {
		print_error("Failed to get scene tree.");
		return false;
	}

	scene_tree->quit();

	return true;
}

bool NativeWindowBase::set_text_size(const float p_text_size) {
	window->set_content_scale_factor(p_text_size);
	return true;
}

bool NativeWindowBase::set_always_on_top(const bool p_always_on_top) {
	window->set_flag(Window::FLAG_ALWAYS_ON_TOP, p_always_on_top);
	return true;
}

bool NativeWindowBase::set_drag_by_content(const bool p_drag_by_content) {
	return true;
}

bool NativeWindowBase::set_drag_by_right_click(const bool p_drag_by_right_click) {
	return true;
}

bool NativeWindowBase::set_frame(const AcrylicWindow::Frame p_frame) {
	return true;
}

bool NativeWindowBase::set_backdrop(const AcrylicWindow::Backdrop p_backdrop) {
	window->set_transparent_background(p_backdrop != AcrylicWindow::BACKDROP_SOLID);
	// need to redraw if changed transparency
	acrylic_window->queue_redraw();
	return true;
}

bool NativeWindowBase::set_corner(const AcrylicWindow::Corner p_corner) {
	return true;
}

bool NativeWindowBase::set_autohide_title_bar(const AcrylicWindow::Autohide p_autohide_title_bar) {
	return true;
}

bool NativeWindowBase::set_accent_title_bar(const AcrylicWindow::Accent p_accent_title_bar) {
	return true;
}

bool NativeWindowBase::set_auto_colors(const bool p_auto_colors) {
	return true;
}

bool NativeWindowBase::set_base_color(const Color& p_base_color) {
	return true;
}

bool NativeWindowBase::set_border_color(const Color& p_border_color) {
	return true;
}

bool NativeWindowBase::set_title_bar_color(const Color& p_title_bar_color) {
	return true;
}

bool NativeWindowBase::set_text_color(const Color& p_text_color) {
	return true;
}

bool NativeWindowBase::set_clear_color(const Color& p_clear_color) {
	auto rendering_server = RenderingServer::get_singleton();
	if (!rendering_server) {
		print_error("Failed to get rendering server.");
		return false;
	}

	rendering_server->set_default_clear_color(p_clear_color);

	return true;
}

} // namespace godot