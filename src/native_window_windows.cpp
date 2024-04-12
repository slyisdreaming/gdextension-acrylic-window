/**************************************************************************/
/*  native_window_windows.cpp                                             */
/*                                                                        */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#include "native_window_windows.hpp"

#if defined(_WIN32) || defined(_WIN64)

#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <map>
#include <mutex>

#define WIN32_LEAN_AND_MEAN

#include <dwmapi.h>
#include <Windows.h>
#include <windowsx.h>

#pragma comment(lib, "Dwmapi.lib")

#define DEBUG_DWM FALSE

using namespace godot;

namespace {
	const char PRINT_CATEGORY[] = "AcrylicWindow";

	struct thunk_s {
		AcrylicWindow* window;
		WNDPROC godot_wndproc;
	};

	std::mutex mutex;
	std::map<HWND, thunk_s> windows;

	bool right_click_down = false;
	HWND right_click_hwnd = NULL;
	bool right_click_can_drag = false;
	POINT right_click_pos = {};
	POINT right_click_move = {};

#if DEBUG_DWM
	long dy = 0;
#endif

	HWND get_native_handle(Window* window) {
		if (!window) {
			print_error("Window is null.");
			return NULL;
		}

		int32_t window_id = window->get_window_id();
		if (window_id == DisplayServer::INVALID_WINDOW_ID) {
			print_error("Invalid window id.");
			return NULL;
		}

		DisplayServer* display_server = DisplayServer::get_singleton();
		if (!display_server) {
			print_error("Display Server is null.");
			return NULL;
		}

		int64_t native_handle = display_server->window_get_native_handle(DisplayServer::WINDOW_HANDLE, window_id);

		return reinterpret_cast<HWND>(native_handle);
	}

	bool get_window_border(HWND hwnd, RECT* border) {
		WINDOWPLACEMENT placement = {};
		placement.length = sizeof(WINDOWPLACEMENT);
		if (!GetWindowPlacement(hwnd, &placement)) {
			print_debug("Failed to GetWindowPlacement. Error: %d.", GetLastError());
			return false;
		}

		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (!style) {
			print_debug("Failed to GetWindowLongPtr(GWL_STYLE). Error: %d.", GetLastError());
			return false;
		}

		RECT rect = {};
		if (!AdjustWindowRectEx(&rect, style & ~WS_CAPTION, FALSE, 0)) {
			print_debug("Failed to GetWindowPlacement. Error: %d.", GetLastError());
			return false;
		}

		// Divide top by 5 because Windows returns -10
		// though -2 is enough to hide thick border at the top.
		border->top = placement.showCmd == SW_SHOWMAXIMIZED ? rect.top : (rect.top / 5);
		border->left = rect.left;
		border->right = rect.right;
		border->bottom = rect.bottom;

		return true;
	}

	void on_right_click_down(HWND hwnd, WPARAM wParam, LPARAM lParam, bool screen_space) {
		POINT click_pos = {
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam)
		};

		if (!screen_space) {
			if (!ClientToScreen(hwnd, &click_pos)) {
				print_error("Failed to ClientToScreen. Error: %d.", GetLastError());
				return;
			}
		}

		right_click_down = true;
		right_click_hwnd = hwnd;
		right_click_can_drag = false;
		right_click_pos = click_pos;
		right_click_move = {};
	}

	void on_right_click_up(HWND hwnd) {
		if (right_click_hwnd != hwnd)
			return;

		right_click_down = false;
		right_click_hwnd = NULL;
		right_click_can_drag = false;
	}

	bool on_syskey_up(WPARAM wParam, LPARAM lParam, AcrylicWindow* window) {
		// Use magical numbers because Godot overrides key constants on Windows.
		if (wParam == 0x0D) { // ENTER
			if ((lParam & (1 << 24)) || (lParam & (1 << 29))) { // ALT
				window->maximize();
				return true;
			}
		}

		return false;
	}

	bool on_key_up(HWND hwnd, WPARAM wParam, LPARAM lParam, AcrylicWindow* window) {
		// Use magical numbers because Godot overrides key constants on Windows.
		switch (wParam) {
		case 0x7A: // F11
			window->maximize();
			return true;

#if 0//DEBUG_DWM
		case 0x26: // Arrow Up
			dy += 1;
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			break;

		case 0x28: // Arrow Down
			dy -= 1;
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			break;
#endif
		}

		return false;
	}

	bool on_ncactivate(WPARAM wParam, AcrylicWindow* window) {
		window->dim(wParam == FALSE);
		return true;
	}

	bool on_nccalcsize(AcrylicWindow::Frame frame, HWND hwnd, WPARAM wParam, LPARAM lParam) {
		if (wParam == FALSE)
			return false;

		if (frame == AcrylicWindow::FRAME_DEFAULT)
			return false;

		if (frame == AcrylicWindow::FRAME_BORDERLESS) {
			// remove a thin resize border at the top
			NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;
			sz->rgrc[0].top -= 2;
			return true;
		}
		
		RECT border = {};
		if (!get_window_border(hwnd, &border)) {
			print_debug("Failed to get window border (WM_NCCALCSIZE). Using default border.");
			border = { 5, 5, 5, 5 };
		}

		NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;

		sz->rgrc[0].top -= border.top;
		sz->rgrc[0].left -= border.left;
		sz->rgrc[0].right -= border.right;
		sz->rgrc[0].bottom -= border.bottom;

#if DEBUG_DWM
		print_debug("%d %d %d %d %d", dy, border.top, border.left, border.right, border.bottom);
#endif

		return true;
	}

	LRESULT on_nchittest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, AcrylicWindow* window) {
		LRESULT result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (result != HTCLIENT)
			return result;

		RECT border = {};
		if (!get_window_border(hwnd, &border)) {
			print_debug("Failed to get window border (WM_NCHITTEST). Using default border.");
			border = { -10, 5, 5, 5 };
		}

		POINT screen_cursor = {
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam)
		};

		POINT client_cursor = screen_cursor;
		if (!ScreenToClient(hwnd, &client_cursor)) {
			print_debug("Failed to ScreenToClient. Error: %d.", GetLastError());
			return HTCLIENT;
		}

