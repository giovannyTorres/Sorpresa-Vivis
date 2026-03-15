extends CharacterBody2D
class_name PlayerController

@export var move_speed: float = 130.0

func _physics_process(_delta: float) -> void:
	var input_vec := Input.get_vector("move_left", "move_right", "move_up", "move_down")
	velocity = input_vec * move_speed
	move_and_slide()

	if Input.is_action_just_pressed("action_primary"):
		EventBus.request_dialogue("intro_capture")
	if Input.is_action_just_pressed("action_companion"):
		EventBus.request_combat("maliketh", {"source": "overworld_test"})
