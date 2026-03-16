extends Node2D

const DEFAULT_RETURN_SCENE := "res://overworld/scenes/map_pradera_bigotes.tscn"

@onready var title_label: Label = $CanvasLayer/Title
@onready var hint_label: Label = $CanvasLayer/Hint
@onready var summary_label: Label = $CanvasLayer/Summary

var _resolved: bool = false

func _ready() -> void:
	var enemy_id := str(GameState.combat_context.get("enemy_id", "enemigo"))
	title_label.text = "Combate CH1 · Manifestación hostil: %s" % enemy_id.capitalize()
	hint_label.text = "SPACE: asestar Ráfaga de puños tiernos + Arañazo ponzoñoso"
	summary_label.text = "La pelusa corrupta bloquea el rastro de Giovanny."

func _unhandled_input(event: InputEvent) -> void:
	if _resolved:
		return
	if not event.is_action_pressed("action_primary"):
		return
	_resolved = true
	_resolve_victory()

func _resolve_victory() -> void:
	var is_ch1 := str(GameState.combat_context.get("chapter_id", "")) == "ch1"
	if is_ch1:
		GameState.mark_flag("ch1_guardian_defeated", true)
		GameState.mark_flag("ch1_progress_gate_unlocked", true)
		GameState.set_objective("Regresa al claro inicial y confirma la pista de Giovanny.")
	var return_scene := str(GameState.combat_context.get("return_scene", DEFAULT_RETURN_SCENE))
	SceneRouter.return_to_overworld(return_scene)