#if DEBUG_DWM
		print_debug("%d %d %d", client_cursor.x, client_cursor.y, border.top);
#endif

		if (client_cursor.y < -border.top)
			return HTTOP;

		if (right_click_down && right_click_hwnd == hwnd && window->get_drag_by_right_click()) {
			if (!(GetAsyncKeyState(0x02) & (1 << 15))) {
				on_right_click_up(hwnd);
			}
			else {
				POINT delta = {
					screen_cursor.x - right_click_pos.x,
					screen_cursor.y - right_click_pos.y,
				};

				if (!right_click_can_drag) {
					right_click_move.x += std::abs(delta.x);
					right_click_move.y += std::abs(delta.y);

					right_click_can_drag = right_click_move.x > 10 || right_click_move.y > 10;
				}

				right_click_pos.x = screen_cursor.x;
				right_click_pos.y = screen_cursor.y;

				if (right_click_can_drag) {
					RECT rect = {};
					if (!GetWindowRect(hwnd, &rect)) {
						print_debug("Failed to GetWindowRect. Error: %d.", GetLastError());
					}
					else {
						if (!SetWindowPos(hwnd, NULL, rect.left + delta.x, rect.top + delta.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER))
							print_debug("Failed to SetWindowPos. Error: %d.", GetLastError());
						return HTCAPTION;
					}
				}
			}
		}

		bool can_drag = window->get_drag_by_content();
		if (!can_drag) {
			border = {};
			if (!AdjustWindowRectEx(&border, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_CAPTION, FALSE, 0)) {
				print_debug("Failed to AdjustWindowRectEx. Error: %d.", GetLastError());
				can_drag = false;
			}
			else {
				can_drag = client_cursor.y < -border.top;
			}
		}

		if (can_drag) {
			Window* root_window = window->get_window();
			if (!root_window) {
				print_error("Failed to get root window.");
				return HTCAPTION;
			}

			// Need this to close popups correctly.
			if (has_popup(root_window))
				return HTCLIENT;

			Control* blocking_control = find_mouse_blocking_control(root_window);
			if (!blocking_control)
				return HTCAPTION;
			
			// Need this to show Aero Snap Layouts.
			// TODO glitchy
			// if (blocking_control->has_meta("maximize_button"))
			//	return HTMAXBUTTON;

			// print_debug("%s", blocking_control->get_name().c_escape().ascii().get_data());
		}

		return HTCLIENT;
	}

	LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		mutex.lock();
		auto thunk = windows.find(hwnd);
		mutex.unlock();

		if (thunk == windows.end()) {
			print_warning("Failed to find window by native handle.");
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		AcrylicWindow* window = thunk->second.window;
		WNDPROC godot_wndproc = thunk->second.godot_wndproc;

		//if (window->get_frame() != AcrylicWindow::FRAME_CUSTOM)
		//	return CallWindowProc(godot_wndproc, hwnd, uMsg, wParam, lParam);

		// If custom frame then call Godot's wndproc at the very end because
		// otherwise it messes up window.

		switch (uMsg) {
		case WM_NCACTIVATE:
			if (on_ncactivate(wParam, window))
				return 0;
			break;

		case WM_NCCALCSIZE:
			if (on_nccalcsize(window->get_frame(), hwnd, wParam, lParam))
				return 0;
			break;

		case WM_NCHITTEST: {
			LRESULT result = on_nchittest(hwnd, uMsg, wParam, lParam, window);
			if (result != HTCLIENT)
				return result;
		} break;
		
		case WM_KEYUP:
			if (on_key_up(hwnd, wParam, lParam, window))
				return 0;
			break;

		case WM_SYSKEYUP:
			if (on_syskey_up(wParam, lParam, window))
				return 0;
			break;

		case WM_RBUTTONDOWN:
			on_right_click_down(hwnd, wParam, lParam, false);
			break;

		case WM_NCRBUTTONDOWN:
			on_right_click_down(hwnd, wParam, lParam, true);
			break;

		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP:
			on_right_click_up(hwnd);
			break;
		}

		return CallWindowProc(godot_wndproc, hwnd, uMsg, wParam, lParam);
	}

	bool subclass_wndproc(AcrylicWindow* acrylic_window, HWND hwnd) {
		print_debug("HWND = %lld", hwnd);

		std::lock_guard<std::mutex> guard(mutex);

		auto godot_wndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&wndproc);
		if (!godot_wndproc) {
			print_error("Failed to SetWindowLongPtr(GWLP_WNDPROC). Error: %d.", GetLastError());
			return false;
		}

		windows.insert({ hwnd, { acrylic_window, godot_wndproc } });

		return true;
	}

	bool restore_wndproc(HWND hwnd) {
		print_debug("HWND = %lld", hwnd);

		WNDPROC godot_wndproc = nullptr; {
			std::lock_guard<std::mutex> guard(mutex);

			auto thunk = windows.find(hwnd);
			if (thunk == windows.end()) {
				print_error("Failed to find window by native handle.");
				return false;
			}

			godot_wndproc = thunk->second.godot_wndproc;
			windows.erase(hwnd);
		}

		if (!SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)godot_wndproc)) {
			print_error("Failed to SetWindowLongPtr(GWLP_WNDPROC). Error: %d.", GetLastError());
			return false;
		}

		return true;
	}

	bool set_color(HWND hwnd, DWMWINDOWATTRIBUTE color_attribute, const Color& color) {
		COLORREF new_color = color.to_abgr32() & 0x00ffffff; // remove alpha to make DwmSetWindowAttribute happy
		HRESULT hresult = DwmSetWindowAttribute(hwnd, color_attribute, &new_color, sizeof(new_color));
		if (FAILED(hresult)) {
			print_error("Failed to set color attribute = %d. Error: %d.", color_attribute, hresult);
			return false;
		}

		return true;
	}
}

