/**************************************************************************/
/*  acrylic_window.hpp                                                    */
/*  Acrylic and Mica styles for apps built in Godot.                      */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#pragma once

#include "helpers.hpp"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/property_tweener.hpp>

namespace godot {

class ColorRect;
//class Tween;

class AcrylicWindow : public Control {
	GDCLASS(AcrylicWindow, Control)

	friend class NativeWindow;

public:
	enum Frame {
		FRAME_DEFAULT,
		FRAME_BORDERLESS,
		FRAME_CUSTOM
	};

	enum Backdrop {
		BACKDROP_SOLID,
		BACKDROP_TRANSPARENT,
		BACKDROP_ACRYLIC,
		BACKDROP_MICA,		
		BACKDROP_TABBED
	};

	enum Corner {
		CORNER_DEFAULT,
		CORNER_DONT_ROUND,
		CORNER_ROUND,
		CORNER_ROUND_SMALL
	};

	enum Autohide {
		AUTOHIDE_NEVER,
		AUTOHIDE_ALWAYS,
		AUTOHIDE_MAXIMIZED
	};

	enum Accent {
		ACCENT_NEVER,
		ACCENT_ALWAYS,
		ACCENT_MOUSE_OVER		
	};

public:
	/* EXPERIMENTAL If enabled, allows to modify the editor a bit. */
	DECLARE_PROPERTY(bool, modify_editor, false)

	DECLARE_PROPERTY(float, text_size, 1.25)
	DECLARE_PROPERTY(bool, always_on_top, true)
	DECLARE_PROPERTY(bool, drag_by_content, true)
	DECLARE_PROPERTY(bool, drag_by_right_click, true)
	DECLARE_PROPERTY(float, dim_strength, 0.25)

	DECLARE_PROPERTY(Frame, frame, FRAME_CUSTOM)
	DECLARE_PROPERTY(Backdrop, backdrop, BACKDROP_ACRYLIC)
	DECLARE_PROPERTY(Corner, corner, CORNER_DEFAULT)
	DECLARE_PROPERTY(Autohide, autohide_title_bar, AUTOHIDE_MAXIMIZED)
	DECLARE_PROPERTY(Accent, accent_title_bar, ACCENT_MOUSE_OVER)
	
	// Automatically adjust colors based on the base_color.
	DECLARE_PROPERTY(bool, auto_colors, true)
	DECLARE_PROPERTY(Color&, base_color, Color(0.133, 0.145, 0.149, 0.741))
	DECLARE_PROPERTY(Color&, border_color, Color(0, 0, 0))
	DECLARE_PROPERTY(Color&, title_bar_color, Color(0, 0, 0))
	DECLARE_PROPERTY(Color&, text_color, Color(1, 1, 1))
	DECLARE_PROPERTY(Color&, clear_color, Color(0, 0, 0))

public:
	void minimize();
	void maximize(bool toggle = true);
	void close();
	void dim(bool on);

public:
	virtual void _ready() override;

protected:
	void _notification(int p_what);
	static void _bind_methods();

private:
	void on_draw();
	void on_ready();
	void on_exit_tree();

private:	
	void adjust_colors();
	void apply_style();

private:
	ColorRect* dim_rect;
	Ref<Tween> dim_tween;
};

}

VARIANT_ENUM_CAST(::godot::AcrylicWindow::Frame)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Backdrop)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Corner)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Autohide)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Accent)
