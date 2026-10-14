extends Node2D
## Aventura completa: tres pantallas, dos controles de combate y un rescate.

const SIZE := Vector2(960, 540)
const FIELD := Rect2(42, 126, 876, 352)
const SPEED := 205.0
const MAX_HP := 6
const ZONES := ["Pradera de Bigotes", "Sendero del Umbral", "Torre de Maliketh"]
const VIVI := preload("res://playable/assets/vivi.svg")
const WIKY := preload("res://playable/assets/wisky.svg")
const MALIKETH := preload("res://playable/assets/maliketh.svg")
const GIOVANNY := preload("res://playable/assets/giovanny.svg")
const INK := Color("201d37")
const CREAM := Color("fff0d5")
const PINK := Color("ff91bb")

var mode := "menu"
var stage := 0
var player := Vector2(114, 310)
var companion := Vector2(72, 350)
var hp := MAX_HP
var facing := Vector2.RIGHT
var enemies: Array[Dictionary] = []
var shots: Array[Dictionary] = []
var hazards: Array[Dictionary] = []
var particles: Array[Dictionary] = []
var obstacles: Array[Rect2] = []
var hearts: Array[Vector2] = []
var bell_position := Vector2.ZERO
var bell_collected := false
var boss_defeated := false
var invincible := 0.0
var attack_cooldown := 0.0
var companion_cooldown := 0.0
var pulse := 0.0
var time := 0.0
var run_time := 0.0
var toast_time := 0.0
var toast := ""
var muted := false
var menu_layer: Control
var heading: Label
var description: Label
var play_button: Button
var menu_button: Button
var quit_button: Button
var sound: AudioStreamPlayer

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_bind_keys()
	_build_menu()
	sound = AudioStreamPlayer.new()
	sound.volume_db = -16
	add_child(sound)
	load_stage(0)
	show_overlay("menu")

func _bind_keys() -> void:
	var bindings := {
		"walk_left": [KEY_A, KEY_LEFT], "walk_right": [KEY_D, KEY_RIGHT],
		"walk_up": [KEY_W, KEY_UP], "walk_down": [KEY_S, KEY_DOWN],
		"shoot": [KEY_SPACE], "help": [KEY_E, KEY_ENTER],
		"pause_game": [KEY_ESCAPE], "retry": [KEY_R],
		"mute_game": [KEY_M], "fullscreen_game": [KEY_F11]
	}
	for action in bindings:
		if not InputMap.has_action(action):
			InputMap.add_action(action)
		for key in bindings[action]:
			var event := InputEventKey.new()
			event.physical_keycode = key
			InputMap.action_add_event(action, event)

func _build_menu() -> void:
	var layer := CanvasLayer.new()
	add_child(layer)
	menu_layer = Control.new()
	menu_layer.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	layer.add_child(menu_layer)
	var shade := ColorRect.new()
	shade.color = Color(0.05, 0.04, 0.12, 0.84)
	shade.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	menu_layer.add_child(shade)
	var box := PanelContainer.new()
	box.position = Vector2(230, 90)
	box.size = Vector2(500, 360)
	var style := StyleBoxFlat.new()
	style.bg_color = INK
	style.border_color = PINK
	style.set_border_width_all(3)
	style.set_corner_radius_all(12)
	style.content_margin_left = 32
	style.content_margin_right = 32
	style.content_margin_top = 24
	style.content_margin_bottom = 24
	box.add_theme_stylebox_override("panel", style)
	menu_layer.add_child(box)
	var column := VBoxContainer.new()
	column.add_theme_constant_override("separation", 14)
	box.add_child(column)
	heading = Label.new()
	heading.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	heading.add_theme_font_size_override("font_size", 34)
	heading.add_theme_color_override("font_color", PINK)
	column.add_child(heading)
	description = Label.new()
	description.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	description.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	description.custom_minimum_size = Vector2(430, 100)
	description.add_theme_font_size_override("font_size", 18)
	description.add_theme_color_override("font_color", CREAM)
	column.add_child(description)
	play_button = _button(column, "Jugar", _on_play)
	menu_button = _button(column, "Volver al inicio", func(): show_overlay("menu"))
	quit_button = _button(column, "Salir", func(): get_tree().quit())

