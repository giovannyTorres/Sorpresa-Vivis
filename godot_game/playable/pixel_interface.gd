class_name PixelInterface
extends RefCounted
## Pixel UI drawn on the same 960 x 540 canvas as the game.
## The bundled OFL font and nearest textures keep the exported UI crisp.

const INK := Color("101820")
const PANEL := Color("17242e")
const GRID := Color("22313c")
const CREAM := Color("f6e3ca")
const MUTED := Color("8a9db2")
const PINK := Color("ef8cb8")
const BLUE := Color("8dd1ef")
const GREEN := Color("a6d784")
const PURPLE := Color("b48bdf")
const ZONES := ["PRADERA DE BIGOTES", "SENDERO DEL UMBRAL", "TORRE DE MALIKETH"]

const FONT := preload("res://playable/assets/pixel_font.ttf")

class MenuBackdrop extends Control:
	func _draw() -> void:
		draw_rect(Rect2(0, 0, 960, 540), PixelInterface.INK)
		for x in range(0, 960, 16):
			draw_rect(Rect2(x, 0, 1, 540), Color("16212a"))
		for y in range(0, 540, 16):
			draw_rect(Rect2(0, y, 960, 1), Color("16212a"))
		PixelInterface._draw_frame(self, Rect2(196, 128, 568, 356), PixelInterface.PINK)
		for rect in [Rect2(18, 128, 164, 172), Rect2(18, 312, 164, 172), Rect2(778, 128, 164, 172), Rect2(778, 312, 164, 172)]:
			var accent: Color = [PixelInterface.BLUE, PixelInterface.PINK, PixelInterface.GREEN, PixelInterface.PURPLE][int(rect.position.y > 300) + int(rect.position.x > 700) * 2]
			PixelInterface._draw_frame(self, rect, accent)
			draw_rect(Rect2(rect.position + Vector2(8, 32), Vector2(148, 1)), accent.darkened(0.55))
			for gx in range(int(rect.position.x + 8), int(rect.end.x - 8), 12):
				draw_rect(Rect2(gx, rect.position.y + 34, 1, 111), PixelInterface.GRID)
			for gy in range(int(rect.position.y + 34), int(rect.end.y - 25), 12):
				draw_rect(Rect2(rect.position.x + 8, gy, 148, 1), PixelInterface.GRID)
		PixelInterface._draw_paw(self, Vector2(29, 141), PixelInterface.BLUE, 1)
		PixelInterface._draw_heart(self, Vector2(29, 325), PixelInterface.PINK, 1)
		PixelInterface._draw_pattern(self, Vector2(791, 141), ["0000110", "0001110", "0011110", "0111100", "0111000", "0110000", "0100000"], PixelInterface.GREEN, 1)
		PixelInterface._draw_pattern(self, Vector2(791, 325), ["0011110", "0111000", "1110000", "1100000", "1110000", "0111000", "0011110"], PixelInterface.PURPLE, 1)
		PixelInterface._draw_heart(self, Vector2(156, 45), PixelInterface.PINK, 3)
		PixelInterface._draw_paw(self, Vector2(776, 47), PixelInterface.BLUE, 3)
		for x in range(18, 944, 37):
			var h := 12 + (x * 7) % 26
			var y := 536 - h
			draw_rect(Rect2(x + 10, y + 5, 4, h - 3), Color("34465a"))
			draw_rect(Rect2(x + 6, y + 6, 12, 5), Color("34465a"))
			draw_rect(Rect2(x + 3, y + 13, 18, 5), Color("34465a"))
			draw_rect(Rect2(x, y + 21, 24, 5), Color("34465a"))
		draw_rect(Rect2(18, 536, 924, 2), Color("34465a"))
		for pos in [Vector2(26, 30), Vector2(132, 107), Vector2(925, 31), Vector2(820, 100)]:
			PixelInterface._spark(self, pos, PixelInterface.MUTED, 1)

