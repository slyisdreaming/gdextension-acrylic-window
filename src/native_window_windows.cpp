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

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/window.hpp>

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

		if (window->get_frame() != AcrylicWindow::FRAME_CUSTOM)
			return CallWindowProc(godot_wndproc, hwnd, uMsg, wParam, lParam);

		// If custom frame then call Godot's wndproc at the very end because
		// otherwise it messes up window.
#if DEBUG_DWM
		static long dy = 0;
#endif
		switch (uMsg) {
		case WM_NCACTIVATE:
			return 0;

		case WM_SYSKEYUP: {
			if (wParam == 0x0D) { // ENTER
				if (lParam & (1 << 24) || lParam & (1 << 29)) { // ALT
					window->maximize();
					return 0;
				}
			}
		} break;

		case WM_KEYUP: {
			// Use magical numbers because Godot overrides key constants on Windows.
			switch (wParam) {
			case 0x7A: // F11
				window->maximize();
				return 0;
#if DEBUG_DWM
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
		} break;

		case WM_NCCALCSIZE: {
			if (wParam) {
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
				return 0;
			}
		} break;

		case WM_NCHITTEST: {
			LRESULT result = DefWindowProc(hwnd, uMsg, wParam, lParam);
			if (result != HTCLIENT)
				return result;

			RECT border = {};
			if (!get_window_border(hwnd, &border)) {
				print_debug("Failed to get window border (WM_NCHITTEST). Using default border.");
				border = { -10, 5, 5, 5 };
			}

			POINT pt = {
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam)
			};

			ScreenToClient(hwnd, &pt);

#if DEBUG_DWM
			print_debug("%d %d %d", pt.x, pt.y, border.top);
#endif
			if (pt.y < -border.top)
				return HTTOP;

			bool can_drag = window->get_drag_by_content();
			if (!can_drag) {
				border = {};
				AdjustWindowRectEx(&border, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_CAPTION, FALSE, 0);
				can_drag = pt.y < -border.top;
			}

			if (can_drag) {
				Window* root_window = window->get_window();
				if (!root_window) {
					print_error("Failed to get root window.");
					return HTCAPTION;
				}

				Control* blocking_control = find_mouse_blocking_control(root_window);
				if (!blocking_control)
					return HTCAPTION;

				// print_debug("%s", blocking_control->get_name().c_escape().ascii().get_data());
			}
		} break;
		} // switch(uMsg)

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
