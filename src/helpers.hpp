/**************************************************************************/
/*  helpers.hpp                                                           */
/*  A set of functions and macros that make life easier with GDExtension. */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#pragma once

#include <godot_cpp/core/class_db.hpp>

namespace godot {
	class Control;
	class Popup;
	class Vector2;
	class Window;

	bool is_editor();

	bool has_popup(Window* window);
	bool has_popup(Control* control);

	// If search_depth == -1 then check all child controls.
	// Else check only controls up to search_depth:
	//
	//                    Search Depth
	// Window           
	//   Child Controls        0
	//     Child Controls      1
	//       Child Controls    2
	//
	Control* find_mouse_blocking_control(Window* window, int search_depth = -1);
	Control* find_mouse_blocking_control(Window* window, const Vector2& global_mouse_position, int search_depth = -1);

	// If search_depth == -1 then check this control and all the child controls.
	// Else check only controls up to search_depth:
	//
	//                    Search Depth
	// Control                 0
	//   Child Controls        1
	//     Child Controls      2
	//
	Control* find_mouse_blocking_control(Control* control, bool ignore_self = true, int search_depth = -1);
	Control* find_mouse_blocking_control(Control* control, const Vector2& global_mouse_position, bool ignore_self = true, int search_depth = -1);
}

//--------------------------------------------------------------------------
// PRINT HELPERS
//--------------------------------------------------------------------------

// EXAMPLES
// print_debug("Control Name: %s", control->get_name().c_escape().ascii().get_data());

void print_error_impl(const char* category, const char* funcname, const char* format, ...);
void print_warning_impl(const char* category, const char* funcname, const char* format, ...);
void print_message_impl(const char* category, const char* funcname, const char* format, ...);
void print_debug_impl(const char* category, const char* funcname, const char* format, ...);

#define print_error(format, ...) \
	print_error_impl(PRINT_CATEGORY, __func__, format, ##__VA_ARGS__)

#define print_warning(format, ...) \
	print_warning_impl(PRINT_CATEGORY, __func__, format, ##__VA_ARGS__)

#define print_message(format, ...) \
	print_message_impl(PRINT_CATEGORY, __func__, format, ##__VA_ARGS__)

#ifdef _DEBUG
#define print_debug(format, ...) \
	print_debug_impl(PRINT_CATEGORY, __func__, format, ##__VA_ARGS__)
#else
#define print_debug(format, ...) \
	void()
#endif

//--------------------------------------------------------------------------
// PROPERTIES
//--------------------------------------------------------------------------

#define DECLARE_PROPERTY(property_type, property_name, default_value) \
	public: \
		void set_##property_name(const property_type p_##property_name); \
		std::remove_reference<property_type>::type get_##property_name() const; \
	private: \
		property_type property_name = default_value;

#define DEFINE_PROPERTY_GET(class_name, property_type, property_name) \
	property_type class_name::get_##property_name() const { \
		return property_name; \
	}

#define DEFINE_PROPERTY_SET(class_name, property_type, property_name) \
	void class_name::set_##property_name(const property_type p_##property_name) { \
		property_name = p_##property_name; \
	}

#define BIND_PROPERTY(class_name, property_type, property_name) \
	::godot::ClassDB::bind_method(::godot::D_METHOD("get_"#property_name), &class_name::get_##property_name); \
	::godot::ClassDB::bind_method(::godot::D_METHOD("set_"#property_name, "p_"#property_name), &class_name::set_##property_name); \
	::godot::ClassDB::add_property(#class_name, ::godot::PropertyInfo(property_type, #property_name), "set_"#property_name, "get_"#property_name);

#define BIND_PROPERTY_ENUM(class_name, property_type, property_name, enum_values) \
	::godot::ClassDB::bind_method(::godot::D_METHOD("get_"#property_name), &class_name::get_ ## property_name); \
	::godot::ClassDB::bind_method(::godot::D_METHOD("set_"#property_name, "p_"#property_name), &class_name::set_ ## property_name); \
	::godot::ClassDB::add_property(#class_name, ::godot::PropertyInfo(property_type, #property_name,  ::godot::PROPERTY_HINT_ENUM, enum_values), "set_"#property_name, "get_"#property_name);

#define BIND_FUNCTION(class_name, function_name, ...) \
	::godot::ClassDB::bind_method(::godot::D_METHOD(#function_name, __VA_ARGS__), &class_name::function_name);
