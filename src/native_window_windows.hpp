/**************************************************************************/
/*  native_window_windows.hpp                                             */
/*                                                                        */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include "native_window_base.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace godot {

class NativeWindow : public NativeWindowBase {
	typedef NativeWindowBase Super;

public:
	NativeWindow(AcrylicWindow* acrylic_window);

public:
	bool is_valid() const;

public:
	void on_ready();
	void on_exit_tree();

public:
	bool minimize();
	bool maximize(bool toggle);
	bool close();

public:
	bool set_always_on_top(const bool p_always_on_top);
	bool set_frame(const AcrylicWindow::Frame p_frame);
	bool set_backdrop(const AcrylicWindow::Backdrop p_backdrop);
	bool set_corner(const AcrylicWindow::Corner p_corner);
	bool set_autohide_title_bar(const AcrylicWindow::Autohide p_autohide_title_bar);
	bool set_accent_title_bar(const AcrylicWindow::Accent p_accent_title_bar);
	bool set_auto_colors(const bool p_auto_colors);
	bool set_base_color(const Color& p_base_color);
	bool set_border_color(const Color& p_border_color);
	bool set_title_bar_color(const Color& p_title_bar_color);
	bool set_text_color(const Color& p_text_color);
	bool set_clear_color(const Color& p_clear_color);

protected:
	HWND hwnd;
};

} // namespace godot

#endif // _WIN32
