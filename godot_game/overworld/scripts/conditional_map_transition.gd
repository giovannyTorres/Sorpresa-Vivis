extends Area2D
class_name ConditionalMapTransition

@export_file("*.tscn") var target_scene: String
@export var required_flag: String = ""
@export var locked_dialogue_id: String = ""

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _on_body_entered(body: Node) -> void:
	if not (body is PlayerController):
		return
	if not required_flag.is_empty() and not GameState.world_flags.get(required_flag, false):
		if not locked_dialogue_id.is_empty():
			EventBus.request_dialogue(locked_dialogue_id)
		return
	if target_scene.is_empty():
		return
	EventBus.request_map_transition(target_scene)
