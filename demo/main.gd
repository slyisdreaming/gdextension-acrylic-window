extends AcrylicWindow

#**************************************************************************#
#  main.gd                                                                 #
#  This is a demo project to showcase AcrylicWindow GDExtension.           #
#**************************************************************************#
#  MIT License                                                             #
#                                                                          #
#  Alexander Vishnevsky (Sly)                                              #
#  Check more on GitHub: https://github.com/slyisdreaming                  #
#  Hug me: https://boosty.to/slyisdreaming                                 #
#                                                                          #
#**************************************************************************#

# Make sure to enable transparency here:
#   Project > Project Settings > Display > Window:
#	  Transparent = true
# Can also set these settings to scale ui:
#	Stretch Mode = canvas_items
#	Aspect = expand
#	Scale = 1.5

@onready var settings: AcrylicSettings = $AcrylicSettings
@onready var title_bar: AcrylicTitleBarPrototype = $AcrylicTitleBarPrototype

#var dim_rect: ColorRect

var settings_tween: Tween

func _ready() -> void:
	settings.set_preset(AcrylicSettings.Preset.AcrylicBlue)
	# Show settings on open for demo.
	title_bar.settings_button.button_pressed = true
	
	#dim_rect = ColorRect.new()
	#dim_rect.name = "DimRect"
	#dim_rect.color = Color(0, 0, 0, 0.25)
	#dim_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	#dim_rect.z_index = 100
	##dim_rect.anchors_preset = Control.PRESET_FULL_RECT
	#
	#dim_rect.set_anchor_and_offset(SIDE_LEFT, 0, 0)
	#dim_rect.set_anchor_and_offset(SIDE_RIGHT, 1, 0)
	#dim_rect.set_anchor_and_offset(SIDE_TOP, 0, 0)
	#dim_rect.set_anchor_and_offset(SIDE_BOTTOM, 1, 0)
#
	#add_child(dim_rect)
	
	#rect.anchors_preset = Control.PRESET_TOP_WIDE
#rect.offset_bottom = 40

func _process(_delta: float) -> void:
	pass
	#dim_rect.visible = not always_on_top
	


func _on_acrylic_title_bar_prototype_settings_toggled(toggled_on: bool) -> void:
	if settings_tween:
		settings_tween.kill()
	settings_tween = create_tween()
	settings_tween.set_parallel(true)
	if toggled_on:
		settings_tween.tween_property(settings, "offset_right", -20, 0.2)
		settings_tween.tween_property(settings, "modulate:a", 1, 0.2)
	else:
		settings_tween.tween_property(settings, "offset_right", 300, 0.4)
		settings_tween.tween_property(settings, "modulate:a", 0, 0.4)
