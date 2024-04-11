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
@onready var title_bar: AcrylicTitleBar = $AcrylicTitleBar

var settings_tween: Tween


func _ready() -> void:
	settings.set_preset(AcrylicSettings.Preset.AcrylicAmethyst)
	# Show settings on open for demo.
	title_bar.settings_button.button_pressed = true


func _process(_delta: float) -> void:
	pass


func _on_acrylic_title_bar_settings_toggled(toggled_on: bool) -> void:
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
