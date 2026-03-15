extends Area2D
class_name MapTransition

@export_file("*.tscn") var target_scene: String
@export var spawn_marker: String = ""

func _on_body_entered(_body: Node2D) -> void:
	if target_scene.is_empty():
		return
	EventBus.request_map_transition(target_scene, spawn_marker)
