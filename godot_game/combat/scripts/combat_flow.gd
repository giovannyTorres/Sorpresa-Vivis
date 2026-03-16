extends Node2D

const DEFAULT_RETURN_SCENE := "res://overworld/scenes/map_pradera_bigotes.tscn"
const PARTY_ORDER := ["vivis", "wiky_wikerman", "enemy"]

@onready var turn_manager: TurnManager = $TurnManager
@onready var action_resolver: ActionResolver = $ActionResolver
@onready var enemy_ai: EnemyAI = $EnemyAI

@onready var title_label: Label = $CanvasLayer/HeaderPanel/VBox/Title
@onready var status_label: Label = $CanvasLayer/HeaderPanel/VBox/TurnStatus
@onready var summary_label: Label = $CanvasLayer/Summary

@onready var vivis_hp_label: Label = $CanvasLayer/PartyPanel/VBox/VivisCard/VivisHP
@onready var wiky_hp_label: Label = $CanvasLayer/PartyPanel/VBox/WikyCard/WikyHP
@onready var enemy_hp_label: Label = $CanvasLayer/EnemyPanel/VBox/EnemyCard/EnemyHP

@onready var vivis_attack_button: Button = $CanvasLayer/ActionPanel/VBox/VivisAttackButton
@onready var wiky_attack_button: Button = $CanvasLayer/ActionPanel/VBox/WikyAttackButton

@onready var log_label: RichTextLabel = $CanvasLayer/LogPanel/VBox/LogText

var _actors: Dictionary = {}
var _attack_data: Dictionary = {}
var _enemy_id: String = "pelusa_corrupta"
var _combat_ended: bool = false

func _ready() -> void:
	_enemy_id = str(GameState.combat_context.get("enemy_id", "pelusa_corrupta"))
	_load_combat_data()
	_wire_signals()
	_refresh_labels()
	turn_manager.start_combat(PARTY_ORDER)

func _wire_signals() -> void:
	turn_manager.turn_started.connect(_on_turn_started)
	turn_manager.combat_finished.connect(_on_combat_finished)
	vivis_attack_button.pressed.connect(_on_vivis_attack_pressed)
	wiky_attack_button.pressed.connect(_on_wiky_attack_pressed)

func _load_combat_data() -> void:
	_actors = {
		"vivis": _build_party_actor("res://data/actors/vivis.json", "rafaga_punos_tiernos"),
		"wiky_wikerman": _build_party_actor("res://data/actors/wiky_wikerman.json", "aranazo_ponzonoso"),
		"enemy": _build_enemy_actor(_enemy_id)
	}
	var ids: Array[String] = []
	for actor_id in _actors.keys():
		for attack_id in _actors[actor_id].get("attack_ids", []):
			ids.append(str(attack_id))
	for attack_id in ids:
		_attack_data[attack_id] = GameState.read_json("res://data/attacks/%s.json" % attack_id)
	_sync_action_labels()

func _build_party_actor(path: String, fallback_attack: String) -> Dictionary:
	var data := GameState.read_json(path)
	return {
		"id": str(data.get("id", "")),
		"name": str(data.get("display_name", data.get("id", "Actor"))),
		"short_name": str(data.get("short_name", data.get("display_name", data.get("id", "Actor")))),
		"hp": int(data.get("max_hp", 100)),
		"max_hp": int(data.get("max_hp", 100)),
		"attack_ids": data.get("attacks", [fallback_attack])
	}

func _build_enemy_actor(enemy_id: String) -> Dictionary:
	var data := GameState.read_json("res://data/actors/%s.json" % enemy_id)
	var enemy_name := str(data.get("display_name", data.get("name", enemy_id.capitalize())))
	return {
		"id": enemy_id,
		"name": enemy_name,
		"short_name": str(data.get("short_name", enemy_name)),
		"hp": int(data.get("max_hp", data.get("hp", 50))),
		"max_hp": int(data.get("max_hp", data.get("hp", 50))),
		"attack_ids": data.get("attack_ids", ["pelotazo_pelusa"])
	}

func _on_turn_started(actor_id: String) -> void:
	if _combat_ended:
		return
	var actor_name := _actors[actor_id].get("short_name", actor_id)
	status_label.text = "Turno actual: %s" % actor_name
	vivis_attack_button.disabled = actor_id != "vivis"
	wiky_attack_button.disabled = actor_id != "wiky_wikerman"
	if actor_id == "enemy":
		_run_enemy_turn()

func _on_vivis_attack_pressed() -> void:
	_player_attack("vivis")

func _on_wiky_attack_pressed() -> void:
	_player_attack("wiky_wikerman")

func _player_attack(actor_id: String) -> void:
	if _combat_ended:
		return
	var attack_id := str((_actors[actor_id].get("attack_ids", [""]) as Array)[0])
	_resolve_attack(actor_id, "enemy", attack_id)