func _button(parent: Node, label: String, callback: Callable) -> Button:
	var button := Button.new()
	button.text = label
	button.custom_minimum_size.y = 42
	button.add_theme_font_size_override("font_size", 19)
	var normal := StyleBoxFlat.new()
	normal.bg_color = Color("413052")
	normal.set_corner_radius_all(5)
	var hover := normal.duplicate() as StyleBoxFlat
	hover.bg_color = Color("725073")
	button.add_theme_stylebox_override("normal", normal)
	button.add_theme_stylebox_override("hover", hover)
	button.add_theme_stylebox_override("focus", hover)
	button.add_theme_color_override("font_color", CREAM)
	button.pressed.connect(callback)
	parent.add_child(button)
	return button

func show_overlay(next_mode: String) -> void:
	mode = next_mode
	menu_layer.visible = true
	menu_button.visible = mode != "menu"
	match mode:
		"menu":
			heading.text = "Sorpresa Vivi’s"
			description.text = "Maliketh se llevó a Giovanny.\nVivi y Wiky van a traerlo de vuelta.\n\n3 lugares · 3 cascabeles · 1 rescate"
			play_button.text = "Jugar  [Enter]"
		"paused":
			heading.text = "Un respiro"
			description.text = "WASD / flechas: mover\nEspacio: disparar · E: ayuda de Wiky\nM: sonido · F11: pantalla completa"
			play_button.text = "Continuar  [Esc]"
		"lost":
			heading.text = "Otra oportunidad"
			description.text = "La pelusa ganó esta ronda.\nPuedes repetir este lugar con toda tu vida.\n\nMuévete al disparar y pide ayuda a Wiky."
			play_button.text = "Reintentar  [R]"
		"won":
			heading.text = "¡Lo lograron!"
			description.text = "Giovanny está a salvo.\nVivi y Wiky vuelven a casa con él.\n\nPara Vivi, con mucho cariño. ♡\nTiempo: %d:%02d" % [int(run_time) / 60, int(run_time) % 60]
			play_button.text = "Volver a jugar  [Enter]"
	play_button.grab_focus()
	queue_redraw()

func _on_play() -> void:
	if mode == "menu" or mode == "won":
		run_time = 0.0
		load_stage(0)
	elif mode == "lost":
		load_stage(stage)
	mode = "playing"
	menu_layer.visible = false
	get_viewport().gui_release_focus()
	queue_redraw()

func load_stage(index: int) -> void:
	stage = index
	player = Vector2(108, 310)
	companion = Vector2(72, 352)
	facing = Vector2.RIGHT
	hp = MAX_HP
	invincible = 1.0
	attack_cooldown = 0.0
	companion_cooldown = 0.0
	pulse = 0.0
	bell_collected = false
	boss_defeated = false
	enemies.clear()
	shots.clear()
	hazards.clear()
	particles.clear()
	hearts.clear()
	obstacles.clear()
	match stage:
		0:
			obstacles.assign([Rect2(280, 190, 104, 72), Rect2(486, 354, 100, 76)])
			bell_position = Vector2(754, 205)
			_spawn_enemy(Vector2(470, 220))
			_spawn_enemy(Vector2(700, 385))
			_spawn_enemy(Vector2(785, 310))
			hearts.assign([Vector2(430, 410)])
			_notify("Recoge el cascabel dorado y limpia la pradera.")
		1:
			obstacles.assign([Rect2(260, 350, 98, 78), Rect2(476, 172, 108, 74)])
			bell_position = Vector2(768, 412)
			_spawn_enemy(Vector2(410, 310))
			_spawn_enemy(Vector2(620, 360))
			_spawn_enemy(Vector2(770, 235))
			_spawn_enemy(Vector2(815, 335))
			hearts.assign([Vector2(392, 205), Vector2(670, 435)])
			_notify("Un cascabel más. La torre ya está cerca.")
		2:
			bell_position = Vector2(480, 400)
			_spawn_enemy(Vector2(692, 284), true)
			hearts.assign([Vector2(175, 190), Vector2(175, 428)])
			_notify("El último cascabel rompe el escudo. ¡Ve por Maliketh!")
	queue_redraw()

