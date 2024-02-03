/**************************************************************************/
/*  acrylic_window.h                                                      */
/*  Acrylic and Mica styles for apps made with Godot.                     */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#pragma once

#include "helpers.h"

#include <godot_cpp/classes/control.hpp>

namespace godot {

class AcrylicWindow : public Control {
	GDCLASS(AcrylicWindow, Control)

public:
	enum Frame {
		FRAME_DEFAULT,
		FRAME_CUSTOM		
	};

	enum Backdrop {
		BACKDROP_SOLID,
		BACKDROP_TRANSPARENT,
		BACKDROP_ACRYLIC,
		BACKDROP_MICA,		
		BACKDROP_TABBED
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
	DECLARE_PROPERTY(float, text_size, 1.25)
	DECLARE_PROPERTY(bool, always_on_top, true)
	DECLARE_PROPERTY(bool, drag_by_content, true)

	/* EXPERIMENTAL If enabled, allows to modify the editor a bit. */
	DECLARE_PROPERTY(bool, modify_editor, false)
	
	DECLARE_PROPERTY(Frame, frame, FRAME_CUSTOM)
	DECLARE_PROPERTY(Backdrop, backdrop, BACKDROP_ACRYLIC)
	DECLARE_PROPERTY(Autohide, autohide_title_bar, AUTOHIDE_MAXIMIZED)
	// Make the title bar color a bit darker than base_color.
	DECLARE_PROPERTY(Accent, accent_title_bar, ACCENT_MOUSE_OVER)

	// Automatically adjust colors based on the base_color.
	DECLARE_PROPERTY(bool, auto_colors, true)

	// Colors
	DECLARE_PROPERTY(Color&, base_color, Color(0.71, 0.188, 0.471, 0.224))
	DECLARE_PROPERTY(Color&, border_color, Color(0, 0, 0))
	DECLARE_PROPERTY(Color&, title_bar_color, Color(0, 0, 0))
	DECLARE_PROPERTY(Color&, text_color, Color(1, 1, 1))
	DECLARE_PROPERTY(Color&, clear_color, Color(0, 0, 0))

public:
	void minimize();
	void maximize(bool toggle = true);
	void close();

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
	void apply_style();
	void update_colors();
	void update_border_color(const Color &p_border_color);
	void update_title_bar_color(const Color &p_caption_color);
	void update_text_color(const Color &p_text_color);
	void update_clear_color(const Color &p_clear_color);
};

}

VARIANT_ENUM_CAST(::godot::AcrylicWindow::Frame)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Backdrop)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Autohide)
VARIANT_ENUM_CAST(::godot::AcrylicWindow::Accent)
