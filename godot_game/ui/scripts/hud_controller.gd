extends CanvasLayer
class_name HudController

@onready var chapter_label: Label = $PanelContainer/VBoxContainer/ChapterLabel
@onready var objective_label: Label = $PanelContainer/VBoxContainer/ObjectiveLabel

func _ready() -> void:
	GameState.chapter_changed.connect(_on_chapter_changed)
	_on_chapter_changed(GameState.current_chapter)

func _on_chapter_changed(chapter_id: String) -> void:
	chapter_label.text = "Capítulo: %s" % chapter_id.to_upper()
	objective_label.text = "Objetivo: encontrar a Giovanny en el Mundo de los Pelos de Gato"
