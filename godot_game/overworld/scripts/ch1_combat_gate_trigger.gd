extends Area2D
class_name Ch1CombatGateTrigger

@export var prerequisite_flag: String = ""
@export var completion_flag: String = "ch1_guardian_defeated"
@export var locked_dialogue_id: String = ""
@export var prep_dialogue_id: String = ""
@export var enemy_id: String = "pelusa_corrupta"
@export var return_scene: String = "res://overworld/scenes/map_pradera_bigotes.tscn"
@export var objective_after: String = ""

var _pending_combat: bool = false

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	EventBus.dialogue_finished.connect(_on_dialogue_finished)

func _exit_tree() -> void:
	if EventBus.dialogue_finished.is_connected(_on_dialogue_finished):
		EventBus.dialogue_finished.disconnect(_on_dialogue_finished)

func _on_body_entered(body: Node) -> void:
	if not (body is PlayerController):
		return
	if GameState.world_flags.get(completion_flag, false):
		return
	if not prerequisite_flag.is_empty() and not GameState.world_flags.get(prerequisite_flag, false):
		if not locked_dialogue_id.is_empty():
			EventBus.request_dialogue(locked_dialogue_id)
		return
	_pending_combat = true
	if not prep_dialogue_id.is_empty():
		EventBus.request_dialogue(prep_dialogue_id)
	else:
		_start_combat()

func _on_dialogue_finished(dialogue_id: String) -> void:
	if _pending_combat and dialogue_id == prep_dialogue_id:
		_start_combat()

func _start_combat() -> void:
	_pending_combat = false
	GameState.mark_flag("ch1_combat_started", true)
	if not objective_after.is_empty():
		GameState.set_objective(objective_after)
	EventBus.request_combat(enemy_id, {
		"source": "ch1_guardian_gate",
		"chapter_id": "ch1",
		"return_scene": return_scene
	})
