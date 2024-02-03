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

#include "acrylic_window.h"

#include "helpers.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/window.hpp>

#include <map>
#include <mutex>

#include <dwmapi.h>
#include <Windows.h>
#include <windowsx.h>

#pragma comment(lib, "Dwmapi.lib")

using namespace godot;

#define DEBUG_DWM FALSE

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

	LRESULT CALLBACK wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		mutex.lock();
		auto thunk = windows.find(hwnd);
		mutex.unlock();

		LRESULT result = 0;

		if (thunk == windows.end()) {
			print_warning("Failed to find window by native handle.");
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		AcrylicWindow *window = thunk->second.window;
		WNDPROC godot_wndproc = thunk->second.godot_wndproc;

#if DEBUG_DWM
		static long dy = 0;
#endif
		if (window->get_frame() == AcrylicWindow::FRAME_CUSTOM) {
			switch (uMsg) {
			case WM_NCACTIVATE:
				return 0;

			case WM_SYSKEYUP:
				if (wParam == 0x0D) { // ENTER
					if (lParam & (1 << 24) || lParam & (1 << 29)) { // ALT
						window->maximize();
						return 0;
					}
				}
				break;

			case WM_KEYUP: {
				// Use magical numbers because Godot overrides key constants on Windows.
				switch (wParam) {
				case 0x7A: // F11
					window->maximize();
					return 0;
#if DEBUG_DWM
				case 0x26: // Arrow Up
					dy += 1;
					print_debug("%d", dy);
					SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
					break;
				case 0x28: // Arrow Down
					dy -= 1;
					print_debug("%d", dy);
					SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
					break;
#endif
				}
			} break;
			case WM_NCCALCSIZE:
				if (wParam) {
					WINDOWPLACEMENT placement = {};
					placement.length = sizeof(WINDOWPLACEMENT);
					GetWindowPlacement(hwnd, &placement);

					//if (placement.showCmd != SW_SHOWMAXIMIZED)
					{
						RECT border = {};
						AdjustWindowRectEx(&border, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_CAPTION, FALSE, 0);
						NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)lParam;
						sz->rgrc[0].top -= placement.showCmd == SW_SHOWMAXIMIZED ? border.top : (border.top / 5); // returns -10, though -2 is enough to hide thick border at the top
						sz->rgrc[0].left -= border.left;
						sz->rgrc[0].right -= border.right;
						sz->rgrc[0].bottom -= border.bottom;

						//sz->rgrc[0].top -= 5;
						//sz->rgrc[0].left -= 5;
						//sz->rgrc[0].right -= 5;
						//sz->rgrc[0].bottom -= 5;
#if DEBUG_DWM
						print_debug("%d %d %d %d %d", dy, border.top, border.left, border.right, border.bottom);
#endif
					}

					return 0;
				}
				break;

			case WM_NCHITTEST: {
				LRESULT result = DefWindowProc(hwnd, uMsg, wParam, lParam);
				if (result == HTCLIENT) {
					RECT border = {};
					AdjustWindowRectEx(&border, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_CAPTION, FALSE, 0);

					POINT pt = {
						GET_X_LPARAM(lParam),
						GET_Y_LPARAM(lParam)
					};

					ScreenToClient(hwnd, &pt);
#if DEBUG_DWM
					print_debug("%d %d %d", pt.x, pt.y, border.top);
#endif
					if (pt.y < -border.top) {
						return HTTOP;
					}
					else {
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
					}
				}
				return result;
			}
			}
		}
		
		// HACK: Call Godot's wndproc at the end because otherwise it messes up window.
		result = CallWindowProc(godot_wndproc, hwnd, uMsg, wParam, lParam);

		return result;
	}

	bool subclass_wndproc(AcrylicWindow* window) {
		HWND hwnd = ::get_native_handle(window->get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return false;
		}

		print_debug("HWND = %lld", hwnd);

		std::lock_guard<std::mutex> guard(mutex);

		auto godot_wndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&wndproc);
		if (!godot_wndproc) {
			print_error("Failed to SetWindowLongPtr(GWLP_WNDPROC). Error: %d.", GetLastError());
			return false;
		}

		windows.insert({ hwnd, { window, godot_wndproc } });

		return true;
	}

	bool restore_wndproc(AcrylicWindow* window) {
		HWND hwnd = ::get_native_handle(window->get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return false;
		}

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

	void set_frame(HWND hwnd, AcrylicWindow::Frame frame) {
#if TRUE
		if (!SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE))
			print_warning("Failed to notify SWP_FRAMECHANGED. Error: %d.", GetLastError());

#else
		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (!style) {
			print_error("Failed to get GWL_STYLE. Error: %d", GetLastError());
			return;
		}

		bool apply = false;

		switch (frame) {
		case AcrylicWindow::FRAME_CUSTOM:
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
			{
				//MARGINS margins = { 0 };
				//DwmExtendFrameIntoClientArea(hwnd, &margins);
				//SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			}

			break;

		case AcrylicWindow::FRAME_DEFAULT:
			if (!(style & WS_CAPTION)) {
				style |= WS_CAPTION;
				apply = true;
			}

			if (!(style & WS_THICKFRAME)) {
				style |= WS_THICKFRAME;
				apply = true;
			}

			break;
		}

		if (apply) {
			if (!SetWindowLongPtr(hwnd, GWL_STYLE, style))
				print_error("Failed to set the new style. Error: %d", GetLastError());

			if (!SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE))
				print_warning("Failed to notify SWP_FRAMECHANGED. Error: %d", GetLastError());
		}
		else {
			print_debug("The new frame is the same as the old one. No changes made.");
		}
#endif
	}

	void set_backdrop(HWND hwnd, AcrylicWindow::Backdrop backdrop) {
		bool apply = false;

		int new_backdrop = DWMSBT_AUTO;
		switch (backdrop) {
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
			if (FAILED(hresult))
				print_error("Failed to set DWMWA_SYSTEMBACKDROP_TYPE. Error: %d.", hresult);
		}
		else {
			print_debug("The new backdrop is the same as the old one. No changes made.");
		}
	}

	void set_color(HWND hwnd, DWMWINDOWATTRIBUTE color_attribute, const Color& color) {
		COLORREF new_color = color.to_abgr32() & 0x00ffffff; // remove alpha to make DwmSetWindowAttribute happy
		HRESULT hresult = DwmSetWindowAttribute(hwnd, color_attribute, &new_color, sizeof(new_color));
		if (FAILED(hresult))
			print_error("Failed to set color attribute = %d. Error: %d.", color_attribute, hresult);
	}

	bool set_always_on_top(HWND hwnd, bool always_on_top) {
		if (!SetWindowPos(hwnd, always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE)) {
			print_error("Failed to SetWindowPos. Error: %d", GetLastError());
			return false;
		}

		return true;
	}
}

