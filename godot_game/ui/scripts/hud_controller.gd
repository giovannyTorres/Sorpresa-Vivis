extends CanvasLayer
class_name HudController

@onready var chapter_label: Label = $PanelContainer/VBoxContainer/ChapterLabel
@onready var objective_label: Label = $PanelContainer/VBoxContainer/ObjectiveLabel
@onready var zone_label: Label = $PanelContainer/VBoxContainer/ZoneLabel
@onready var context_label: Label = $PanelContainer/VBoxContainer/ContextLabel

func _ready() -> void:
	GameState.chapter_changed.connect(_on_chapter_changed)
	GameState.objective_changed.connect(_on_objective_changed)
	GameState.zone_changed.connect(_on_zone_changed)
	_on_chapter_changed(GameState.current_chapter)
	_on_objective_changed(GameState.current_objective)
	_on_zone_changed(GameState.current_zone_name)
	context_label.text = GameState.chapter_context

func _on_chapter_changed(chapter_id: String) -> void:
	var chapter_name := GameState.current_chapter_name
	if chapter_name.is_empty():
		chapter_name = chapter_id.to_upper()
	chapter_label.text = "%s · %s" % [chapter_id.to_upper(), chapter_name]

func _on_objective_changed(objective_text: String) -> void:
	objective_label.text = "Objetivo: %s" % objective_text

func _on_zone_changed(zone_name: String) -> void:
	zone_label.text = "Zona: %s" % zone_name
