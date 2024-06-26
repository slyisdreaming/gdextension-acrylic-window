class_name AcrylicTitleBar
extends ColorRect

#**************************************************************************#
#  acrylic_title_bar.gd                                                    #
#                                                                          #
#**************************************************************************#
#  MIT License                                                             #
#                                                                          #
#  Alexander Vishnevsky (Sly)                                              #
#  Check more on GitHub: https://github.com/slyisdreaming                  #
#  Hug me: https://boosty.to/slyisdreaming                                 #
#                                                                          #
#**************************************************************************#

signal settings_toggled(toggled_on: bool)

@export var acrylic_window: AcrylicWindow
@onready var title_header: HBoxContainer = $TitleHeader
@onready var title_label: Label = $TitleHeader/TitleLabel
@onready var title_buttons: HBoxContainer = $TitleButtons
@onready var settings_button: Button = $TitleButtons/SettingsButton
@onready var pin_button: Button = $TitleButtons/PinButton
@onready var minimize_button: Button = $TitleButtons/MinimizeButton
@onready var maximize_button: Button = $TitleButtons/MaximizeButton
@onready var close_button: Button = $TitleButtons/CloseButton
@onready var autohide_tween: SwitchTween = $AutohideTween
@onready var accent_tween: SwitchTween = $AccentTween

var settings_tween: Tween
var pin_tween: Tween
var maximize_tween: Tween
var minimize_tween: Tween
var close_tween: Tween

func _ready() -> void:
	# SIGNALS
	acrylic_window.text_size_changed.connect(update_text_size)
	acrylic_window.always_on_top_changed.connect(update_always_on_top)
	acrylic_window.text_color_changed.connect(update_text_color)
	
	# UPDATES
	update_text_size(acrylic_window.text_size)
	update_always_on_top(acrylic_window.always_on_top)
	update_text_color(acrylic_window.text_color)
	
	resized.connect(_on_resized)
	_on_resized()
	
func _process(delta: float) -> void:
	var mouse_position: Vector2 = get_global_mouse_position()
	var has_mouse: bool = get_global_rect().has_point(mouse_position)
	
	# accent ---------------------------------------------------------------
	color.r = acrylic_window.title_bar_color.r
	color.g = acrylic_window.title_bar_color.g
	color.b = acrylic_window.title_bar_color.b

	var should_accent: bool = false
	match acrylic_window.accent_title_bar:
		AcrylicWindow.ACCENT_ALWAYS:
			should_accent = true
		AcrylicWindow.ACCENT_MOUSE_OVER:
			should_accent = has_mouse #and not title_buttons.get_global_rect().has_point(mouse_position)
			
	if should_accent:
		if not accent_tween.on:
			accent_tween.on = true
			accent_tween.tween_property(self, "color:a", acrylic_window.title_bar_color.a, 0.2)
	else:
		if accent_tween.on:
			accent_tween.on = false
			accent_tween.tween_property(self, "color:a", 0.0, 0.4)
	
	# autohide -------------------------------------------------------------
	var should_hide: bool = acrylic_window.frame != AcrylicWindow.FRAME_CUSTOM
	if not should_hide:
		match acrylic_window.autohide_title_bar:
			AcrylicWindow.AUTOHIDE_ALWAYS:
				should_hide = not has_mouse
			AcrylicWindow.AUTOHIDE_MAXIMIZED:
				should_hide = not has_mouse and get_window().mode == Window.MODE_MAXIMIZED
	
	if should_hide:
		if not autohide_tween.on:
			autohide_tween.on = true
			autohide_tween.tween_property(self, "modulate:a", 0.0, 0.4).set_ease(Tween.EASE_IN_OUT)
	else:
		if autohide_tween.on:
			autohide_tween.on = false
			autohide_tween.tween_property(self, "modulate:a", 1.0, 0.200).set_ease(Tween.EASE_OUT)


func animate_on_hover(hover: bool, control: Control, tween: Tween, max_a: float = 1.0):
	if tween:
		tween.kill()
	tween = create_tween()
	tween.set_parallel(true)
	if hover:
		tween.tween_property(control, "theme_override_styles/hover:bg_color:a", max_a, 0.2)
		tween.tween_property(control, "theme_override_styles/normal:bg_color:a", max_a, 0.2)
	else:
		tween.tween_property(control, "theme_override_styles/hover:bg_color:a", 0, 0.4)
		tween.tween_property(control, "theme_override_styles/normal:bg_color:a", 0, 0.4)

#region UPDATES

func update_text_size(text_size: float) -> void:
	var s = 1.0 / text_size
	scale = Vector2(s, s)
	set_anchor_and_offset(SIDE_RIGHT, text_size, 0)
	

func update_always_on_top(always_on_top: bool) -> void:
	pin_button.button_pressed = always_on_top
	pin_button.text = "" if always_on_top else ""
	
	
func update_text_color(text_color: Color) -> void:
	settings_button.add_theme_color_override("font_color", text_color)
	pin_button.add_theme_color_override("font_color", text_color)
	minimize_button.add_theme_color_override("font_color", text_color)
	maximize_button.add_theme_color_override("font_color", text_color)
	close_button.add_theme_color_override("font_color", text_color)


#endregion

#region CALLBACKS

func _on_settings_button_toggled(toggled_on: bool) -> void:
	settings_toggled.emit(toggled_on)
	

func _on_pin_button_toggled(toggled_on: bool) -> void:
	acrylic_window.always_on_top = toggled_on


func _on_minimize_button_pressed() -> void:
	acrylic_window.minimize()


func _on_maximize_button_pressed() -> void:
	acrylic_window.maximize(true)


func _on_close_button_pressed() -> void:
	acrylic_window.close()
		

func _on_settings_button_mouse_entered() -> void:
	animate_on_hover(true, settings_button, settings_tween, 0.9)


func _on_settings_button_mouse_exited() -> void:
	animate_on_hover(false, settings_button, settings_tween)


func _on_pin_button_mouse_entered() -> void:
	animate_on_hover(true, pin_button, pin_tween, 0.9)


func _on_pin_button_mouse_exited() -> void:
	animate_on_hover(false, pin_button, pin_tween)


func _on_minimize_button_mouse_entered() -> void:
	animate_on_hover(true, minimize_button, minimize_tween, 0.25)


func _on_minimize_button_mouse_exited() -> void:
	animate_on_hover(false, minimize_button, minimize_tween)


func _on_maximize_button_mouse_entered() -> void:
	animate_on_hover(true, maximize_button, maximize_tween)


func _on_maximize_button_mouse_exited() -> void:
	animate_on_hover(false, maximize_button, maximize_tween)


func _on_close_button_mouse_entered() -> void:
	animate_on_hover(true, close_button, close_tween)
	

func _on_close_button_mouse_exited() -> void:
	animate_on_hover(false, close_button, close_tween)
	

func _on_resized() -> void:
	title_label.visible = title_header.size.x < size.x - title_buttons.size.x
	
#endregion

