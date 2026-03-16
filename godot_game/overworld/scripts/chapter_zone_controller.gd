extends Node2D
class_name ChapterZoneController

@export var chapter_id: String = "ch1"

@onready var welcome_label: Label = $HUD/WelcomeTone
@onready var progress_label: Label = $HUD/ProgressLabel
@onready var hint_label: Label = $HUD/ZoneHint

var _flow: Dictionary = {}

func _ready() -> void:
	_flow = GameState.get_chapter_flow(chapter_id)
	GameState.load_chapter(chapter_id)
	GameState.set_zone(str(_flow.get("zone_name", GameState.current_zone_name)))
	welcome_label.text = str(_flow.get("welcome_text", ""))
	hint_label.text = str(_flow.get("hint_text", ""))
	EventBus.dialogue_finished.connect(_on_dialogue_finished)
	_mark_started()
	_try_start_dialogue()
	_try_completion_dialogue()
	_sync_objective()
	_update_progress()

func _exit_tree() -> void:
	if EventBus.dialogue_finished.is_connected(_on_dialogue_finished):
		EventBus.dialogue_finished.disconnect(_on_dialogue_finished)

func _on_dialogue_finished(dialogue_id: String) -> void:
	if dialogue_id == str(_flow.get("entry_dialogue", "")):
		GameState.push_banner(
			"Capitulo %s" % chapter_id.to_upper(),
			GameState.current_chapter_name,
			"chapter"
		)
	elif dialogue_id == str(_flow.get("completion_dialogue", "")):
		_apply_completion()
		var transition_dialogue := str(_flow.get("transition_dialogue", ""))
		if not transition_dialogue.is_empty():
			EventBus.request_dialogue(transition_dialogue)
	elif dialogue_id == str(_flow.get("transition_dialogue", "")):
		var transition_seen_flag := str(_flow.get("transition_seen_flag", ""))
		if not transition_seen_flag.is_empty():
			GameState.mark_flag(transition_seen_flag, true)
		var exit_flag := str(_flow.get("exit_unlock_flag", ""))
		if not exit_flag.is_empty():
			GameState.mark_flag(exit_flag, true)
			GameState.push_banner("Nuevo destino", "La ruta hacia CH2 ya puede cruzarse.", "transition")
	_sync_objective()
	_update_progress()

func _mark_started() -> void:
	var started_flag := str(_flow.get("chapter_started_flag", ""))
	if started_flag.is_empty():
		return
	if GameState.world_flags.get(started_flag, false):
		return
	GameState.mark_flag(started_flag, true)

func _try_start_dialogue() -> void:
	var entry_flag := str(_flow.get("entry_flag", ""))
	var entry_dialogue := str(_flow.get("entry_dialogue", ""))
	if entry_flag.is_empty() or entry_dialogue.is_empty():
		return
	if GameState.world_flags.get(entry_flag, false):
		return
	GameState.mark_flag(entry_flag, true)
	EventBus.request_dialogue(entry_dialogue)

func _try_completion_dialogue() -> void:
	var completion_trigger_flag := str(_flow.get("completion_trigger_flag", ""))
	var completion_seen_flag := str(_flow.get("completion_seen_flag", ""))
	var completion_dialogue := str(_flow.get("completion_dialogue", ""))
	if completion_trigger_flag.is_empty() or completion_dialogue.is_empty():
		return
	if not GameState.world_flags.get(completion_trigger_flag, false):
		return
	if not completion_seen_flag.is_empty() and GameState.world_flags.get(completion_seen_flag, false):
		return
	if not completion_seen_flag.is_empty():
		GameState.mark_flag(completion_seen_flag, true)
	EventBus.request_dialogue(completion_dialogue)

func _apply_completion() -> void:
	for reward_flag in _flow.get("completion_rewards", []):
		GameState.mark_flag(str(reward_flag), true)
	GameState.push_banner(
		"%s completado" % chapter_id.to_upper(),
		"La aventura avanza hacia el siguiente destino.",
		"success"
	)

func _sync_objective() -> void:
	var objective_states: Array = _flow.get("objective_states", [])
	for state_data in objective_states:
		var state := state_data as Dictionary
		if _matches_state(state):
			GameState.set_objective_by_id(chapter_id, str(state.get("objective_id", "")))

func _matches_state(state: Dictionary) -> bool:
	for flag_name in state.get("require_flags", []):
		if not GameState.world_flags.get(str(flag_name), false):
			return false
	for flag_name in state.get("block_flags", []):
		if GameState.world_flags.get(str(flag_name), false):
			return false
	return true

func _update_progress() -> void:
	var chunks: Array[String] = []
	for progress_data in _flow.get("progress_flags", []):
		var progress := progress_data as Dictionary
		var label := str(progress.get("label", "Paso"))
		var flag_name := str(progress.get("flag", ""))
		chunks.append("%s:%s" % [label, _mark(GameState.world_flags.get(flag_name, false))])
	progress_label.text = "%s · %s" % [chapter_id.to_upper(), " · ".join(chunks)]

func _mark(value: bool) -> String:
	return "OK" if value else "--"
