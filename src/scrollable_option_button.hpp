/**************************************************************************/
/*  scrollable_option_button.hpp                                          */
/*  This OptionButton allows to scroll items by mouse wheel.              */
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

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/option_button.hpp>

namespace godot {

class ScrollableOptionButton : public OptionButton {
	GDCLASS(ScrollableOptionButton, OptionButton)

public:
	virtual void _gui_input(const Ref<InputEvent>& event) override;

protected:
	static void _bind_methods();
};

}