namespace godot {

NativeWindow::NativeWindow(AcrylicWindow* acrylic_window)
	: Super(acrylic_window)
	, hwnd(NULL)
{
	if (!window)
		return;

	hwnd = ::get_native_handle(window);
	if (hwnd == NULL) {
		print_error("Failed to get native handle.");
		return;
	}
}

bool NativeWindow::is_valid() const {
	return hwnd != NULL;
}

void NativeWindow::on_ready() {
	if (!::subclass_wndproc(acrylic_window, hwnd))
		print_error("Failed to subclass wndproc.");
}

void NativeWindow::on_exit_tree() {
	if (!::restore_wndproc(hwnd))
		print_error("Failed to restore wndproc.");
}

bool NativeWindow::minimize() {
	if (!ShowWindow(hwnd, SW_MINIMIZE)) {
		print_error("Failed to ShowWindow. Error: %d.", GetLastError());
		return false;
	}

	return true;
}

bool NativeWindow::maximize(bool toggle) {
	int show_command = SW_MAXIMIZE;
	if (toggle) {
		WINDOWPLACEMENT placement = {};
		placement.length = sizeof(WINDOWPLACEMENT);
		if (!GetWindowPlacement(hwnd, &placement)) {
			print_error("Failed to GetWindowPlacement. Error: %d.", GetLastError());
			return false;
		}

		if (placement.showCmd == SW_SHOWMAXIMIZED)
			show_command = SW_RESTORE;
	}

	if (!ShowWindow(hwnd, show_command)) {
		print_error("Failed to ShowWindow. Error: %d.", GetLastError());
		return false;
	}

	return true;
}

bool NativeWindow::close() {
	if (!PostMessage(hwnd, WM_CLOSE, 0, 0)) {
		print_error("Failed to PostMessage(WM_CLOSE). Error: %d.", GetLastError());
		return false;
	}

	return true;
}

bool NativeWindow::set_always_on_top(const bool p_always_on_top) {
	if (!SetWindowPos(hwnd, p_always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE)) {
		print_error("Failed to SetWindowPos. Error: %d", GetLastError());
		return false;
	}

	return true;
}

bool NativeWindow::set_frame(const AcrylicWindow::Frame p_frame) {
	print_debug("New Frame: %d", p_frame);
#if TRUE
	if (!SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE)) {
		print_warning("Failed to notify SWP_FRAMECHANGED. Error: %d.", GetLastError());
		return false;
	}
#else
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (!style) {
		print_error("Failed to get GWL_STYLE. Error: %d", GetLastError());
		return false;
	}

