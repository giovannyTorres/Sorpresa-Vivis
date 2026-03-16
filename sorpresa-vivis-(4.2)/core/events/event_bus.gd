extends Node
class_name EventBus

signal combat_requested(enemy_id: String, context: Dictionary)
signal dialogue_requested(dialogue_id: String)
signal map_transition_requested(target_scene: String, spawn_marker: String)
signal cutscene_requested(cutscene_id: String)

func request_combat(enemy_id: String, context: Dictionary = {}) -> void:
	combat_requested.emit(enemy_id, context)

func request_dialogue(dialogue_id: String) -> void:
	dialogue_requested.emit(dialogue_id)

func request_map_transition(target_scene: String, spawn_marker: String = "") -> void:
	map_transition_requested.emit(target_scene, spawn_marker)

func request_cutscene(cutscene_id: String) -> void:
	cutscene_requested.emit(cutscene_id)
