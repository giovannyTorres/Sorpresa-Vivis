extends Area2D
class_name Ch1StoryTrigger

@export var trigger_flag: String = ""
@export var dialogue_id: String = ""
@export var objective_after: String = ""
@export var one_shot: bool = true

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _on_body_entered(body: Node) -> void:
	if not (body is PlayerController):
		return
	if not trigger_flag.is_empty() and GameState.world_flags.get(trigger_flag, false):
		return
	if not trigger_flag.is_empty():
		GameState.mark_flag(trigger_flag, true)
	if not objective_after.is_empty():
		GameState.set_objective(objective_after)
	if not dialogue_id.is_empty():
		EventBus.request_dialogue(dialogue_id)
	if one_shot:
		set_deferred("monitoring", false)
		set_deferred("monitorable", false)
