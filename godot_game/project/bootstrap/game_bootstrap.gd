extends Node
class_name GameBootstrap

@onready var root_layer: Node = $"../Root"

func _ready() -> void:
	# Punto único de arranque del juego.
	GameState.reset_for_new_game()
	SceneRouter.configure_root(root_layer)
	SceneRouter.go_to_overworld("res://overworld/scenes/map_pradera_bigotes.tscn")
