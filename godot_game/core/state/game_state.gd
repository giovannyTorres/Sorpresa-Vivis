extends Node
class_name GameState

signal save_dirty()
signal chapter_changed(chapter_id: String)

var current_chapter: String = "ch1"
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
