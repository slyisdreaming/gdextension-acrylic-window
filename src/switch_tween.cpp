/**************************************************************************/
/*  switch_tween.cpp                                                      */
/*  Tween node that keeps state on/off.                                   */
/**************************************************************************/
/*  MIT License                                                           */
/*                                                                        */
/*  Alexander Vishnevsky (Sly)                                            */
/*  Check more on GitHub: https://github.com/slyisdreaming                */
/*  Hug me: https://boosty.to/slyisdreaming                               */
/*                                                                        */
/**************************************************************************/

#include "switch_tween.hpp"

#include "helpers.hpp"

namespace {
	const char PRINT_CATEGORY[] = "SwitchTween";
}

namespace godot {

void SwitchTween::_bind_methods() {
	BIND_PROPERTY(SwitchTween, Variant::BOOL, on);
	BIND_FUNCTION(SwitchTween, tween_property, "object", "property", "final_val", "duration");
}

Ref<PropertyTweener> SwitchTween::tween_property(Object* object, const NodePath& property, const Variant& final_val, double duration) {
	return tween->tween_property(object, property, final_val, duration);
}

#pragma region PROPERTIES

DEFINE_PROPERTY_GET(SwitchTween, bool, on)

void SwitchTween::set_on(const bool p_on) {
	if (tween.is_valid())
		tween->kill();

	tween = create_tween();
	on = p_on;
}

#pragma endregion
}