static func build_menu(game: Node2D) -> Dictionary:
	var layer := CanvasLayer.new()
	layer.layer = 20
	game.add_child(layer)
	var menu_layer := Control.new()
	menu_layer.name = "PixelMenu"
	menu_layer.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	menu_layer.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	layer.add_child(menu_layer)
	var backdrop := MenuBackdrop.new()
	backdrop.mouse_filter = Control.MOUSE_FILTER_IGNORE
	backdrop.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	menu_layer.add_child(backdrop)

	var heading := _label(menu_layer, Rect2(185, 17, 590, 66), "SORPRESA VIVI'S", CREAM, 52)
	heading.add_theme_color_override("font_shadow_color", Color("aa6671"))
	heading.add_theme_constant_override("shadow_offset_x", 0)
	heading.add_theme_constant_override("shadow_offset_y", 4)
	_label(menu_layer, Rect2(162, 83, 636, 22), "PEQUEÑAS AVENTURAS, GRANDES SORPRESAS", CREAM, 16)
	_label(menu_layer, Rect2(20, 35, 122, 66), "UN MUNDO\nLLENO DE\nPEQUEÑAS\nMARAVILLAS", MUTED, 12)
	_label(menu_layer, Rect2(818, 35, 122, 66), "EXPLORA\nCOMBATE\nCOLECCIONA\nDESCUBRE", MUTED, 12)

	_character_card(menu_layer, Rect2(18, 128, 164, 172), "WIKY WIKERMAN", "MAGIA DIGITAL", BLUE, "wisky")
	_character_card(menu_layer, Rect2(18, 312, 164, 172), "VIVIS", "UN GRAN CORAZON", PINK, "vivi")
	_character_card(menu_layer, Rect2(778, 128, 164, 172), "GIOVANNY", "PODER NATURAL", GREEN, "giovanny")
	_character_card(menu_layer, Rect2(778, 312, 164, 172), "MALIKETH", "MAGIA LUNAR", PURPLE, "maliketh")

	var description := _label(menu_layer, Rect2(214, 154, 532, 116), "", CREAM, 18)
	description.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	description.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	description.add_theme_constant_override("line_spacing", 0)
	var buttons := VBoxContainer.new()
	buttons.position = Vector2(268, 281)
	buttons.size = Vector2(424, 152)
	buttons.add_theme_constant_override("separation", 10)
	menu_layer.add_child(buttons)
	var play_button := _button(buttons, "JUGAR  [ENTER]", PINK, Callable(game, "_on_play"))
	var menu_button := _button(buttons, "VOLVER AL INICIO", BLUE, func(): game.call("show_overlay", "menu"))
	var quit_button := _button(buttons, "SALIR", MUTED, func(): game.get_tree().quit())
	_label(menu_layer, Rect2(220, 447, 520, 18), "WASD  MOVER   ESPACIO  DISPARAR   E  WIKY", MUTED, 12)
	_label(menu_layer, Rect2(218, 490, 524, 24), "LA AMABILIDAD TAMBIEN ES UNA GRAN AVENTURA", PINK, 12)
	return {"menu_layer": menu_layer, "heading": heading, "description": description,
		"play_button": play_button, "menu_button": menu_button, "quit_button": quit_button}

static func _label(parent: Node, bounds: Rect2, value: String, color: Color, font_size: int) -> Label:
	var label := Label.new()
	label.position = bounds.position
	label.size = bounds.size
	label.text = value
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	label.add_theme_font_override("font", font())
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", color)
	parent.add_child(label)
	return label

static func _button(parent: Node, text_value: String, color: Color, callback: Callable) -> Button:
	var button := Button.new()
	button.text = text_value
	button.custom_minimum_size = Vector2(424, 42)
	button.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
	button.add_theme_font_override("font", font())
	button.add_theme_font_size_override("font_size", 20)
	button.add_theme_color_override("font_color", CREAM)
	button.add_theme_color_override("font_hover_color", CREAM)
	button.add_theme_color_override("font_focus_color", CREAM)
	button.add_theme_color_override("font_pressed_color", INK)
	for state in ["normal", "hover", "pressed", "focus"]:
		var style := StyleBoxFlat.new()
		style.bg_color = Color("26313d") if state == "normal" else (color if state == "pressed" else Color("394353"))
		style.border_color = color
		style.set_border_width_all(2 if state != "focus" else 3)
		style.shadow_color = Color("090f14")
		style.shadow_size = 2
		style.shadow_offset = Vector2(0, 3)
		style.content_margin_top = 7
		style.content_margin_bottom = 7
		button.add_theme_stylebox_override(state, style)
	button.pressed.connect(callback)
	parent.add_child(button)
	return button