#define EDITOR_GUARD \
	if (!modify_editor && is_editor()) \
		return;

namespace godot {

void AcrylicWindow::minimize() {
	HWND hwnd = ::get_native_handle(get_window());
	if (hwnd == NULL) {
		print_error("Failed to get native handle.");
		return;
	}

	::ShowWindow(hwnd, SW_MINIMIZE);
}

void AcrylicWindow::maximize(bool toggle) {
	HWND hwnd = ::get_native_handle(get_window());
	if (hwnd == NULL) {
		print_error("Failed to get native handle.");
		return;
	}

	int show_command = SW_MAXIMIZE;
	if (toggle) {
		WINDOWPLACEMENT placement = {};
		placement.length = sizeof(WINDOWPLACEMENT);
		if (!::GetWindowPlacement(hwnd, &placement)) {
			print_error("Failed to GetWindowPlacement. Error: %d.", GetLastError());
			return;
		}
		
		if (placement.showCmd == SW_SHOWMAXIMIZED)
			show_command = SW_RESTORE;
	}

	::ShowWindow(hwnd, show_command);
}

void AcrylicWindow::close() {
	HWND hwnd = ::get_native_handle(get_window());
	if (hwnd == NULL) {
		print_error("Failed to get native handle.");
		return;
	}

	if (!::PostMessage(hwnd, WM_CLOSE, 0, 0))
		print_error("Failed to PostMessage(WM_CLOSE). Error: %d.", GetLastError());
}

void AcrylicWindow::_ready() {
	// NOTE: This function is called twice in the editor: when opening a scene 
	// in the editor and when loading a scene in a game running in the editor.
	// 	
	// BUG: Godot complains that super._ready is not defined when called from
	// GDScript.
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

	//case NOTIFICATION_WM_CLOSE_REQUEST:
		//print_warning("NOTIFICATION_WM_CLOSE_REQUEST");
		//break;
	}
}

void AcrylicWindow::_bind_methods() {
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

	BIND_PROPERTY(AcrylicWindow, Variant::FLOAT, text_size);
	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, always_on_top);
	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, drag_by_content);
	BIND_PROPERTY(AcrylicWindow, Variant::BOOL, modify_editor);
	
	BIND_PROPERTY_ENUM(AcrylicWindow, Variant::INT, frame, "Default, Custom");
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
		draw_rect(Rect2(w-1, 0, 1, h), Color(0, 1, 0));
		draw_rect(Rect2(0, h-1, w, 1), Color(0, 0, 1));
		draw_rect(Rect2(0, 0, 1, h), Color(1, 1, 0));
	}
