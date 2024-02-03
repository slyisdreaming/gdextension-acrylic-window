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

@onready var settings_grid: GridContainer = $SettingsGrid
@onready var preset_button: OptionButton = $SettingsGrid/PresetButton
@onready var text_size_slider: HSlider = $SettingsGrid/TextSizeSlider
@onready var always_on_top_button: CheckButton = $SettingsGrid/AlwaysOnTopButton
@onready var drag_by_content_button: CheckButton = $SettingsGrid/DragByContentButton
@onready var frame_button: OptionButton = $SettingsGrid/FrameButton
@onready var backdrop_button: OptionButton = $SettingsGrid/BackdropButton
@onready var autohide_title_bar_button: OptionButton = $SettingsGrid/AutohideTitleBarButton
@onready var accent_title_bar_button: OptionButton = $SettingsGrid/AccentTitleBarButton
@onready var auto_colors_button: CheckButton = $SettingsGrid/AutoColorsButton
@onready var base_color_button: ColorPickerButton = $SettingsGrid/BaseColorButton
@onready var border_color_button: ColorPickerButton = $SettingsGrid/BorderColorButton
@onready var title_bar_color_button: ColorPickerButton = $SettingsGrid/TitleBarColorButton
@onready var text_color_button: ColorPickerButton = $SettingsGrid/TextColorButton
@onready var clear_color_button: ColorPickerButton = $SettingsGrid/ClearColorButton
@onready var border_color_label: Label = $SettingsGrid/BorderColorLabel
@onready var title_bar_color_label: Label = $SettingsGrid/TitleBarColorLabel
@onready var text_color_label: Label = $SettingsGrid/TextColorLabel
@onready var clear_color_label: Label = $SettingsGrid/ClearColorLabel

# PROTOTYPE
@onready var title_bar_prototype: ColorRect = $TitleBar_PROTOTYPE
@onready var title_buttons: HBoxContainer = $TitleBar_PROTOTYPE/TitleButtons
@onready var pin_button: Button = $TitleBar_PROTOTYPE/TitleButtons/PinButton

func _ready() -> void:
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

	#var rect = ColorRect.new()
	#rect.size = Vector2(size.x, 50)
	#rect.scale = Vector2(1.0 / text_size, 1.0 / text_size)
	#rect.color = Color(1, 0, 0)
	#
	#rect.set_anchor_and_offset(SIDE_LEFT, 0, 0)
	#rect.set_anchor_and_offset(SIDE_RIGHT, 1, 0)
	#rect.set_anchor_and_offset(SIDE_TOP, 0, 0)
	#rect.set_anchor_and_offset(SIDE_BOTTOM, 0, 40)
	#
	#rect.anchors_preset = Control.PRESET_TOP_WIDE
	#rect.offset_bottom = 40
	#
	#rect.set_anchors_and_offsets_preset(Control.PRESET_TOP_WIDE, Control.PRESET_MODE_KEEP_WIDTH, 0)
	#	
	#add_child(rect)
	
	
func _process(delta: float) -> void:
	var s = 1.0 / text_size
	title_bar_prototype.scale = Vector2(s, s)
	title_bar_prototype.set_anchor_and_offset(SIDE_RIGHT, text_size, 0)
	
	if accent_title_bar == ACCENT_MOUSE_OVER:
		if title_bar_prototype.get_global_rect().has_point(get_global_mouse_position()) \
			and not title_buttons.get_global_rect().has_point(get_global_mouse_position()):
			title_bar_prototype.color = title_bar_color
		else:
			title_bar_prototype.color = Color.TRANSPARENT
			
	match autohide_title_bar:
		AUTOHIDE_ALWAYS:
			title_bar_prototype.visible = title_bar_prototype.get_global_rect().has_point(get_global_mouse_position())
			title_bar_prototype.modulate = Color.WHITE if title_bar_prototype.get_global_rect().has_point(get_global_mouse_position()) else Color.TRANSPARENT
			
		AUTOHIDE_MAXIMIZED:
			#title_bar_prototype.visible = get_window().mode != Window.MODE_MAXIMIZED or title_bar_prototype.get_global_rect().has_point(get_global_mouse_position())
			title_bar_prototype.modulate = Color.WHITE if get_window().mode != Window.MODE_MAXIMIZED or title_bar_prototype.get_global_rect().has_point(get_global_mouse_position()) else Color.TRANSPARENT
			


func _input(event: InputEvent) -> void:
	if event is InputEventMouseMotion:
		var motion: = event as InputEventMouseMotion
		#print("MOTION: {motion}".format({"motion": [motion.position]}))
		
#func _unhandled_input(event):
	#var current_focus_control = get_window().gui_get_focus_owner()
	#if current_focus_control:
		#current_focus_control.release_focus()
		
		
