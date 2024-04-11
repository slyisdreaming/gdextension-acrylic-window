/**************************************************************************/
/*  scrollable_option_button.cpp                                          */
/*  This OptionButton allows to scroll items by mouse wheel.              */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#include "scrollable_option_button.hpp"

#include "helpers.hpp"

#include <godot_cpp/classes/input_event_mouse_button.hpp>

namespace {
	const char PRINT_CATEGORY[] = "ScrollableOptionButton";
}

namespace godot {

void ScrollableOptionButton::_gui_input(const Ref<InputEvent>& event) {
	ERR_FAIL_COND(event.is_null());

	Ref<InputEventMouseButton> mb = event;

	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			int32_t new_item_index = -1;
			switch (mb->get_button_index()) {
				case MOUSE_BUTTON_WHEEL_UP:
					new_item_index = get_selected() - 1;
					break;

				case MOUSE_BUTTON_WHEEL_DOWN:
					new_item_index = get_selected() + 1;
					break;
			}

			if (new_item_index >= 0 && new_item_index < get_item_count()) {
				select(new_item_index);
				//emit_signal(SNAME("item_selected"), get_selected());
				emit_signal("item_selected", get_selected());
				return;
			}
		}
	}

	OptionButton::_gui_input(event);
}

void ScrollableOptionButton::_bind_methods()
{}

}