#endif

}

void AcrylicWindow::on_ready() {
	if (!is_editor()) {
		if (!::subclass_wndproc(this))
			print_error("Failed to subclass wndproc.");
	}

	// Need this for drag_by_content property.
	set_mouse_filter(MOUSE_FILTER_PASS);

	apply_style();
}

void AcrylicWindow::on_exit_tree() {
	if (!is_editor()) {
		if (!::restore_wndproc(this))
			print_error("Failed to restore wndproc.");
	}
}

#pragma endregion

#pragma region PROPERTIES

DEFINE_PROPERTY_GET(AcrylicWindow, float, text_size)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, always_on_top)
DEFINE_PROPERTY_GET(AcrylicWindow, bool, drag_by_content)
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

void AcrylicWindow::set_text_size(const float p_text_size) {
	if (!is_node_ready()) {
		text_size = p_text_size;
		return;
	}

	if (text_size != p_text_size) {
		text_size = p_text_size;
		
		if (is_editor())
			return;

		Window* window = get_window();
		if (!window) {
			print_error("Failed to get window.");
			return;
		}

		window->set_content_scale_factor(text_size);
	}
}

void AcrylicWindow::set_always_on_top(const bool p_always_on_top) {
	if (!is_node_ready()) {
		always_on_top = p_always_on_top;
		return;
	}

	if (always_on_top != p_always_on_top) {
		always_on_top = p_always_on_top;

		EDITOR_GUARD;

		HWND hwnd = ::get_native_handle(get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return;
		}

		if (!::set_always_on_top(hwnd, p_always_on_top)) {
			print_error("Failed to set always_on_top.");
			return;
		}
	}

	// Can't use this one because it affects all the windows.
	// DisplayServer::get_singleton()->window_set_flag(DisplayServer::WINDOW_FLAG_ALWAYS_ON_TOP, always_on_top);
}

void AcrylicWindow::set_drag_by_content(const bool p_drag_by_content) {
	drag_by_content = p_drag_by_content;
}

void AcrylicWindow::set_autohide_title_bar(const AcrylicWindow::Autohide p_autohide_title_bar) {
	if (!is_node_ready()) {
		autohide_title_bar = p_autohide_title_bar;
		return;
	}

	if (autohide_title_bar != p_autohide_title_bar) {
		autohide_title_bar = p_autohide_title_bar;
	}
}

void AcrylicWindow::set_accent_title_bar(const AcrylicWindow::Accent p_accent_title_bar) {
	if (!is_node_ready()) {
		accent_title_bar = p_accent_title_bar;
		return;
	}

	if (accent_title_bar != p_accent_title_bar) {
		accent_title_bar = p_accent_title_bar;
		update_colors();
	}
}

void AcrylicWindow::set_auto_colors(const bool p_auto_colors) {
	if (!is_node_ready()) {
		auto_colors = p_auto_colors;
		return;
	}

	if (auto_colors != p_auto_colors) {
		auto_colors = p_auto_colors;
		if (auto_colors)
			update_colors();
	}
}

void AcrylicWindow::set_modify_editor(const bool p_modify_editor) {
	if (!is_node_ready()) {
		modify_editor = p_modify_editor;
		return;
	}

	if (modify_editor != p_modify_editor) {
		modify_editor = p_modify_editor;
		if (modify_editor)
			apply_style();
	}
}

void AcrylicWindow::set_frame(const AcrylicWindow::Frame p_frame) {
	if (!is_node_ready()) {
		frame = p_frame;
		return;
	}

	if (frame != p_frame) {
		frame = p_frame;

		EDITOR_GUARD;

		HWND hwnd = ::get_native_handle(get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return;
		}

		::set_frame(hwnd, frame);
	}
}

void AcrylicWindow::set_backdrop(const AcrylicWindow::Backdrop p_backdrop) {
	if (!is_node_ready()) {
		backdrop = p_backdrop;
		return;
	}

	if (backdrop != p_backdrop) {
		backdrop = p_backdrop;

		EDITOR_GUARD;

		Window* window = get_window();
		if (!window) {
			print_error("Failed to get window.");
			return;
		}

		HWND hwnd = ::get_native_handle(window);
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return;
		}

		queue_redraw();

		::set_backdrop(hwnd, backdrop);
	}
}

void AcrylicWindow::set_base_color(const Color &p_base_color) {
	if (!is_node_ready()) {
		base_color = p_base_color;
		return;
	}

	if (base_color != p_base_color) {
		base_color = p_base_color;
		queue_redraw();
		if (auto_colors)
			update_colors();
	}
}

