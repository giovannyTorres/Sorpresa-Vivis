extends Area2D

@export var dialogue_id: String = "ch1_maliketh_echo"
@export var trigger_flag: String = "ch1_maliketh_echo_seen"

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _on_body_entered(body: Node) -> void:
	if not (body is PlayerController):
		return
	if GameState.world_flags.get(trigger_flag, false):
		return
	GameState.mark_flag(trigger_flag, true)
	EventBus.request_dialogue(dialogue_id)
