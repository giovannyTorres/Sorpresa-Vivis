extends Node2D
class_name CompanionFollower

@export var target_path: NodePath
@export var follow_distance: float = 40.0
@export var smoothing: float = 6.0

@onready var _target := get_node_or_null(target_path) as Node2D

func _process(delta: float) -> void:
	if _target == null:
		return
	var to_target := _target.global_position - global_position
	if to_target.length() > follow_distance:
		global_position = global_position.lerp(_target.global_position - to_target.normalized() * follow_distance, clamp(smoothing * delta, 0.0, 1.0))
