extends CanvasLayer
class_name DialoguePresenter

@onready var panel: PanelContainer = $Panel
@onready var speaker_label: Label = $Panel/VBox/Speaker
@onready var text_label: RichTextLabel = $Panel/VBox/Text
@onready var hint_label: Label = $Panel/VBox/Hint

var runner: DialogueRunner
var _active: bool = false

func _ready() -> void:
	runner = DialogueRunner.new()
	add_child(runner)
	runner.load_dialogue_folder("res://data/dialogues")
	runner.dialogue_started.connect(_on_dialogue_started)
	runner.dialogue_line_changed.connect(_on_dialogue_line_changed)
	runner.dialogue_finished.connect(_on_dialogue_finished)
	EventBus.dialogue_requested.connect(_on_dialogue_requested)
	visible = false

func _unhandled_input(event: InputEvent) -> void:
	if not _active:
		return
	if event.is_action_pressed("action_primary"):
		get_viewport().set_input_as_handled()
		runner.next_line()

func _on_dialogue_requested(dialogue_id: String) -> void:
	runner.start_dialogue(dialogue_id)

func _on_dialogue_started(_dialogue_id: String) -> void:
	_active = true
	visible = true
	GameState.set_narrative_locked(true)
	hint_label.text = "SPACE para continuar"

func _on_dialogue_line_changed(speaker: String, text: String) -> void:
	speaker_label.text = speaker
	text_label.text = text

func _on_dialogue_finished(dialogue_id: String) -> void:
	_active = false
	visible = false
	GameState.set_narrative_locked(false)
	EventBus.notify_dialogue_finished(dialogue_id)
