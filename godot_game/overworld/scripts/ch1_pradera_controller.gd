extends Node2D

@onready var welcome_label: Label = $HUD/WelcomeTone
@onready var progress_label: Label = $HUD/ProgressLabel

var _objectives: Dictionary = {}

func _ready() -> void:
	_load_objectives()
	GameState.set_zone("Pradera de Bigotes")
	if GameState.world_flags.get("ch1_completed", false):
		GameState.set_objective(_obj("completed"))
	if not GameState.world_flags.get("ch1_guardian_defeated", false):
		GameState.set_objective(_obj("start"))
	welcome_label.text = "Pradera de Bigotes · Bienvenida mullida, rareza en el aire"
	EventBus.dialogue_finished.connect(_on_dialogue_finished)
	if not GameState.world_flags.get("ch1_overworld_intro_seen", false):
		GameState.mark_flag("ch1_overworld_intro_seen", true)
		EventBus.request_dialogue("ch1_overworld_intro")
	if GameState.world_flags.get("ch1_guardian_defeated", false) and not GameState.world_flags.get("ch1_resolution_seen", false):
		GameState.mark_flag("ch1_resolution_seen", true)
		EventBus.request_dialogue("ch1_resolution")
	_update_progress()

func _exit_tree() -> void:
	if EventBus.dialogue_finished.is_connected(_on_dialogue_finished):
		EventBus.dialogue_finished.disconnect(_on_dialogue_finished)

func _on_dialogue_finished(dialogue_id: String) -> void:
	if dialogue_id == "ch1_overworld_intro":
		GameState.set_objective(_obj("start"))
		_update_progress()
	if dialogue_id == "ch1_clue":
		GameState.set_objective(_obj("after_clue"))
		_update_progress()
	if dialogue_id == "ch1_resolution":
		GameState.mark_flag("ch1_completed", true)
		GameState.mark_flag("item_resonancia_cascabel", true)
		GameState.set_objective(_obj("completed"))
		_update_progress()

func _load_objectives() -> void:
	var file := FileAccess.open("res://data/events/ch1_objectives.json", FileAccess.READ)
	if file == null:
		return
	var parsed: Variant = JSON.parse_string(file.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		return
	_objectives = (parsed as Dictionary).get("ch1", {})

func _obj(key: String) -> String:
	return str(_objectives.get(key, ""))

func _update_progress() -> void:
	var explored := GameState.world_flags.get("ch1_clue_seen", false)
	var fought := GameState.world_flags.get("ch1_guardian_defeated", false)
	var closed := GameState.world_flags.get("ch1_completed", false)
	progress_label.text = "Progreso CH1 · Pista:%s · Combate:%s · Cierre:%s" % [_mark(explored), _mark(fought), _mark(closed)]

func _mark(value: bool) -> String:
	return "✓" if value else "·"