func _spawn_enemy(position_value: Vector2, boss: bool = false) -> void:
	enemies.append({"pos": position_value, "hp": 30 if boss else 3,
		"boss": boss, "timer": 1.4, "flash": 0.0})

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.echo:
		return
	if event.is_action_pressed("fullscreen_game"):
		var fullscreen := DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_FULLSCREEN
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED if fullscreen else DisplayServer.WINDOW_MODE_FULLSCREEN)
	elif event.is_action_pressed("mute_game"):
		muted = not muted
		if muted:
			sound.stop()
		_notify("Sonido desactivado" if muted else "Sonido activado")
	elif event.is_action_pressed("pause_game"):
		if mode == "playing":
			show_overlay("paused")
		elif mode == "paused":
			_on_play()
	elif event.is_action_pressed("retry") and mode == "lost":
		_on_play()
	elif event.is_action_pressed("help") and mode == "playing":
		call_companion()

func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_FOCUS_OUT and mode == "playing":
		show_overlay("paused")

func _physics_process(delta: float) -> void:
	if mode != "playing":
		return
	time += delta
	run_time += delta
	invincible = maxf(0.0, invincible - delta)
	attack_cooldown = maxf(0.0, attack_cooldown - delta)
	companion_cooldown = maxf(0.0, companion_cooldown - delta)
	pulse = maxf(0.0, pulse - delta)
	toast_time = maxf(0.0, toast_time - delta)
	var direction := Input.get_vector("walk_left", "walk_right", "walk_up", "walk_down")
	if direction.length_squared() > 0.0:
		facing = direction
		player = move_actor(player, direction * SPEED * delta, 14.0)
	var follow := player - facing * 48.0 + Vector2(0, 18)
	companion = companion.move_toward(follow, 180.0 * delta)
	if Input.is_action_pressed("shoot"):
		shoot()
	_update_enemies(delta)
	_update_shots(delta)
	_update_hazards(delta)
	_update_pickups()
	for effect in particles:
		effect["life"] -= delta
		effect["pos"] += effect["vel"] * delta
	particles = particles.filter(func(p): return p["life"] > 0.0)
	if mode == "playing" and stage < 2 and gate_open() and Rect2(854, 260, 64, 104).has_point(player):
		load_stage(stage + 1)
	queue_redraw()

func move_actor(origin: Vector2, motion: Vector2, radius: float) -> Vector2:
	var result := origin
	var candidate := origin + Vector2(motion.x, 0)
	if _walkable(candidate, radius):
		result.x = candidate.x
	candidate = result + Vector2(0, motion.y)
	if _walkable(candidate, radius):
		result.y = candidate.y
	return result

func _walkable(point: Vector2, radius: float) -> bool:
	if not FIELD.grow(-radius).has_point(point):
		return false
	for obstacle in obstacles:
		if obstacle.grow(radius).has_point(point):
			return false
	return true

func shoot() -> void:
	if attack_cooldown > 0.0 or mode != "playing":
		return
	attack_cooldown = 0.25
	var direction := facing
	var closest := 600.0
	for enemy in enemies:
		var distance := player.distance_to(enemy["pos"])
		if distance < closest:
			closest = distance
			direction = player.direction_to(enemy["pos"])
	shots.append({"pos": player + direction * 20, "vel": direction * 510.0, "life": 1.5})
	_tone(680, 0.045)

