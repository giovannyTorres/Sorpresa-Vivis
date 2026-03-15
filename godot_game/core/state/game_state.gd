extends Node

signal save_dirty()
signal chapter_changed(chapter_id: String)
signal objective_changed(objective_text: String)
signal zone_changed(zone_name: String)

var current_chapter: String = "ch1"
var current_chapter_name: String = ""
var chapter_context: String = ""
var current_objective: String = ""
var current_zone_name: String = ""
var narrative_locked: bool = false
var player_party: Dictionary = {}
var world_flags: Dictionary = {}
var combat_context: Dictionary = {}

func reset_for_new_game() -> void:
	current_chapter = "ch1"
	player_party = {
		"vivis": {"hp": 100, "max_hp": 100, "attack_ids": ["rafaga_punos_tiernos"]},
		"wiky_wikerman": {"hp": 100, "max_hp": 100, "attack_ids": ["aranazo_ponzonoso"]}
	}
	world_flags = {"intro_capture_seen": false, "giovanny_rescued": false}
	combat_context = {}
	narrative_locked = false
	load_chapter_data("res://data/chapters/ch1.json")
	save_dirty.emit()

func enter_combat(payload: Dictionary) -> void:
	combat_context = payload.duplicate(true)

func exit_combat() -> void:
	combat_context.clear()

func mark_flag(flag_name: String, value: Variant = true) -> void:
	world_flags[flag_name] = value
	save_dirty.emit()

func set_chapter(chapter_id: String) -> void:
	if current_chapter == chapter_id:
		return
	current_chapter = chapter_id
	chapter_changed.emit(chapter_id)
	save_dirty.emit()

func load_chapter_data(path: String) -> void:
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		push_warning("No se pudo abrir capítulo: %s" % path)
		return
	var parsed: Variant = JSON.parse_string(file.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		push_warning("Capítulo inválido: %s" % path)
		return
	var chapter_data := parsed as Dictionary
	set_chapter(chapter_data.get("id", "ch1"))
	current_chapter_name = chapter_data.get("name", current_chapter.to_upper())
	chapter_context = chapter_data.get("context", "")
	set_objective(chapter_data.get("main_objective", ""))

func set_objective(objective_text: String) -> void:
	current_objective = objective_text
	objective_changed.emit(objective_text)

func set_zone(zone_name: String) -> void:
	current_zone_name = zone_name
	zone_changed.emit(zone_name)

func set_narrative_locked(value: bool) -> void:
	narrative_locked = value
