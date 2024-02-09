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

@onready var acrylic_settings: AcrylicSettings = $AcrylicSettings

func _ready() -> void:
	acrylic_settings.set_preset(AcrylicSettings.Preset.AcrylicBlue)

