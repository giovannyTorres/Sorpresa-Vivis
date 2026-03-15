extends CharacterBody2D
class_name PlayerController

@export var move_speed: float = 130.0
@export var world_bounds: Rect2 = Rect2(64.0, 120.0, 1792.0, 896.0)

func _physics_process(_delta: float) -> void:
	if GameState.narrative_locked:
		velocity = Vector2.ZERO
		move_and_slide()
		return

	var input_vec := Input.get_vector("move_left", "move_right", "move_up", "move_down")
	velocity = input_vec * move_speed
	move_and_slide()

	global_position.x = clamp(global_position.x, world_bounds.position.x, world_bounds.end.x)
	global_position.y = clamp(global_position.y, world_bounds.position.y, world_bounds.end.y)
