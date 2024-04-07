/**************************************************************************/
/*  helpers.cpp                                                           */
/*  A set of functions and macros that make life easier with GDExtension. */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#include "helpers.hpp"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/popup.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>


#include <cstdarg>
#include <ctime>
#include <type_traits>

using namespace godot;

namespace {
	const char PRINT_CATEGORY[] = "helpers";
}

namespace godot {

bool is_editor() {
	Engine* engine = Engine::get_singleton();
	if (!engine) {
		print_error("Failed to get engine.");
		return false;
	}

	return engine->is_editor_hint();
}

bool has_popup(Window* window) {
	//TypedArray<Node> nodes = window->find_children("", "Popup", true, false);
	//for (int i = 0; i < nodes.size(); i++) {
	//	Popup* popup = Object::cast_to<Popup>(nodes[i]);
	//	if (popup->is_visible())
	//		return true;
	//}

	//Control* focus = window->gui_get_focus_owner();
	//if (focus)
	//	print_debug("%s", focus->get_name().c_escape().ascii().get_data());

	//auto rendering_server = RenderingServer::get_singleton();
	//auto display_server = DisplayServer::get_singleton();

	//return display_server->window_get_active_popup() != DisplayServer::INVALID_WINDOW_ID;

	int child_count = window->get_child_count(true);
	for (int i = 0; i < child_count; i++) {
		Node* child = window->get_child(i, true);

		Popup* popup = Object::cast_to<Popup>(child);
		if (popup) {
			if (popup->is_visible())
				return true;

			continue;
		}

		Control* control = Object::cast_to<Control>(child);
		if (control) {
			if (control->is_visible() && has_popup(control))
				return true;
			
			continue;
		}
	}

	return false;
}

bool has_popup(Control* control) {
	int child_count = control->get_child_count(true);
	for (int i = 0; i < child_count; i++) {
		Node* child = control->get_child(i, true);

		Popup* popup = Object::cast_to<Popup>(child);
		if (popup) {
			if (popup->is_visible())
				return true;

			continue;
		}

		Control* child_control = Object::cast_to<Control>(child);
		if (child_control) {
			if (child_control->is_visible() && has_popup(child_control))
				return true;

			continue;
		}
	}

	return false;
}

Control* find_mouse_blocking_control(Window* window, int search_depth) {
	Viewport *viewport = window->get_viewport();
	if (!viewport) {
		print_error("Failed to get viewport.");
		return nullptr;
	}

	return find_mouse_blocking_control(window, viewport->get_mouse_position(), search_depth);
}

Control* find_mouse_blocking_control(Window* window, const Vector2& global_mouse_position, int search_depth) {
	int child_count = window->get_child_count();
	for (int i = 0; i < child_count; i++) {
		Node* child = window->get_child(i);
		//print_debug("%s", child->get_name().c_escape().ascii().get_data());

		Control* control = Object::cast_to<Control>(child);
		if (!control) {
			print_debug("NOT CONTROL");
			continue;
		}

		Control* blocking_control = find_mouse_blocking_control(control, global_mouse_position, search_depth);
		if (blocking_control)
			return blocking_control;
	}

	return nullptr;
}

Control* find_mouse_blocking_control(Control* control, bool ignore_self, int search_depth) {
	return find_mouse_blocking_control(control, control->get_global_mouse_position(), ignore_self, search_depth);
}

Control* find_mouse_blocking_control(Control* control, const Vector2& global_mouse_position, bool ignore_self, int search_depth) {
	if (control->get_global_rect().has_point(global_mouse_position)) {
		if (!ignore_self && control->get_mouse_filter() == Control::MOUSE_FILTER_STOP)
			//if (!ignore_self && control->get_mouse_filter() != Control::MOUSE_FILTER_IGNORE)
			return control;
	}

	int child_count = control->get_child_count();
	for (int i = 0; i < child_count; i++) {
		//print_debug("  %s", control->get_child(i)->get_name().c_escape().ascii().get_data());

		Control* child_control = Object::cast_to<Control>(control->get_child(i));
		if (!child_control) {
			//print_debug("NOT CONTROL");
			continue;
		}

		Control* blocking_control = find_mouse_blocking_control(child_control, global_mouse_position, false, search_depth > 0 ? (search_depth - 1) : -1);
		if (blocking_control) {
			//print_debug("  BLOCK: %s", blocking_control->get_name().c_escape().ascii().get_data());
			return blocking_control;
		}
	}

	return nullptr;
}

}

//--------------------------------------------------------------------------
// PRINT HELPERS
//--------------------------------------------------------------------------

#define MAKE_TIMESTAMP \
	std::time_t time = std::time({}); \
	char timestamp[std::size("yyyy-mm-dd hh:mm:ss")]; \
	std::strftime(std::data(timestamp), std::size(timestamp), "%F %T", std::localtime(&time));

void print_error_impl(const char* category, const char* funcname, const char* format, ...) {
	MAKE_TIMESTAMP;

	fprintf(stderr, "\033[31;1mERROR:   ");
	fprintf(stderr, "\033[0;31m%s [%s::%s] ", timestamp, category, funcname);

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);

	fprintf(stderr, "\n\033[0m");
}

void print_warning_impl(const char* category, const char* funcname, const char* format, ...) {
	MAKE_TIMESTAMP;

	fprintf(stderr, "\033[33;1mWARNING: ");
	fprintf(stderr, "\033[0;33m%s [%s::%s] ", timestamp, category, funcname);
	
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);

	fprintf(stderr, "\n\033[0m");

	//_err_print_error(funcname, category, 0, format, true, true);
}

void print_message_impl(const char* category, const char* funcname, const char* format, ...) {
	MAKE_TIMESTAMP;

	fprintf(stdout, "MESSAGE: ");
	fprintf(stdout, "%s [%s::%s] ", timestamp, category, funcname);

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stdout, format, argptr);
	va_end(argptr);

	fprintf(stdout, "\n");
}

void print_debug_impl(const char* category, const char* funcname, const char* format, ...) {
	MAKE_TIMESTAMP;

	fprintf(stdout, "\033[36;1mDEBUG:   ");
	fprintf(stdout, "\033[0;36m%s [%s::%s] ", timestamp, category, funcname);

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stdout, format, argptr);
	va_end(argptr);

	fprintf(stdout, "\n\033[0m");

	//_err_print_error(funcname, category, 0, format, true, true);
}
