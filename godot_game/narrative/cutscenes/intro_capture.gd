extends Node2D

const CH1_OVERWORLD_SCENE := "res://overworld/scenes/map_pradera_bigotes.tscn"

@onready var title_label: Label = $CanvasLayer/Title
@onready var subtitle_label: Label = $CanvasLayer/Subtitle

func _ready() -> void:
	GameState.load_chapter_data("res://data/chapters/ch1.json")
	title_label.text = "CH1 · %s" % GameState.current_chapter_name
	subtitle_label.text = GameState.chapter_context
	EventBus.dialogue_finished.connect(_on_dialogue_finished)
	EventBus.request_dialogue("ch1_intro")

func _exit_tree() -> void:
	if EventBus.dialogue_finished.is_connected(_on_dialogue_finished):
		EventBus.dialogue_finished.disconnect(_on_dialogue_finished)

func _on_dialogue_finished(dialogue_id: String) -> void:
	if dialogue_id != "ch1_intro":
		return
	GameState.mark_flag("intro_capture_seen", true)
	EventBus.request_map_transition(CH1_OVERWORLD_SCENE)
