extends Node2D
## Aventura completa: tres pantallas, dos controles de combate y un rescate.

const SIZE := Vector2(960, 540)
const FIELD := Rect2(42, 126, 876, 352)
const SPEED := 205.0
const MAX_HP := 6
const ZONES := ["Pradera de Bigotes", "Sendero del Umbral", "Torre de Maliketh"]
const CharacterArt := preload("res://playable/pixel_characters.gd")
const Effects := preload("res://playable/pixel_effects.gd")
const Interface := preload("res://playable/pixel_interface.gd")
const Scenery := preload("res://playable/pixel_scenery.gd")
const PIXEL_FONT := preload("res://playable/assets/pixel_font.ttf")
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
var character_atlas: Texture2D
var moving := false
var companion_moving := false
var cast_remaining := 0.0
var companion_cast_remaining := 0.0
var companion_faces_left := false
var hurt_remaining := 0.0

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	character_atlas = load(CharacterArt.ATLAS_PATH) as Texture2D
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
	var widgets: Dictionary = Interface.build_menu(self)
	menu_layer = widgets["menu_layer"]
	heading = widgets["heading"]
	description = widgets["description"]
	play_button = widgets["play_button"]
	menu_button = widgets["menu_button"]
	quit_button = widgets["quit_button"]

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
	for portrait in menu_layer.find_children("CharacterPortrait*", "TextureRect", true, false):
		portrait.texture = CharacterArt.portrait(character_atlas, portrait.get_meta("character_row"), mode == "won")
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
	moving = false
	companion_moving = false
	cast_remaining = 0.0
	companion_cast_remaining = 0.0
	hurt_remaining = 0.0
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
	cast_remaining = maxf(0.0, cast_remaining - delta)
	companion_cast_remaining = maxf(0.0, companion_cast_remaining - delta)
	hurt_remaining = maxf(0.0, hurt_remaining - delta)
	var direction := Input.get_vector("walk_left", "walk_right", "walk_up", "walk_down")
	moving = direction.length_squared() > 0.0
	if direction.length_squared() > 0.0:
		facing = direction
		player = move_actor(player, direction * SPEED * delta, 14.0)
	var follow := player - facing * 48.0 + Vector2(0, 18)
	companion_moving = companion.distance_to(follow) > 4.0
	if absf(follow.x - companion.x) > 2.0:
		companion_faces_left = follow.x < companion.x
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
	cast_remaining = 0.17
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
	companion_cast_remaining = 0.5
	pulse = 0.45
	for enemy in enemies:
		if player.distance_to(enemy["pos"]) < 175.0:
			_damage_enemy(enemy, 3)
	_remove_dead_enemies()
	_tone(350, 0.14)
	_notify("¡Wiky libera su magia digital!")

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
	hurt_remaining = 0.4
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
	Scenery.draw_world(self, stage, time, obstacles, gate_open())
	for heart in hearts:
		Effects.heart(self, heart, 2.5)
	if not bell_collected:
		_draw_bell(bell_position + Vector2(0, sin(time * 4) * 3))
	if stage == 2:
		CharacterArt.draw_character(self, character_atlas, 2, Vector2(824, 245), 90)
		if not boss_defeated:
			for i in range(5):
				draw_rect(Rect2(782 + i * 19, 150, 3, 99), Color("87739e"))
				draw_rect(Rect2(778 + i * 19, 150, 1, 99), Color("cfb9e4"))
			draw_rect(Rect2(773, 148, 94, 4), Color("c0a7d4"))
			draw_rect(Rect2(773, 246, 94, 4), Color("5a436e"))
		elif player.distance_to(Vector2(824, 230)) < 85:
			_text(Vector2(747, 140), "[E] RESCATAR", CREAM, 18)
	for enemy in enemies:
		var point: Vector2 = enemy["pos"]
		if enemy["boss"]:
			var frame := 4 if enemy["flash"] > 0 else (3 if enemy["timer"] > 1.4 else (1 + int(time * 5) % 2))
			CharacterArt.draw_character(self, character_atlas, 3, point, 88, frame, true,
				Color.WHITE if enemy["flash"] <= 0 else PINK)
			if not bell_collected:
				Effects.lunar_shield(self, point - Vector2(0, 32), time)
		else:
			_draw_pelusa(point, enemy["flash"] > 0)
	for shot in shots:
		Effects.pink_magic(self, shot["pos"], shot["vel"], time)
	for hazard in hazards:
		Effects.lunar_magic(self, hazard["pos"], hazard["vel"], time)
	var walk_frame := 1 + int(time * 7) % 2
	var wiky_frame := 3 if companion_cast_remaining > 0 else (walk_frame if companion_moving else 0)
	CharacterArt.draw_character(self, character_atlas, 1, companion, 68, wiky_frame, companion_faces_left)
	if invincible <= 0 or int(time * 15) % 2 == 0:
		var frame := 4 if hurt_remaining > 0 else (3 if cast_remaining > 0 else (walk_frame if moving else 0))
		CharacterArt.draw_character(self, character_atlas, 0, player, 92, frame, facing.x < -0.1)
	if pulse > 0:
		Effects.digital_pulse(self, player, 1 - pulse / 0.45)
	for effect in particles:
		Effects.sparkle(self, effect["pos"].round(), effect["color"], 1)
	Interface.draw_hud(self, {
		"stage": stage, "hp": hp, "max_hp": MAX_HP,
		"bell_collected": bell_collected, "boss_defeated": boss_defeated,
		"enemies": enemies.size(), "boss_hp": enemies[0]["hp"] if stage == 2 and not enemies.is_empty() else 0,
		"companion_cooldown": companion_cooldown, "run_time": run_time,
		"toast": toast, "toast_time": toast_time, "muted": muted
	})

func _draw_bell(point: Vector2) -> void:
	draw_circle(point, 25 + sin(time * 3) * 2, Color(1, 0.85, 0.4, 0.13))
	draw_rect(Rect2(point - Vector2(6, 12), Vector2(12, 6)), Color("ffeeac"))
	draw_rect(Rect2(point - Vector2(10, 6), Vector2(20, 18)), Color("ffd36f"))
	draw_rect(Rect2(point - Vector2(14, -7), Vector2(28, 5)), Color("ffecaa"))
	draw_circle(point + Vector2(0, 15), 4, Color("b98549"))

func _draw_pelusa(point: Vector2, flash: bool) -> void:
	Effects.pelusa(self, point, flash, time)

func _text(point: Vector2, value: String, color: Color, font_size: int) -> void:
	draw_string(PIXEL_FONT, point, value, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, color)