void AcrylicWindow::set_border_color(const Color &p_border_color) {
	if (!is_node_ready()) {
		border_color = p_border_color;
		return;
	}

	if (!auto_colors)
		update_border_color(p_border_color);
}

void AcrylicWindow::set_title_bar_color(const Color &p_title_bar_color) {
	if (!is_node_ready()) {
		title_bar_color = p_title_bar_color;
		return;
	}

	if (!auto_colors)
		update_title_bar_color(p_title_bar_color);
}

void AcrylicWindow::set_text_color(const Color &p_text_color) {
	if (!is_node_ready()) {
		text_color = p_text_color;
		return;
	}

	if (!auto_colors)
		update_text_color(p_text_color);
}

void AcrylicWindow::set_clear_color(const Color &p_clear_color) {
	if (!is_node_ready()) {
		clear_color = p_clear_color;
		return;
	}

	if (!auto_colors)
		update_clear_color(p_clear_color);
}

void AcrylicWindow::apply_style() {
	if (is_editor()) {
		if (!modify_editor)
			return;

		print_warning("The new style will be applied to the Editor. Uncheck Modify Editor to disable this.");
	}

	Window* window = get_window();
	if (!window) {
		print_error("Failed to get window.");
		return;
	}

	HWND hwnd = ::get_native_handle(window);
	if (hwnd == NULL) {
		print_error("Failed to get native handle.");
		return;
	}

	window->set_content_scale_factor(text_size);

	if (!::set_always_on_top(hwnd, always_on_top))
		print_error("Failed to set always_on_top.");
	
	window->set_transparent_background(backdrop != BACKDROP_SOLID);

	auto rendering_server = RenderingServer::get_singleton();
	if (rendering_server)
		rendering_server->set_default_clear_color(clear_color);

	::set_backdrop(hwnd, backdrop);
	::set_color(hwnd, DWMWA_BORDER_COLOR, border_color);
	::set_color(hwnd, DWMWA_CAPTION_COLOR, title_bar_color);
	::set_color(hwnd, DWMWA_TEXT_COLOR, text_color);
	::set_frame(hwnd, frame);
}

void AcrylicWindow::update_colors() {
	float luminance = base_color.get_luminance();
	Color border_tint = Color(luminance, luminance, luminance) * 0.75;
	Color new_border_color = base_color.lerp(border_tint, 1 - base_color.a);
	Color new_title_bar_color = new_border_color;
	Color new_text_color = luminance < 0.65 ? Color(1, 1, 1) : Color(0, 0, 0);
	Color new_clear_color = base_color.darkened(0.85);

	if (backdrop == BACKDROP_SOLID && !accent_title_bar) {
		new_title_bar_color = base_color;
		new_border_color = base_color;
	}

	update_border_color(new_border_color);
	update_title_bar_color(new_title_bar_color);
	update_text_color(new_text_color);
	update_clear_color(new_clear_color);
}

void AcrylicWindow::update_border_color(const Color &p_border_color) {
	if (border_color != p_border_color) {
		border_color = p_border_color;

		EDITOR_GUARD;

		HWND hwnd = ::get_native_handle(get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return;
		}

		::set_color(hwnd, DWMWA_BORDER_COLOR, border_color);
	}
}

void AcrylicWindow::update_title_bar_color(const Color &p_title_bar_color) {
	if (title_bar_color != p_title_bar_color) {
		title_bar_color = p_title_bar_color;
		
		EDITOR_GUARD;

		HWND hwnd = ::get_native_handle(get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return;
		}

		::set_color(hwnd, DWMWA_CAPTION_COLOR, title_bar_color);
	}
}

void AcrylicWindow::update_text_color(const Color &p_text_color) {
	if (text_color != p_text_color) {
		text_color = p_text_color;

		EDITOR_GUARD;

		HWND hwnd = ::get_native_handle(get_window());
		if (hwnd == NULL) {
			print_error("Failed to get native handle.");
			return;
		}

		::set_color(hwnd, DWMWA_TEXT_COLOR, text_color);
	}
}

void AcrylicWindow::update_clear_color(const Color &p_clear_color) {
	if (clear_color != p_clear_color) {
		clear_color = p_clear_color;
		
		EDITOR_GUARD;

		auto rendering_server = RenderingServer::get_singleton();
		if (!rendering_server) {
			print_error("Failed to get rendering server.");
			return;
		}

		rendering_server->set_default_clear_color(Color(base_color.r, base_color.g, base_color.b));
	}
}

#pragma endregion

}