func call_companion() -> void:
	if mode != "playing":
		return
	if stage == 2 and boss_defeated and player.distance_to(Vector2(824, 230)) < 85.0:
		show_overlay("won")
		_tone(1040, 0.25)
		return
	if companion_cooldown > 0.0:
		_notify("Wiky está descansando. Espera %d s." % ceili(companion_cooldown))
		return
	companion_cooldown = 5.0
	pulse = 0.45
	for enemy in enemies:
		if player.distance_to(enemy["pos"]) < 175.0:
			_damage_enemy(enemy, 3)
	_remove_dead_enemies()
	_tone(350, 0.14)
	_notify("¡Wiky usa Arañazo Ponzoñoso!")

func _update_enemies(delta: float) -> void:
	for enemy in enemies:
		enemy["flash"] = maxf(0, enemy["flash"] - delta)
		var position_value: Vector2 = enemy["pos"]
		if enemy["boss"]:
			enemy["pos"] = Vector2(695 + sin(time * 0.8) * 80, 302 + sin(time * 1.4) * 105)
			enemy["timer"] -= delta
			if enemy["timer"] <= 0.0:
				enemy["timer"] = 1.25 if enemy["hp"] < 15 else 1.65
				var aim := position_value.direction_to(player)
				for angle in [-0.28, 0.0, 0.28]:
					hazards.append({"pos": position_value, "vel": aim.rotated(angle) * 180, "life": 5.0})
		else:
			var toward := position_value.direction_to(player)
			enemy["pos"] = move_actor(position_value, toward * (63 + stage * 14) * delta, 16)
		if player.distance_to(enemy["pos"]) < (34.0 if enemy["boss"] else 27.0):
			take_damage()

func _update_shots(delta: float) -> void:
	for shot in shots:
		shot["life"] -= delta
		var from: Vector2 = shot["pos"]
		shot["pos"] += shot["vel"] * delta
		for obstacle in obstacles:
			if obstacle.grow(4).has_point(shot["pos"]):
				shot["life"] = 0.0
		if shot["life"] <= 0.0:
			continue
		for enemy in enemies:
			if enemy["hp"] <= 0:
				continue
			var nearest := Geometry2D.get_closest_point_to_segment(enemy["pos"], from, shot["pos"])
			if nearest.distance_to(enemy["pos"]) < (33.0 if enemy["boss"] else 22.0):
				_damage_enemy(enemy, 1)
				shot["life"] = 0.0
				break
	shots = shots.filter(func(s): return s["life"] > 0.0 and FIELD.grow(20).has_point(s["pos"]))
	_remove_dead_enemies()

func _damage_enemy(enemy: Dictionary, damage: int) -> void:
	if enemy["boss"] and not bell_collected:
		_notify("¡Recoge el cascabel dorado para romper su escudo!")
		return
	enemy["hp"] -= damage
	enemy["flash"] = 0.15
	_burst(enemy["pos"], PINK)

func _remove_dead_enemies() -> void:
	for enemy in enemies:
		if enemy["hp"] <= 0 and enemy["boss"]:
			boss_defeated = true
			hazards.clear()
			_notify("¡Maliketh cayó! Acércate a Giovanny y pulsa E.")
	enemies = enemies.filter(func(e): return e["hp"] > 0)

func _update_hazards(delta: float) -> void:
	for hazard in hazards:
		hazard["life"] -= delta
		var from: Vector2 = hazard["pos"]
		hazard["pos"] += hazard["vel"] * delta
		var nearest := Geometry2D.get_closest_point_to_segment(player, from, hazard["pos"])
		if nearest.distance_to(player) < 18.0:
			take_damage()
			hazard["life"] = 0.0
	hazards = hazards.filter(func(h): return h["life"] > 0.0 and FIELD.grow(30).has_point(h["pos"]))

