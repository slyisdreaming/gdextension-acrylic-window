class_name AcrylicSettings
extends GridContainer

#**************************************************************************#
#  acrylic_settings.gd                                                     #
#                                                                          #
#**************************************************************************#
#  MIT License                                                             #
#                                                                          #
#  Alexander Vishnevsky (Sly)                                              #
#  Check more on GitHub: https://github.com/slyisdreaming                  #
#  Hug me: https://boosty.to/slyisdreaming                                 #
#                                                                          #
#**************************************************************************#

enum Preset {
	AcrylicAmethyst,
	AcrylicBlue,
	AcrylicFaintBlue,
	AcrylicRed,
	AcrylicGreen,
	AcrylicDarkGrey,
	AcrylicWhite,
	MicaBlue,
	MicaRed,
	MicaGreen,
	MicaGrey,
	MicaDarkGrey,
	MicaWhite
}

@export var acrylic_window: AcrylicWindow

@onready var acrylic_theme = preload("res://addons/acrylic-window/acrylic_theme.tres")

@onready var preset_button: OptionButton = $PresetButton
@onready var text_size_slider: HSlider = $TextSizeSlider
@onready var always_on_top_button: CheckButton = $AlwaysOnTopButton
@onready var drag_by_content_button: CheckButton = $DragByContentButton
@onready var drag_by_right_click_button: CheckButton = $DragByRightClickButton
@onready var dim_strength_slider: HSlider = $DimStrengthSlider
@onready var frame_button: OptionButton = $FrameButton
@onready var backdrop_button: OptionButton = $BackdropButton
@onready var autohide_title_bar_button: OptionButton = $AutohideTitleBarButton
@onready var accent_title_bar_button: OptionButton = $AccentTitleBarButton
@onready var auto_colors_button: CheckButton = $AutoColorsButton
@onready var base_color_button: ColorPickerButton = $BaseColorButton
@onready var border_color_button: ColorPickerButton = $BorderColorButton
@onready var title_bar_color_button: ColorPickerButton = $TitleBarColorButton
@onready var text_color_button: ColorPickerButton = $TextColorButton
@onready var clear_color_button: ColorPickerButton = $ClearColorButton
@onready var border_color_label: Label = $BorderColorLabel
@onready var title_bar_color_label: Label = $TitleBarColorLabel
@onready var text_color_label: Label = $TextColorLabel
@onready var clear_color_label: Label = $ClearColorLabel

func _ready() -> void:
	# OPTION BUTTONS
	for key in Preset.keys():
		preset_button.add_item(key)
		
	frame_button.add_item("None")
	frame_button.add_item("Default")
	frame_button.add_item("Custom")
	
	backdrop_button.add_item("Solid")
	backdrop_button.add_item("Transparent")
	backdrop_button.add_item("Acrylic")
	backdrop_button.add_item("Mica")
	backdrop_button.add_item("Tabbed")

	autohide_title_bar_button.add_item("Never")
	autohide_title_bar_button.add_item("Always")
	autohide_title_bar_button.add_item("Maximized")
	
	accent_title_bar_button.add_item("Never")
	accent_title_bar_button.add_item("Always")
	accent_title_bar_button.add_item("Mouse Over")
		
	# SIGNALS
	acrylic_window.always_on_top_changed.connect(update_always_on_top)
	acrylic_window.backdrop_changed.connect(update_backdrop)
	acrylic_window.auto_colors_changed.connect(update_auto_colors)
	acrylic_window.base_color_changed.connect(update_base_color)
	acrylic_window.border_color_changed.connect(update_border_color)
	acrylic_window.title_bar_color_changed.connect(update_title_bar_color)
	acrylic_window.text_color_changed.connect(update_text_color)
	acrylic_window.clear_color_changed.connect(update_clear_color)
	
	# UPDATES
	update_always_on_top(acrylic_window.always_on_top)
	update_backdrop(acrylic_window.backdrop)
	update_auto_colors(acrylic_window.auto_colors)
	update_base_color(acrylic_window.base_color)
	update_border_color(acrylic_window.border_color)
	update_title_bar_color(acrylic_window.title_bar_color)
	update_text_color(acrylic_window.text_color)
	update_clear_color(acrylic_window.clear_color)
	
	text_size_slider.value = acrylic_window.text_size
	drag_by_content_button.button_pressed = acrylic_window.drag_by_content
	drag_by_right_click_button.button_pressed = acrylic_window.drag_by_right_click
	dim_strength_slider.value = acrylic_window.dim_strength
	frame_button.select(acrylic_window.frame)
	autohide_title_bar_button.select(acrylic_window.autohide_title_bar)
	accent_title_bar_button.select(acrylic_window.accent_title_bar)