func _run_enemy_turn() -> void:
	await get_tree().create_timer(0.45).timeout
	if _combat_ended:
		return
	var party_state := {
		"vivis": _actors["vivis"],
		"wiky_wikerman": _actors["wiky_wikerman"]
	}
	var action := enemy_ai.choose_action(_actors["enemy"], party_state)
	var target_id := str(action.get("target_id", "vivis"))
	var attack_id := str(action.get("attack_id", "pelotazo_pelusa"))
	_resolve_attack("enemy", target_id, attack_id)

func _resolve_attack(attacker_id: String, defender_id: String, attack_id: String) -> void:
	var attacker := _actors[attacker_id]
	var defender := _actors[defender_id]
	var attack := _attack_data.get(attack_id, {"display_name": attack_id, "damage_min": 5, "damage_max": 8})
	var result := action_resolver.resolve_attack(attacker, defender, attack)
	var line := "%s usa %s y causa %s de dano a %s." % [
		attacker.get("short_name", attacker_id),
		attack.get("display_name", attack_id),
		result.get("damage", 0),
		defender.get("short_name", defender_id)
	]
	_append_log(line)
	var finished := _evaluate_combat_state()
	turn_manager.resolve_current_turn({"combat_finished": finished, "victory": _actors["enemy"].get("hp", 1) <= 0})
	_refresh_labels()

func _evaluate_combat_state() -> bool:
	if _actors["enemy"].get("hp", 0) <= 0:
		return true
	if _actors["vivis"].get("hp", 0) <= 0 and _actors["wiky_wikerman"].get("hp", 0) <= 0:
		return true
	return false

func _on_combat_finished(victory: bool) -> void:
	_combat_ended = true
	vivis_attack_button.disabled = true
	wiky_attack_button.disabled = true
	if victory:
		status_label.text = "Combate resuelto"
		_append_log("Victoria. La Pelusa Corrupta se disipa.")
		_resolve_victory()
		return
	status_label.text = "Retirada temporal"
	_append_log("Derrota temporal. Vivis y Wiky retroceden para reagruparse.")
	_resolve_defeat()

func _resolve_victory() -> void:
	var chapter_id := str(GameState.combat_context.get("chapter_id", ""))
	if chapter_id == "ch1":
		GameState.mark_flag("ch1_guardian_defeated", true)
		GameState.mark_flag("ch1_progress_gate_unlocked", true)
		GameState.set_objective_by_id("ch1", "after_combat")
	GameState.queue_banner("Victoria", "El guardian del altar ha sido purgado.", "success")
	await get_tree().create_timer(0.9).timeout
	var return_scene := str(GameState.combat_context.get("return_scene", DEFAULT_RETURN_SCENE))
	SceneRouter.return_to_overworld(return_scene)

func _resolve_defeat() -> void:
	GameState.mark_flag("ch1_guardian_defeated", false)
	GameState.set_objective("Recupera fuerzas e intentalo de nuevo en el altar.")
	GameState.queue_banner("Retirada temporal", "Vuelve al altar cuando el equipo este listo.", "warning")
	await get_tree().create_timer(0.9).timeout
	var return_scene := str(GameState.combat_context.get("return_scene", DEFAULT_RETURN_SCENE))
	SceneRouter.return_to_overworld(return_scene)

func _append_log(text: String) -> void:
	if log_label.text.is_empty():
		log_label.text = text
	else:
		log_label.text += "\n%s" % text
	log_label.scroll_to_line(log_label.get_line_count())

func _refresh_labels() -> void:
	var combat_chapter := str(GameState.combat_context.get("chapter_id", GameState.current_chapter))
	title_label.text = "Combate - %s" % GameState.get_chapter_label(combat_chapter)
	summary_label.text = "Objetivo del encuentro: despejar el paso y volver al mapa sin perder el avance."
	vivis_hp_label.text = "%s - %s / %s HP" % [
		_actors["vivis"].get("short_name", "Vivis"),
		_actors["vivis"].get("hp", 0),
		_actors["vivis"].get("max_hp", 0)
	]
	wiky_hp_label.text = "%s - %s / %s HP" % [
		_actors["wiky_wikerman"].get("short_name", "Wiky"),
		_actors["wiky_wikerman"].get("hp", 0),
		_actors["wiky_wikerman"].get("max_hp", 0)
	]
	enemy_hp_label.text = "%s - %s / %s HP" % [
		_actors["enemy"].get("short_name", "Enemigo"),
		_actors["enemy"].get("hp", 0),
		_actors["enemy"].get("max_hp", 0)
	]
	if log_label.text.is_empty():
		log_label.text = "El combate comienza. Aprovecha cada turno para sostener el avance del capitulo."

func _sync_action_labels() -> void:
	vivis_attack_button.text = _format_action_label("vivis")
	wiky_attack_button.text = _format_action_label("wiky_wikerman")

func _format_action_label(actor_id: String) -> String:
	var actor := _actors.get(actor_id, {})
	var attack_id := str((actor.get("attack_ids", [""]) as Array)[0])
	var attack := _attack_data.get(attack_id, {})
	return "%s - %s" % [
		actor.get("short_name", actor_id),
		attack.get("display_name", attack_id)
	]