static func _character_card(parent: Node, bounds: Rect2, title: String, power: String, accent: Color, asset: String) -> void:
	_label(parent, Rect2(bounds.position + Vector2(5, 8), Vector2(154, 18)), title, accent, 16)
	var portrait := TextureRect.new()
	portrait.position = bounds.position + Vector2(24, 44)
	portrait.size = Vector2(116, 96)
	if ResourceLoader.exists("res://playable/assets/characters.png"):
		var sheet := load("res://playable/assets/characters.png") as Texture2D
		var row: int = {"vivi": 0, "wisky": 1, "giovanny": 2, "maliketh": 3}[asset]
		portrait.name = "CharacterPortrait%d" % row
		portrait.set_meta("character_row", row)
		var character_art = load("res://playable/pixel_characters.gd")
		portrait.texture = character_art.portrait(sheet, row)
	else:
		portrait.texture = load("res://playable/assets/%s.svg" % asset) as Texture2D
	portrait.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	portrait.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	portrait.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	portrait.mouse_filter = Control.MOUSE_FILTER_IGNORE
	parent.add_child(portrait)
	_label(parent, Rect2(bounds.position + Vector2(5, 148), Vector2(154, 15)), power, accent, 12)

static func draw_hud(canvas: Node2D, state: Dictionary) -> void:
	var stage := clampi(int(state.get("stage", 0)), 0, 2)
	var hp := int(state.get("hp", 6))
	var max_hp := int(state.get("max_hp", 6))
	var bell := bool(state.get("bell_collected", false))
	var defeated := bool(state.get("boss_defeated", false))
	var enemy_count := int(state.get("enemies", 0))
	var cooldown := float(state.get("companion_cooldown", 0.0))
	var seconds := int(state.get("run_time", 0.0))
	canvas.draw_rect(Rect2(0, 0, 960, 104), INK)
	for x in range(0, 960, 16):
		canvas.draw_rect(Rect2(x, 0, 1, 104), Color("17232c"))
	for y in range(0, 104, 16):
		canvas.draw_rect(Rect2(0, y, 960, 1), Color("17232c"))
	canvas.draw_rect(Rect2(12, 102, 936, 2), PINK.darkened(0.4))
	canvas.draw_rect(Rect2(584, 15, 1, 74), Color("344553"))
	_draw_heart(canvas, Vector2(25, 16), PINK, 2)
	_text(canvas, Vector2(55, 37), "SORPRESA VIVI'S", CREAM, 24)
	_text(canvas, Vector2(24, 63), "%02d / 03  %s" % [stage + 1, ZONES[stage]], CREAM, 16)
	var objective := "CASCABEL: %s  /  PELUSAS: %d" % ["LISTO" if bell else "BUSCALO", enemy_count]
	if stage == 2:
		objective = "GIOVANNY: ACERCATE Y PULSA E" if defeated else ("DERROTA A MALIKETH" if bell else "CASCABEL DORADO: ROMPE EL ESCUDO")
	elif bell and enemy_count == 0:
		objective = "CAMINO LIBRE: SALIDA A LA DERECHA"
	_text(canvas, Vector2(24, 88), objective, GREEN, 14)
	_text(canvas, Vector2(608, 30), "VIDA", PINK, 12)
	for index in range(max_hp):
		_draw_heart(canvas, Vector2(670 + index * 27, 15), PINK if index < hp else Color("41404a"), 2)
	_draw_paw(canvas, Vector2(608, 49), BLUE, 2)
	_text(canvas, Vector2(633, 66), "E  WIKY: %s" % ("LISTO" if cooldown <= 0 else "%02d S" % ceili(cooldown)), BLUE, 16)
	for x in range(634, 846, 6):
		canvas.draw_rect(Rect2(x, 77, 4, 3), Color("324551"))
	var ready_ratio := 1.0 - clampf(cooldown / 5.0, 0.0, 1.0)
	for x in range(634, int(634 + 212 * ready_ratio), 6):
		canvas.draw_rect(Rect2(x, 77, 4, 3), BLUE.darkened(0.2))
	_text(canvas, Vector2(865, 89), "%d:%02d" % [seconds / 60, seconds % 60], CREAM, 16)
	if stage == 2 and not defeated and enemy_count > 0:
		_draw_frame(canvas, Rect2(300, 109, 360, 14), PURPLE, Color("201d2c"))
		var boss_ratio := clampf(float(state.get("boss_hp", 30)) / 30.0, 0.0, 1.0)
		canvas.draw_rect(Rect2(306, 114, floorf(348 * boss_ratio), 4), PURPLE)
	if float(state.get("toast_time", 0.0)) > 0:
		var message := String(state.get("toast", ""))
		_draw_frame(canvas, Rect2(99, 450, 762, 32), GREEN, Color("101c21"))
		var text_size := 18 if font().get_string_size(message, HORIZONTAL_ALIGNMENT_LEFT, -1, 18).x < 730 else 14
		_text(canvas, Vector2(480, 472), message, CREAM, text_size, true)
	canvas.draw_rect(Rect2(0, 504, 960, 36), INK)
	canvas.draw_rect(Rect2(12, 504, 936, 1), Color("36495a"))
	_text(canvas, Vector2(24, 526), "WASD / FLECHAS  MOVER    ESPACIO  DISPARAR    E  WIKY    ESC  PAUSA", CREAM, 14)
	_text(canvas, Vector2(804, 526), "M  %s" % ("SIN SONIDO" if bool(state.get("muted", false)) else "SONIDO"), MUTED, 14)

