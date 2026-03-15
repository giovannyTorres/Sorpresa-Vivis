extends Area2D
class_name CombatTestTrigger

@export var enemy_id: String = "maliketh"
@export var encounter_context: Dictionary = {"source": "overworld_combat_zone"}

@onready var prompt_label: Label = $PromptLabel
var _player_inside: bool = false

func _ready() -> void:
	body_entered.connect(_on_body_entered)
	body_exited.connect(_on_body_exited)
	prompt_label.visible = false

func _process(_delta: float) -> void:
	if _player_inside and not GameState.narrative_locked and Input.is_action_just_pressed("action_companion"):
		EventBus.request_combat(enemy_id, encounter_context)

func _on_body_entered(body: Node) -> void:
	if body is PlayerController:
		_player_inside = true
		prompt_label.visible = true

func _on_body_exited(body: Node) -> void:
	if body is PlayerController:
		_player_inside = false
		prompt_label.visible = false
