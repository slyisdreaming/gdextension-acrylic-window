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
	AcrylicBlue,
	AcrylicFaintBlue,
	AcrylicRed,
	AcrylicGreen,
	AcrylicDarkGrey,
	MicaBlue,
	MicaRed,
	MicaGreen,
	MicaGrey,
	MicaDarkGrey
}

@export var acrylic_window: AcrylicWindow

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

	for key in Preset.keys():
		preset_button.add_item(key)
	
	update_ui()


func set_preset(preset: Preset):
	preset_button.select(preset)
	
	match preset:
		Preset.AcrylicBlue:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "224ff284"
		Preset.AcrylicFaintBlue:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "224ff234"
		Preset.AcrylicRed:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "d80d0d9b"
		Preset.AcrylicGreen:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "1dd06434"
		Preset.AcrylicDarkGrey:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "222526bd"
		Preset.MicaBlue:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "224ff27e"
		Preset.MicaRed:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "d80d0d9b"
		Preset.MicaGreen:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "14bc6491"
		Preset.MicaGrey:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "4d4d4dce"
		Preset.MicaDarkGrey:
			acrylic_window.backdrop = AcrylicWindow.BACKDROP_MICA
			acrylic_window.auto_colors = true
			acrylic_window.base_color = "222526bd"
	
	update_ui()


func update_ui() -> void:
	text_size_slider.value = acrylic_window.text_size
	always_on_top_button.button_pressed = acrylic_window.always_on_top
	drag_by_content_button.button_pressed = acrylic_window.drag_by_content
	drag_by_right_click_button.button_pressed = acrylic_window.drag_by_right_click
	dim_strength_slider.value = acrylic_window.dim_strength
	frame_button.select(acrylic_window.frame)
	backdrop_button.select(acrylic_window.backdrop)
	autohide_title_bar_button.select(acrylic_window.autohide_title_bar)
	accent_title_bar_button.select(acrylic_window.accent_title_bar)
	auto_colors_button.button_pressed = acrylic_window.auto_colors
	base_color_button.color = acrylic_window.base_color
	border_color_button.color = acrylic_window.border_color
	title_bar_color_button.color = acrylic_window.title_bar_color
	text_color_button.color = acrylic_window.text_color
	clear_color_button.color = acrylic_window.clear_color
	
	border_color_label.visible = not acrylic_window.auto_colors
	title_bar_color_label.visible = not acrylic_window.auto_colors
	text_color_label.visible = not acrylic_window.auto_colors
	clear_color_label.visible = not acrylic_window.auto_colors
	border_color_button.visible = not acrylic_window.auto_colors
	title_bar_color_button.visible = not acrylic_window.auto_colors
	text_color_button.visible = not acrylic_window.auto_colors
	clear_color_button.visible = not acrylic_window.auto_colors
	
	
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
	update_ui()


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