func take_damage() -> void:
	if invincible > 0.0 or mode != "playing":
		return
	hp -= 1
	invincible = 1.1
	_burst(player, Color("ff626e"))
	_tone(140, 0.12)
	if hp <= 0:
		show_overlay("lost")

func _update_pickups() -> void:
	if not bell_collected and player.distance_to(bell_position) < 30.0:
		bell_collected = true
		_burst(bell_position, Color("ffd56f"))
		_tone(900, 0.16)
		_notify("¡Cascabel encontrado!" if stage < 2 else "¡Escudo roto! Maliketh ya recibe daño.")
	for heart in hearts:
		if hp < MAX_HP and player.distance_to(heart) < 25.0:
			hp = mini(MAX_HP, hp + 2)
			hearts.erase(heart)
			_burst(heart, PINK)
			_tone(780, 0.1)
			break

func gate_open() -> bool:
	return bell_collected and enemies.is_empty()

func _notify(message: String) -> void:
	toast = message
	toast_time = 3.5

func _burst(point: Vector2, color: Color) -> void:
	for i in range(8):
		particles.append({"pos": point, "vel": Vector2.from_angle(i * TAU / 8) * 65,
			"life": 0.35, "color": color})

func _tone(frequency: float, duration: float) -> void:
	if muted:
		return
	var count := int(22050 * duration)
	var bytes := PackedByteArray()
	bytes.resize(count * 2)
	for i in range(count):
		var envelope := (1.0 - float(i) / count) * minf(1, float(i) / 100.0)
		var sample := int(sin(TAU * frequency * i / 22050.0) * 7500 * envelope)
		bytes.encode_s16(i * 2, sample)
	var stream := AudioStreamWAV.new()
	stream.format = AudioStreamWAV.FORMAT_16_BITS
	stream.mix_rate = 22050
	stream.data = bytes
	sound.stream = stream
	sound.play()

func _draw() -> void:
	_draw_world()
	for heart in hearts:
		_text(heart + Vector2(-9, 6), "♥", PINK, 26)
	if not bell_collected:
		_draw_bell(bell_position + Vector2(0, sin(time * 4) * 3))
	if stage == 2:
		_sprite(GIOVANNY, Vector2(824, 230), 0.85)
		if not boss_defeated:
			for i in range(5):
				draw_line(Vector2(790 + i * 17, 165), Vector2(790 + i * 17, 242), Color("9b85ca"), 4)
			draw_rect(Rect2(783, 163, 82, 82), Color("bba7e6"), false, 3)
		elif player.distance_to(Vector2(824, 230)) < 85:
			_text(Vector2(738, 155), "[E] Rescatar", CREAM, 21)
	for enemy in enemies:
		var point: Vector2 = enemy["pos"]
		if enemy["boss"]:
			_sprite(MALIKETH, point, 1.35, Color.WHITE if enemy["flash"] <= 0 else PINK)
			if not bell_collected:
				draw_arc(point - Vector2(0, 25), 48, 0, TAU, 32, Color("b299eb"), 3)
		else:
			_draw_pelusa(point, enemy["flash"] > 0)
	for shot in shots:
		draw_circle(shot["pos"], 6, CREAM)
		draw_circle(shot["pos"], 3, PINK)
	for hazard in hazards:
		draw_circle(hazard["pos"], 10, Color("753258"))
		draw_circle(hazard["pos"], 6, Color("ff677c"))
	_sprite(WIKY, companion, 0.7)
	if invincible <= 0 or int(time * 15) % 2 == 0:
		_sprite(VIVI, player, 0.85)
	if pulse > 0:
		draw_arc(player, 175 * (1 - pulse / 0.45), 0, TAU, 48, Color(0.6, 0.9, 1, pulse / 0.45), 5)
	for effect in particles:
		draw_rect(Rect2(effect["pos"], Vector2(4, 4)), effect["color"])
	_draw_hud()