func set_preset(preset: Preset):
	preset_button.select(preset)
	
	match preset:
		Preset.AcrylicAmethyst:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "050e2e84"
			acrylic_window.border_color = "080c1da1"
			acrylic_window.title_bar_color = "080c1da1"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "01020784"
			
		Preset.AcrylicBlue:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "224ff284"
			acrylic_window.border_color = "2f469ba1"
			acrylic_window.title_bar_color = "0c2994a1"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "050c2484"
			
		Preset.AcrylicFaintBlue:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "224ff234"
			acrylic_window.border_color = "374162a3"
			acrylic_window.title_bar_color = "212841a3"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "050c2434"
			
		Preset.AcrylicRed:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "d80d0d9b"
			acrylic_window.border_color = "941818a9"
			acrylic_window.title_bar_color = "860101a9"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "2002029b"
			
		Preset.AcrylicGreen:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "1dd06434"
			acrylic_window.border_color = "678b75a3"
			acrylic_window.title_bar_color = "2d512da3"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "041f0f34"
			
		Preset.AcrylicDarkGrey:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "0e0e0ebd"
			acrylic_window.border_color = "678b75a3"
			acrylic_window.title_bar_color = "2d512da3"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "041f0f34"
			
		Preset.AcrylicWhite:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "ffffffc2"
			acrylic_window.border_color = "f0f0f0c1"
			acrylic_window.title_bar_color = "8f8f8f8e"
			acrylic_window.text_color = "000000ff"
			acrylic_window.clear_color = "262626c2"
			
			
		Preset.MicaBlue:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "224ff27e"
			acrylic_window.border_color = "3046969f"
			acrylic_window.title_bar_color = "051c739f"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "050c247e"
			
		Preset.MicaRed:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "d80d0d9b"
			acrylic_window.border_color = "941818a9"
			acrylic_window.title_bar_color = "6f0000a9"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "2002029b"
			
		Preset.MicaGreen:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "14bc6491"
			acrylic_window.border_color = "494949cb"
			acrylic_window.title_bar_color = "2a2a2acb"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "0c0c0cce"
			
		Preset.MicaGrey:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "4d4d4dce"
			acrylic_window.border_color = "161616be"
			acrylic_window.title_bar_color = "262626be"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "0c0c0cce"
			
		Preset.MicaDarkGrey:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "181818af"
			acrylic_window.border_color = "0f0f0fbe"
			acrylic_window.title_bar_color = "2e2d2dbe"
			acrylic_window.text_color = "ffffff"
			acrylic_window.clear_color = "020202bd"
			
		Preset.MicaWhite:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = false
			acrylic_window.base_color = "fffffff4"
			acrylic_window.border_color = "fcfcfcf2"
			acrylic_window.title_bar_color = "8f8f8f8e"
			acrylic_window.text_color = "000000"
			acrylic_window.clear_color = "262626f4"


#region UPDATES

func update_always_on_top(always_on_top: bool) -> void:
	always_on_top_button.button_pressed = always_on_top
	
	
func update_backdrop(backdrop: AcrylicWindow.Backdrop) -> void:
	backdrop_button.select(backdrop)
	
	
func update_auto_colors(auto_colors: bool) -> void:
	auto_colors_button.button_pressed = auto_colors
	border_color_label.visible = not auto_colors
	title_bar_color_label.visible = not auto_colors
	text_color_label.visible = not auto_colors
	clear_color_label.visible = not auto_colors
	border_color_button.visible = not auto_colors
	title_bar_color_button.visible = not auto_colors
	text_color_button.visible = not auto_colors
	clear_color_button.visible = not auto_colors
	

func update_base_color(base_color: Color) -> void:
	base_color_button.color = base_color
	

func update_border_color(border_color: Color) -> void:
	border_color_button.color = border_color

	
func update_title_bar_color(title_bar_color: Color) -> void:
	title_bar_color_button.color = title_bar_color
	
	
func update_text_color(text_color: Color) -> void:
	text_color_button.color = text_color
	acrylic_theme.set_color("font_color", "Label", text_color)
	# propagate_call("update")
	
	
func update_clear_color(clear_color: Color) -> void:
	clear_color_button.color = clear_color

#endregion
	
#region CALLBACKS

func _on_preset_button_item_selected(index: int) -> void:
	set_preset(index)


func _on_text_size_slider_value_changed(value: float) -> void:
	acrylic_window.text_size = value
	
	
func _on_always_on_top_button_toggled(toggled_on: bool) -> void:
	acrylic_window.always_on_top = toggled_on


func _on_drag_by_content_button_toggled(toggled_on: bool) -> void:
	acrylic_window.drag_by_content = toggled_on


func _on_drag_by_right_click_button_toggled(toggled_on: bool) -> void:
	acrylic_window.drag_by_right_click = toggled_on


func _on_dim_strength_slider_value_changed(value: float) -> void:
	acrylic_window.dim_strength = value


func _on_frame_button_item_selected(index: int) -> void:
	acrylic_window.frame = index


func _on_backdrop_button_item_selected(index: int) -> void:
	acrylic_window.backdrop = index


func _on_autohide_title_bar_button_item_selected(index: int) -> void:
	acrylic_window.autohide_title_bar = index
	

func _on_accent_title_bar_button_item_selected(index: int) -> void:
	acrylic_window.accent_title_bar = index


func _on_auto_colors_button_toggled(toggled_on: bool) -> void:
	acrylic_window.auto_colors = toggled_on


func _on_base_color_button_color_changed(color: Color) -> void:
	acrylic_window.base_color = color


func _on_border_color_button_color_changed(color: Color) -> void:
	acrylic_window.border_color = color


func _on_title_bar_color_button_color_changed(color: Color) -> void:
	acrylic_window.title_bar_color = color


func _on_text_color_button_color_changed(color: Color) -> void:
	acrylic_window.text_color = color


func _on_clear_color_button_color_changed(color: Color) -> void:
	acrylic_window.clear_color = color
	
#endregion
