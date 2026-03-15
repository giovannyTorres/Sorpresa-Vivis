extends Node

signal scene_changed(scene_path: String)

var _root: Node
var _active_scene: Node

func configure_root(root: Node) -> void:
	_root = root

func go_to_overworld(scene_path: String) -> void:
	_change_scene(scene_path)

func go_to_combat(scene_path: String, payload: Dictionary = {}) -> void:
	GameState.enter_combat(payload)
	_change_scene(scene_path)

func return_to_overworld(scene_path: String) -> void:
	GameState.exit_combat()
	_change_scene(scene_path)

func _change_scene(scene_path: String) -> void:
	if _root == null:
		push_error("SceneRouter sin root configurado")
		return
	if _active_scene != null:
		_active_scene.queue_free()
	var packed := load(scene_path) as PackedScene
	if packed == null:
		push_error("No se pudo cargar escena: %s" % scene_path)
		return
	_active_scene = packed.instantiate()
	_root.add_child(_active_scene)
	scene_changed.emit(scene_path)