	bool apply = false;

	switch (frame) {
	case AcrylicWindow::FRAME_CUSTOM: {
		//NOTE WS_CAPTION is required for animations when minimizing or maximizing window.
		if (style & WS_CAPTION) {
			style &= ~WS_CAPTION;
			apply = true;
		}

		//if (style & WS_THICKFRAME) {
		//	style &= ~WS_THICKFRAME;
		//	apply = true;
		//}

		//SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(BLACK_BRUSH));
		//MARGINS margins = { 0 };
		//DwmExtendFrameIntoClientArea(hwnd, &margins);
		//SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	} break;

	case AcrylicWindow::FRAME_DEFAULT: {
		if (!(style & WS_CAPTION)) {
			style |= WS_CAPTION;
			apply = true;
		}

		if (!(style & WS_THICKFRAME)) {
			style |= WS_THICKFRAME;
			apply = true;
		}
	} break;
	}

	if (apply) {
		if (!SetWindowLongPtr(hwnd, GWL_STYLE, style)) {
			print_error("Failed to set the new style. Error: %d", GetLastError());
			return false;
		}

		if (!SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE))
			print_warning("Failed to notify SWP_FRAMECHANGED. Error: %d", GetLastError());
	}
	else {
		print_debug("The new frame is the same as the old one. No changes made.");
	}
#endif
	return true;
}

bool NativeWindow::set_backdrop(const AcrylicWindow::Backdrop p_backdrop) {
	bool apply = false;

	int new_backdrop = DWMSBT_AUTO;
	switch (p_backdrop) {
	case AcrylicWindow::BACKDROP_SOLID:
		new_backdrop = DWMSBT_AUTO;
		break;
	case AcrylicWindow::BACKDROP_TRANSPARENT:
		new_backdrop = DWMSBT_NONE;
		break;
	case AcrylicWindow::BACKDROP_ACRYLIC:
		new_backdrop = DWMSBT_TRANSIENTWINDOW;
		break;
	case AcrylicWindow::BACKDROP_MICA:
		new_backdrop = DWMSBT_MAINWINDOW;
		break;
	case AcrylicWindow::BACKDROP_TABBED:
		new_backdrop = DWMSBT_TABBEDWINDOW;
		break;
	}

	int old_backdrop = DWMSBT_AUTO;
	HRESULT hresult = DwmGetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &old_backdrop, sizeof(old_backdrop));
	if (FAILED(hresult)) {
		print_error("Failed to get DWMWA_SYSTEMBACKDROP_TYPE. Error: %d.", hresult);
		apply = true;
	}
	else {
		apply = new_backdrop != old_backdrop;
	}

	if (apply) {
		print_debug("Setting the new backdrop %d. The current backdrop is %d.", new_backdrop, old_backdrop);

		hresult = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &new_backdrop, sizeof(new_backdrop));
		if (FAILED(hresult)) {
			print_error("Failed to set DWMWA_SYSTEMBACKDROP_TYPE. Error: %d.", hresult);
			return false;
		}
	}
	else {
		print_debug("The new backdrop is the same as the old one. No changes made.");
	}

	return Super::set_backdrop(p_backdrop);
}

bool NativeWindow::set_autohide_title_bar(const AcrylicWindow::Autohide p_autohide_title_bar) {
	return true;
}

bool NativeWindow::set_accent_title_bar(const AcrylicWindow::Accent p_accent_title_bar) {
	return true;
}

bool NativeWindow::set_auto_colors(const bool p_auto_colors) {
	return true;
}

bool NativeWindow::set_base_color(const Color& p_base_color) {
	return true;
}

bool NativeWindow::set_border_color(const Color& p_border_color) {
	return ::set_color(hwnd, DWMWA_BORDER_COLOR, p_border_color);
}

bool NativeWindow::set_title_bar_color(const Color& p_title_bar_color) {
	return ::set_color(hwnd, DWMWA_CAPTION_COLOR, p_title_bar_color);
}

bool NativeWindow::set_text_color(const Color& p_text_color) {
	return ::set_color(hwnd, DWMWA_TEXT_COLOR, p_text_color);
}

bool NativeWindow::set_clear_color(const Color& p_clear_color) {
	return Super::set_clear_color(p_clear_color);
}

} // namespace godot

#endif // _WIN32
