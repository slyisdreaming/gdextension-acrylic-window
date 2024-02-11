/**************************************************************************/
/*  switch_tween.hpp                                                      */
/*  Tween node that keeps state on/off                                    */
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

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/property_tweener.hpp>
#include <godot_cpp/classes/tween.hpp>

namespace godot {

class SwitchTween : public Node {
	GDCLASS(SwitchTween, Node)

public:
	DECLARE_PROPERTY(bool, on, false)

public:
	Ref<PropertyTweener> tween_property(Object* object, const NodePath& property, const Variant& final_val, double duration);

protected:
	static void _bind_methods();

private:
	Ref<Tween> tween;
};

}