func _draw_world() -> void:
	var dark := stage == 2
	draw_rect(Rect2(Vector2.ZERO, SIZE), INK)
	draw_rect(Rect2(0, 106, 960, 396), Color("342a4e") if dark else Color("376c54"))
	var ground := Color("51425f") if dark else (Color("527969") if stage == 1 else Color("78aa6b"))
	draw_rect(FIELD, ground)
	for y in range(136, 478, 24):
		for x in range(52, 918, 24):
			var value := (x * 13 + y * 7) % 11
			if value < 5:
				draw_rect(Rect2(x, y, 3, 5), ground.lightened(0.09))
				if not dark:
					draw_line(Vector2(x, y + 5), Vector2(x + 4, y + 1), ground.darkened(0.1), 2)
	var path_color := Color("716077") if dark else Color("d4b78d")
	draw_rect(Rect2(44, 286, 868, 56), path_color)
	for i in range(32):
		draw_rect(Rect2(58 + i * 27, 299 + (i % 3) * 11, 6, 3), path_color.darkened(0.08))
	if dark:
		for x in range(80, 930, 120):
			draw_rect(Rect2(x, 108, 36, 39), Color("302339"))
			draw_rect(Rect2(x, 443, 36, 55), Color("302339"))
			draw_rect(Rect2(x + 7, 114, 6, 20), Color("766581"))
			draw_rect(Rect2(x + 7, 450, 6, 20), Color("766581"))
		_text(Vector2(791, 145), "Giovanny", CREAM, 18)
	else:
		for i in range(10):
			_draw_tree(Vector2(35 + i * 100, 137))
			_draw_tree(Vector2(15 + i * 104, 508))
		for i in range(24):
			var x := 68 + (i * 173) % 820
			var y := 167 + (i * 97) % 286
			var col := PINK if i % 2 == 0 else Color("fbe395")
			draw_rect(Rect2(x, y, 5, 5), col)
			draw_rect(Rect2(x + 1, y + 5, 2, 5), Color("416846"))
	for obstacle in obstacles:
		draw_rect(obstacle.grow(4), Color(0.08, 0.17, 0.16, 0.3))
		draw_rect(obstacle, Color("517f94") if stage == 0 else Color("413d55"))
		draw_rect(Rect2(obstacle.position + Vector2(8, 10), obstacle.size - Vector2(16, 20)), Color("74abb0") if stage == 0 else Color("5b536c"))
	if stage < 2:
		var color := Color("eac577") if gate_open() else Color("6e5875")
		draw_rect(Rect2(856, 254, 57, 116), INK)
		draw_rect(Rect2(862, 260, 45, 104), color)
		if gate_open():
			_text(Vector2(871, 318), "➜", INK, 30)
			_text(Vector2(787, 240), "¡Siguiente!", CREAM, 19)
		else:
			for y in range(270, 357, 17):
				draw_line(Vector2(862, y), Vector2(907, y), INK, 3)

func _draw_tree(point: Vector2) -> void:
	draw_rect(Rect2(point - Vector2(5, 31), Vector2(10, 35)), Color("76543e"))
	draw_rect(Rect2(point - Vector2(28, 54), Vector2(56, 30)), Color("284e45"))
	draw_rect(Rect2(point - Vector2(23, 68), Vector2(46, 30)), Color("376b51"))
	draw_rect(Rect2(point - Vector2(14, 77), Vector2(28, 20)), Color("559266"))

func _draw_bell(point: Vector2) -> void:
	draw_circle(point, 25 + sin(time * 3) * 2, Color(1, 0.85, 0.4, 0.13))
	draw_rect(Rect2(point - Vector2(6, 12), Vector2(12, 6)), Color("ffeeac"))
	draw_rect(Rect2(point - Vector2(10, 6), Vector2(20, 18)), Color("ffd36f"))
	draw_rect(Rect2(point - Vector2(14, -7), Vector2(28, 5)), Color("ffecaa"))
	draw_circle(point + Vector2(0, 15), 4, Color("b98549"))

