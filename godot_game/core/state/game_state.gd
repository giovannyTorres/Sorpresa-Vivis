extends Node

signal save_dirty()
signal chapter_changed(chapter_id: String)
signal objective_changed(objective_text: String)
signal zone_changed(zone_name: String)
signal banner_requested(title: String, subtitle: String, tone: String)

var current_chapter: String = "ch1"
var current_chapter_name: String = ""
var chapter_context: String = ""
var current_objective: String = ""
var current_objective_id: String = ""
var current_zone_name: String = ""
var narrative_locked: bool = false
var player_party: Dictionary = {}
var world_flags: Dictionary = {}
var combat_context: Dictionary = {}

var _chapter_flow_data: Dictionary = {}
var _chapter_objectives: Dictionary = {}

func reset_for_new_game() -> void:
	current_chapter = "ch1"
	current_chapter_name = ""
	chapter_context = ""
	current_objective = ""
	current_objective_id = ""
	current_zone_name = ""
	player_party = {
		"vivis": {"hp": 100, "max_hp": 100, "attack_ids": ["rafaga_punos_tiernos"]},
		"wiky_wikerman": {"hp": 100, "max_hp": 100, "attack_ids": ["aranazo_ponzonoso"]}
	}
	world_flags = {
		"intro_capture_seen": false,
		"giovanny_rescued": false,
		"ch1_overworld_intro_seen": false,
		"ch1_clue_seen": false,
		"ch1_combat_started": false,
		"ch1_guardian_defeated": false,
		"ch1_resolution_seen": false,
		"ch1_completed": false,
		"ch1_transition_seen": false,
		"ch1_exit_to_ch2_open": false,
		"ch2_started": false,
		"ch2_intro_seen": false,
		"ch2_echo_seen": false,
		"item_resonancia_cascabel": false
	}
	combat_context = {}
	narrative_locked = false
	_ensure_chapter_data()
	load_chapter("ch1")
	save_dirty.emit()

func enter_combat(payload: Dictionary) -> void:
	combat_context = payload.duplicate(true)

func exit_combat() -> void:
	combat_context.clear()

func mark_flag(flag_name: String, value: Variant = true) -> void:
	world_flags[flag_name] = value
	save_dirty.emit()

func set_chapter(chapter_id: String) -> void:
	current_chapter = chapter_id
	chapter_changed.emit(chapter_id)
	save_dirty.emit()

func load_chapter(chapter_id: String) -> void:
	_ensure_chapter_data()
	var chapter_flow := get_chapter_flow(chapter_id)
	var path := str(chapter_flow.get("chapter_path", ""))
	if path.is_empty():
		path = "res://data/chapters/%s.json" % chapter_id
	load_chapter_data(path)

func load_chapter_data(path: String) -> void:
	var chapter_data := _load_json(path)
	if chapter_data.is_empty():
		push_warning("No se pudo abrir capitulo: %s" % path)
		return
	current_chapter = str(chapter_data.get("id", "ch1"))
	current_chapter_name = str(chapter_data.get("name", current_chapter.to_upper()))
	chapter_context = str(chapter_data.get("context", ""))
	chapter_changed.emit(current_chapter)
	set_objective(str(chapter_data.get("main_objective", "")))
	save_dirty.emit()

func get_chapter_flow(chapter_id: String) -> Dictionary:
	_ensure_chapter_data()
	return _chapter_flow_data.get(chapter_id, {})

func get_objective_text(chapter_id: String, objective_id: String) -> String:
	_ensure_chapter_data()
	var chapter_objectives := _chapter_objectives.get(chapter_id, {})
	return str(chapter_objectives.get(objective_id, ""))

func set_objective_by_id(chapter_id: String, objective_id: String) -> void:
	current_objective_id = objective_id
	set_objective(get_objective_text(chapter_id, objective_id))

func set_objective(objective_text: String) -> void:
	current_objective = objective_text
	objective_changed.emit(objective_text)
	save_dirty.emit()

func set_zone(zone_name: String) -> void:
	current_zone_name = zone_name
	zone_changed.emit(zone_name)
	save_dirty.emit()

func set_narrative_locked(value: bool) -> void:
	narrative_locked = value

func push_banner(title: String, subtitle: String = "", tone: String = "info") -> void:
	banner_requested.emit(title, subtitle, tone)

func _ensure_chapter_data() -> void:
	if _chapter_flow_data.is_empty():
		var flow_root := _load_json("res://data/events/chapter_flow.json")
		_chapter_flow_data = flow_root.get("chapters", {})
	if _chapter_objectives.is_empty():
		_chapter_objectives = _load_json("res://data/events/chapter_objectives.json")

func _load_json(path: String) -> Dictionary:
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		return {}
	var parsed: Variant = JSON.parse_string(file.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		return {}
	return parsed as Dictionary