func update_ui() -> void:
	text_size_slider.value = text_size
	always_on_top_button.button_pressed = always_on_top
	drag_by_content_button.button_pressed = drag_by_content
	frame_button.select(frame)
	backdrop_button.select(backdrop)
	autohide_title_bar_button.select(autohide_title_bar)
	accent_title_bar_button.select(accent_title_bar)
	auto_colors_button.button_pressed = auto_colors
	base_color_button.color = base_color
	border_color_button.color = border_color
	title_bar_color_button.color = title_bar_color
	text_color_button.color = text_color
	clear_color_button.color = clear_color
	
	border_color_label.visible = not auto_colors
	title_bar_color_label.visible = not auto_colors
	text_color_label.visible = not auto_colors
	clear_color_label.visible = not auto_colors
	border_color_button.visible = not auto_colors
	title_bar_color_button.visible = not auto_colors
	text_color_button.visible = not auto_colors
	clear_color_button.visible = not auto_colors
	
	# PROTOTYPE
	update_pin_button(always_on_top)
	
	
#region CALLBACKS

func _on_preset_button_item_selected(index: int) -> void:
	match index:
		Preset.AcrylicBlue:
			backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			auto_colors = true
			base_color = "224ff284"
		Preset.AcrylicFaintBlue:
			backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			auto_colors = true
			base_color = "224ff234"
		Preset.AcrylicRed:
			backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			auto_colors = true
			base_color = "d80d0d9b"
		Preset.AcrylicGreen:
			backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			auto_colors = true
			base_color = "1dd06434"
		Preset.AcrylicDarkGrey:
			backdrop = AcrylicWindow.BACKDROP_ACRYLIC
			auto_colors = true
			base_color = "222526bd"
		Preset.MicaBlue:
			backdrop = AcrylicWindow.BACKDROP_MICA
			auto_colors = true
			base_color = "224ff27e"
		Preset.MicaRed:
			backdrop = AcrylicWindow.BACKDROP_MICA
			auto_colors = true
			base_color = "d80d0d9b"
		Preset.MicaGreen:
			backdrop = AcrylicWindow.BACKDROP_MICA
			auto_colors = true
			base_color = "14bc6491"
		Preset.MicaGrey:
			backdrop = AcrylicWindow.BACKDROP_MICA
			auto_colors = true
			base_color = "4d4d4dce"
		Preset.MicaDarkGrey:
			backdrop = AcrylicWindow.BACKDROP_MICA
			auto_colors = true
			base_color = "222526bd"
	
	update_ui()


func _on_text_size_slider_value_changed(value: float) -> void:
	text_size = value


func _on_always_on_top_button_toggled(toggled_on: bool) -> void:
	always_on_top = toggled_on
	update_pin_button(toggled_on)


func _on_drag_by_content_button_toggled(toggled_on: bool) -> void:
	drag_by_content = toggled_on


func _on_frame_button_item_selected(index: int) -> void:
	frame = index


func _on_backdrop_button_item_selected(index: int) -> void:
	backdrop = index


func _on_autohide_title_bar_button_item_selected(index: int) -> void:
	autohide_title_bar = index
	match autohide_title_bar:
		AUTOHIDE_NEVER:
			title_bar_prototype.visible = true
		AUTOHIDE_ALWAYS:
			title_bar_prototype.visible = false
		AUTOHIDE_MAXIMIZED:
			title_bar_prototype.visible = true
	

func _on_accent_title_bar_button_item_selected(index: int) -> void:
	accent_title_bar = index
	match accent_title_bar:
		ACCENT_NEVER:
			title_bar_prototype.color = Color.TRANSPARENT
		ACCENT_ALWAYS:
			title_bar_prototype.color = title_bar_color
		ACCENT_MOUSE_OVER:
			title_bar_prototype.color = title_bar_prototype.color


func _on_auto_colors_button_toggled(toggled_on: bool) -> void:
	auto_colors = toggled_on
	update_ui()


func _on_base_color_button_color_changed(color: Color) -> void:
	base_color = color


func _on_border_color_button_color_changed(color: Color) -> void:
	border_color = color


func _on_title_bar_color_button_color_changed(color: Color) -> void:
	title_bar_color = color


func _on_text_color_button_color_changed(color: Color) -> void:
	text_color = color


func _on_clear_color_button_color_changed(color: Color) -> void:
	clear_color = color
	
#endregion

#region PROTOTYPE

func update_pin_button(toggled_on: bool) -> void:
	pin_button.button_pressed = toggled_on
	pin_button.text = "" if toggled_on else ""


func _on_pin_button_toggled(toggled_on: bool) -> void:
	update_pin_button(toggled_on)
	always_on_top = toggled_on
	always_on_top_button.button_pressed = toggled_on
	pin_button.is_hovered()


func _on_minimize_button_pressed() -> void:
	minimize()


func _on_maximize_button_pressed() -> void:
	maximize(true)


func _on_close_button_pressed() -> void:
	close()

#endregion
