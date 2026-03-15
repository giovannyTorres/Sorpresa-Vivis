extends Node
class_name GameBootstrap

const DEFAULT_OVERWORLD_SCENE := "res://overworld/scenes/map_pradera_bigotes.tscn"
const DEFAULT_COMBAT_SCENE := "res://combat/scenes/combat_scene.tscn"

@onready var root_layer: Node = $"../Root"

func _ready() -> void:
	# Arranque mínimo estable: se abre directamente el overworld.
	# Cuando la intro narrativa tenga mayor implementación se puede rutear antes de esto.
	GameState.reset_for_new_game()
	SceneRouter.configure_root(root_layer)
	_bind_global_events()
	SceneRouter.go_to_overworld(DEFAULT_OVERWORLD_SCENE)

func _bind_global_events() -> void:
	if not EventBus.combat_requested.is_connected(_on_combat_requested):
		EventBus.combat_requested.connect(_on_combat_requested)
	if not EventBus.map_transition_requested.is_connected(_on_map_transition_requested):
		EventBus.map_transition_requested.connect(_on_map_transition_requested)

func _on_combat_requested(enemy_id: String, context: Dictionary) -> void:
	var payload := context.duplicate(true)
	payload["enemy_id"] = enemy_id
	SceneRouter.go_to_combat(DEFAULT_COMBAT_SCENE, payload)

func _on_map_transition_requested(target_scene: String, _spawn_marker: String) -> void:
	if target_scene.is_empty():
		return
	SceneRouter.go_to_overworld(target_scene)