func _draw_pelusa(point: Vector2, flash: bool) -> void:
	draw_ellipse_shadow(point, 21)
	var color := PINK if flash else Color("634969")
	draw_rect(Rect2(point - Vector2(20, 31), Vector2(40, 29)), color)
	draw_rect(Rect2(point - Vector2(13, 39), Vector2(26, 38)), color)
	draw_rect(Rect2(point - Vector2(11, 26), Vector2(6, 6)), Color("ffe39f"))
	draw_rect(Rect2(point + Vector2(5, -26), Vector2(6, 6)), Color("ffe39f"))
	draw_rect(Rect2(point - Vector2(5, 13), Vector2(10, 3)), INK)

func draw_ellipse_shadow(point: Vector2, radius: float) -> void:
	draw_set_transform(point, 0, Vector2(1, 0.3))
	draw_circle(Vector2.ZERO, radius, Color(0.06, 0.05, 0.12, 0.28))
	draw_set_transform(Vector2.ZERO)

func _sprite(texture: Texture2D, feet: Vector2, scale_value: float, tint: Color = Color.WHITE) -> void:
	var sprite_size := texture.get_size() * scale_value
	draw_ellipse_shadow(feet, sprite_size.x * 0.33)
	draw_texture_rect(texture, Rect2(feet - Vector2(sprite_size.x / 2, sprite_size.y), sprite_size), false, tint)

func _draw_hud() -> void:
	draw_rect(Rect2(0, 0, 960, 104), INK)
	_text(Vector2(28, 37), "SORPRESA VIVI’S", PINK, 25)
	_text(Vector2(28, 65), "%d / 3   %s" % [stage + 1, ZONES[stage]], CREAM, 19)
	var objective := "Cascabel: %s   ·   Pelusas: %d   ·   Salida a la derecha" % ["✓" if bell_collected else "pendiente", enemies.size()]
	if stage == 2:
		objective = "Acércate a Giovanny y pulsa E" if boss_defeated else ("Derrota a Maliketh" if bell_collected else "Recoge el cascabel dorado para romper el escudo")
	_text(Vector2(28, 91), objective, Color("c6bcdb"), 17)
	_text(Vector2(623, 36), "VIDA", CREAM, 15)
	for i in range(MAX_HP):
		_text(Vector2(681 + i * 34, 41), "♥", PINK if i < hp else Color("54475f"), 28)
	var help_text := "E  Wiky: listo" if companion_cooldown <= 0 else "E  Wiky: %d s" % ceili(companion_cooldown)
	_text(Vector2(623, 67), help_text, Color("a7e9e3"), 18)
	_text(Vector2(823, 89), "%d:%02d" % [int(run_time) / 60, int(run_time) % 60], CREAM, 18)
	if stage == 2 and not enemies.is_empty():
		draw_rect(Rect2(330, 112, 300, 8), Color("32223e"))
		draw_rect(Rect2(330, 112, 300 * maxf(0, enemies[0]["hp"]) / 30.0, 8), PINK)
	if toast_time > 0:
		draw_rect(Rect2(115, 452, 730, 30), Color(0.09, 0.08, 0.17, 0.92))
		var font := ThemeDB.fallback_font
		var width := font.get_string_size(toast, HORIZONTAL_ALIGNMENT_LEFT, -1, 17).x
		_text(Vector2((960 - width) / 2, 473), toast, CREAM, 17)
	draw_rect(Rect2(0, 504, 960, 36), INK)
	_text(Vector2(28, 528), "WASD / flechas  Mover      Espacio  Disparar      E  Wiky      Esc  Pausa", CREAM, 16)
	_text(Vector2(813, 528), "M  " + ("Sin sonido" if muted else "Sonido"), Color("c6bcdb"), 15)

func _text(point: Vector2, value: String, color: Color, font_size: int) -> void:
	draw_string(ThemeDB.fallback_font, point, value, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, color)
