class_name AcrylicTitleBarPrototype
extends ColorRect

#**************************************************************************#
#  acrylic_title_bar_prototype.gd                                          #
#                                                                          #
#**************************************************************************#
#  MIT License                                                             #
#                                                                          #
#  Alexander Vishnevsky (Sly)                                              #
#  Check more on GitHub: https://github.com/slyisdreaming                  #
#  Hug me: https://boosty.to/slyisdreaming                                 #
#                                                                          #
#**************************************************************************#

@export var acrylic_window: AcrylicWindow

@onready var title_buttons: HBoxContainer = $TitleButtons
@onready var pin_button: Button = $TitleButtons/PinButton

func _ready() -> void:
	pass
	#update_ui()
	
	
func _process(delta: float) -> void:
	var s = 1.0 / acrylic_window.text_size
	scale = Vector2(s, s)
	set_anchor_and_offset(SIDE_RIGHT, acrylic_window.text_size, 0)
	
	color = acrylic_window.title_bar_color
	update_pin_button(acrylic_window.always_on_top)
	
	update_ui()
	
	if acrylic_window.accent_title_bar == AcrylicWindow.ACCENT_MOUSE_OVER:
		if get_global_rect().has_point(get_global_mouse_position()) \
			and not title_buttons.get_global_rect().has_point(get_global_mouse_position()):
			color = acrylic_window.title_bar_color
		else:
			color = Color.TRANSPARENT
			
	match acrylic_window.autohide_title_bar:
		AcrylicWindow.AUTOHIDE_ALWAYS:
			visible = get_global_rect().has_point(get_global_mouse_position())
			modulate = Color.WHITE if get_global_rect().has_point(get_global_mouse_position()) else Color.TRANSPARENT
			
		AcrylicWindow.AUTOHIDE_MAXIMIZED:
			#title_bar_prototype.visible = get_window().mode != Window.MODE_MAXIMIZED or title_bar_prototype.get_global_rect().has_point(get_global_mouse_position())
			modulate = Color.WHITE if get_window().mode != Window.MODE_MAXIMIZED or get_global_rect().has_point(get_global_mouse_position()) else Color.TRANSPARENT
	

func _input(event: InputEvent) -> void:
	if event is InputEventMouseMotion:
		var motion: = event as InputEventMouseMotion
		#print("MOTION: {motion}".format({"motion": [motion.position]}))
		
#func _unhandled_input(event):
	#var current_focus_control = get_window().gui_get_focus_owner()
	#if current_focus_control:
		#current_focus_control.release_focus()
		
		
func update_ui() -> void:
	color = acrylic_window.title_bar_color
	update_pin_button(acrylic_window.always_on_top)
	
	
#region CALLBACKS

#func _on_autohide_title_bar_button_item_selected(index: int) -> void:
	#autohide_title_bar = index
	#match autohide_title_bar:
		#AUTOHIDE_NEVER:
			#title_bar_prototype.visible = true
		#AUTOHIDE_ALWAYS:
			#title_bar_prototype.visible = false
		#AUTOHIDE_MAXIMIZED:
			#title_bar_prototype.visible = true
	#
#
#func _on_accent_title_bar_button_item_selected(index: int) -> void:
	#accent_title_bar = index
	#match accent_title_bar:
		#ACCENT_NEVER:
			#title_bar_prototype.color = Color.TRANSPARENT
		#ACCENT_ALWAYS:
			#title_bar_prototype.color = title_bar_color
		#ACCENT_MOUSE_OVER:
			#title_bar_prototype.color = title_bar_prototype.color
#
#
#func _on_auto_colors_button_toggled(toggled_on: bool) -> void:
	#auto_colors = toggled_on
	#update_ui()
#
#
#func _on_title_bar_color_button_color_changed(color: Color) -> void:
	#title_bar_color = color

	
#endregion

#region PROTOTYPE

func update_pin_button(toggled_on: bool) -> void:
	pin_button.button_pressed = toggled_on
	pin_button.text = "" if toggled_on else ""


func _on_pin_button_toggled(toggled_on: bool) -> void:
	update_pin_button(toggled_on)
	acrylic_window.always_on_top = toggled_on
	pin_button.is_hovered()


func _on_minimize_button_pressed() -> void:
	acrylic_window.minimize()


func _on_maximize_button_pressed() -> void:
	acrylic_window.maximize(true)


func _on_close_button_pressed() -> void:
	acrylic_window.close()

#endregion
