extends SceneTree
## Prueba de integración sin plugins. Cualquier fallo termina con código 1.

var failures := 0
var checks := 0

func _init() -> void:
	call_deferred("run")

func expect(condition: bool, label: String) -> void:
	checks += 1
	if not condition:
		failures += 1
		push_error("FAIL: " + label)

func run() -> void:
	var scene := load("res://playable/game.tscn") as PackedScene
	var game = scene.instantiate()
	root.add_child(game)
	await process_frame
	game.set_physics_process(false)
	game.muted = true
	expect(game.mode == "menu", "Arranque en menú")
	game.play_button.pressed.emit()
	expect(game.mode == "playing" and game.hp == 6, "Botón Jugar inicia partida")

	# Movimiento real, diagonales normalizadas y obstáculos sólidos.
	var start: Vector2 = game.player
	Input.action_press("walk_right")
	game._physics_process(0.1)
	Input.action_release("walk_right")
	expect(is_equal_approx(game.player.x - start.x, 20.5), "Movimiento independiente de FPS")
	game.player = start
	Input.action_press("walk_right")
	Input.action_press("walk_up")
	game._physics_process(0.1)
	Input.action_release("walk_right")
	Input.action_release("walk_up")
	expect(is_equal_approx(game.player.distance_to(start), 20.5), "Diagonal sin velocidad extra")
	expect(game.move_actor(Vector2(260, 220), Vector2(15, 0), 14).x == 260, "Colisión con estanque")
	expect(game.move_actor(Vector2(56, 300), Vector2(-30, 0), 14).x == 56, "Límite de mapa")
	game.player = Vector2(884, 310)
	game._physics_process(0.016)
	expect(game.stage == 0, "Salida cerrada antes de completar objetivos")

	# Pausa detiene daño, proyectiles, tiempo y recargas.
	game.show_overlay("paused")
	var elapsed: float = game.run_time
	var life: int = game.hp
	game._physics_process(5.0)
	game.take_damage()
	expect(game.run_time == elapsed and game.hp == life, "Pausa congela partida")
	game.play_button.pressed.emit()
	game.load_stage(0)
	game.invincible = 0
	game.take_damage()
	game.take_damage()
	expect(game.hp == 5, "Invulnerabilidad evita daño múltiple inmediato")
	game.player = game.hearts[0]
	game._update_pickups()
	expect(game.hp == 6 and game.hearts.is_empty(), "Corazón cura y se consume")

	# Dos mapas completos con proyectiles reales, cascabel y transición.
	for index in range(2):
		game.load_stage(index)
		game.player = game.bell_position
		game._update_pickups()
		expect(game.bell_collected, "Cascabel de mapa %d" % index)
		var safety := 0
		while not game.enemies.is_empty() and safety < 30:
			var target: Vector2 = game.enemies[0]["pos"]
			game.player = target - Vector2(70, 0)
			game.attack_cooldown = 0
			game.shoot()
			game._update_shots(0.12)
			safety += 1
		expect(game.enemies.is_empty(), "Enemigos derrotables con disparos mapa %d" % index)
		expect(game.gate_open(), "Salida desbloqueada mapa %d" % index)
		game.player = Vector2(884, 310)
		game._physics_process(0.016)
		expect(game.stage == index + 1 and game.hp == 6, "Transición con vida restaurada")

	# Escudo, ataque del jefe, compañero, rescate y final.
	var boss: Dictionary = game.enemies[0]
	game._damage_enemy(boss, 1)
	expect(boss["hp"] == 30, "Escudo bloquea daño antes del cascabel")
	game.player = game.bell_position
	game._update_pickups()
	boss["timer"] = 0
	game._update_enemies(0.016)
	expect(game.hazards.size() == 3, "Maliketh lanza pelusa en abanico")
	game.hazards.assign([{"pos": game.player - Vector2(20, 0), "vel": Vector2(180, 0), "life": 1.0}])
	game.invincible = 0
	game._update_hazards(0.2)
	expect(game.hp == 5, "Proyectil enemigo daña aun cruzando al jugador")
	game.player = boss["pos"] - Vector2(60, 0)
	game.call_companion()
	expect(boss["hp"] == 27 and game.companion_cooldown == 5, "Wiky inflige daño")
	game.call_companion()
	expect(boss["hp"] == 27, "Recarga impide repetir habilidad")
	for i in range(27):
		game.attack_cooldown = 0
		game.shoot()
		game._update_shots(0.1)
	expect(game.boss_defeated and game.enemies.is_empty(), "Jefe derrotado por combate")
	expect(game.hazards.is_empty(), "Victoria del jefe limpia amenazas")
	game.player = Vector2(824, 230)
	game.call_companion()
	expect(game.mode == "won" and game.menu_layer.visible, "Rescate muestra final feliz")
	game.play_button.pressed.emit()
	expect(game.stage == 0 and game.mode == "playing" and not game.boss_defeated, "Nueva partida limpia progreso")

	# Derrota y checkpoint repetible, sin salir del juego.
	game.load_stage(1)
	for i in range(6):
		game.invincible = 0
		game.take_damage()
	expect(game.mode == "lost", "Derrota muestra reintento")
	game.play_button.pressed.emit()
	expect(game.mode == "playing" and game.stage == 1 and game.hp == 6, "Reintento conserva mapa y restaura vida")
	game._notification(Node.NOTIFICATION_APPLICATION_FOCUS_OUT)
	expect(game.mode == "paused", "Pausa automática al cambiar de ventana")
	print("PLAYTHROUGH: %d checks, %d failures" % [checks, failures])
	game.queue_free()
	await process_frame
	quit(1 if failures else 0)
