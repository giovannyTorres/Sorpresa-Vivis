extends Node2D

@onready var welcome_label: Label = $HUD/WelcomeTone

func _ready() -> void:
	GameState.set_zone("Pradera de Bigotes")
	GameState.set_objective("Explora la Pradera de Bigotes y detecta la presencia de Maliketh.")
	welcome_label.text = "Pradera de Bigotes · Bienvenida mullida, rareza en el aire"
	EventBus.dialogue_finished.connect(_on_dialogue_finished)
	if not GameState.world_flags.get("ch1_overworld_intro_seen", false):
		GameState.mark_flag("ch1_overworld_intro_seen", true)
		EventBus.request_dialogue("ch1_overworld_intro")

func _exit_tree() -> void:
	if EventBus.dialogue_finished.is_connected(_on_dialogue_finished):
		EventBus.dialogue_finished.disconnect(_on_dialogue_finished)

func _on_dialogue_finished(dialogue_id: String) -> void:
	if dialogue_id == "ch1_overworld_intro":
		GameState.set_objective("Sigue la pelusa oscura cerca del altar y busca pistas de Giovanny.")