static func _draw_frame(canvas: CanvasItem, rect: Rect2, accent: Color, background: Color = PANEL) -> void:
	canvas.draw_rect(rect, background)
	canvas.draw_rect(Rect2(rect.position + Vector2(7, 0), Vector2(rect.size.x - 14, 2)), accent)
	canvas.draw_rect(Rect2(rect.position + Vector2(7, rect.size.y - 2), Vector2(rect.size.x - 14, 2)), accent)
	canvas.draw_rect(Rect2(rect.position + Vector2(0, 7), Vector2(2, rect.size.y - 14)), accent)
	canvas.draw_rect(Rect2(rect.position + Vector2(rect.size.x - 2, 7), Vector2(2, rect.size.y - 14)), accent)
	for flip_x in [false, true]:
		for flip_y in [false, true]:
			var horizontal_x := rect.end.x - 8 if flip_x else rect.position.x + 2
			var vertical_x := rect.end.x - 4 if flip_x else rect.position.x + 2
			var horizontal_y := rect.end.y - 4 if flip_y else rect.position.y + 2
			var vertical_y := rect.end.y - 8 if flip_y else rect.position.y + 2
			canvas.draw_rect(Rect2(horizontal_x, horizontal_y, 6, 2), accent)
			canvas.draw_rect(Rect2(vertical_x, vertical_y, 2, 6), accent)
			canvas.draw_rect(Rect2(vertical_x, horizontal_y, 1, 1), accent.lightened(0.25))

static func _draw_heart(canvas: CanvasItem, point: Vector2, color: Color, scale_value: int = 2) -> void:
	var pattern := ["0110110", "1111111", "1111111", "0111110", "0011100", "0001000"]
	_draw_pattern(canvas, point, pattern, color.darkened(0.4), scale_value, Vector2(0, 1))
	_draw_pattern(canvas, point, pattern, color, scale_value)
	canvas.draw_rect(Rect2(point + Vector2(scale_value * 2, scale_value), Vector2(scale_value, scale_value)), color.lightened(0.25))

static func _draw_paw(canvas: CanvasItem, point: Vector2, color: Color, scale_value: int = 2) -> void:
	_draw_pattern(canvas, point, ["0010100", "0110110", "0100010", "0000000", "0011100", "0111110", "0011100"], color, scale_value)

static func _spark(canvas: CanvasItem, point: Vector2, color: Color, scale_value: int) -> void:
	_draw_pattern(canvas, point, ["00100", "00100", "11111", "00100", "00100"], color, scale_value)

static func _draw_pattern(canvas: CanvasItem, point: Vector2, pattern: Array, color: Color, scale_value: int, offset: Vector2 = Vector2.ZERO) -> void:
	for y in range(pattern.size()):
		for x in range(String(pattern[y]).length()):
			if String(pattern[y])[x] == "1":
				canvas.draw_rect(Rect2(point + (Vector2(x, y) + offset) * scale_value, Vector2.ONE * scale_value), color)

static func _text(canvas: CanvasItem, point: Vector2, value: String, color: Color, size_value: int, centered: bool = false) -> void:
	var start := point
	if centered:
		start.x -= font().get_string_size(value, HORIZONTAL_ALIGNMENT_LEFT, -1, size_value).x / 2
	canvas.draw_string(font(), start.round(), value, HORIZONTAL_ALIGNMENT_LEFT, -1, size_value, color)

static func font() -> FontFile:
	return FONT
