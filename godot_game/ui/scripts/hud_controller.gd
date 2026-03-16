extends CanvasLayer
class_name HudController

@onready var chapter_label: Label = $PanelContainer/VBoxContainer/ChapterLabel
@onready var objective_label: Label = $PanelContainer/VBoxContainer/ObjectiveLabel
@onready var zone_label: Label = $PanelContainer/VBoxContainer/ZoneLabel
@onready var context_label: Label = $PanelContainer/VBoxContainer/ContextLabel
@onready var banner_panel: PanelContainer = $AnnouncementPanel
@onready var banner_title: Label = $AnnouncementPanel/VBoxContainer/Title
@onready var banner_subtitle: Label = $AnnouncementPanel/VBoxContainer/Subtitle
@onready var banner_timer: Timer = $AnnouncementTimer

var _ready_for_feedback: bool = false
var _last_chapter_seen: String = ""
var _last_objective_seen: String = ""

func _ready() -> void:
	GameState.chapter_changed.connect(_on_chapter_changed)
	GameState.objective_changed.connect(_on_objective_changed)
	GameState.zone_changed.connect(_on_zone_changed)
	GameState.banner_requested.connect(_on_banner_requested)
	banner_panel.visible = false
	_on_chapter_changed(GameState.current_chapter)
	_on_objective_changed(GameState.current_objective)
	_on_zone_changed(GameState.current_zone_name)
	_last_chapter_seen = GameState.current_chapter
	_last_objective_seen = GameState.current_objective
	_ready_for_feedback = true

func _on_chapter_changed(chapter_id: String) -> void:
	var chapter_name := GameState.current_chapter_name
	if chapter_name.is_empty():
		chapter_name = chapter_id.to_upper()
	chapter_label.text = "%s · %s" % [chapter_id.to_upper(), chapter_name]
	context_label.text = GameState.chapter_context
	if _ready_for_feedback and chapter_id != _last_chapter_seen:
		_show_banner("Capitulo %s" % chapter_id.to_upper(), chapter_name)
	_last_chapter_seen = chapter_id

func _on_objective_changed(objective_text: String) -> void:
	objective_label.text = "Objetivo: %s" % objective_text
	if _ready_for_feedback and not objective_text.is_empty() and objective_text != _last_objective_seen:
		_show_banner("Nuevo objetivo", objective_text)
	_last_objective_seen = objective_text

func _on_zone_changed(zone_name: String) -> void:
	zone_label.text = "Zona: %s" % zone_name

func _on_banner_requested(title: String, subtitle: String, _tone: String) -> void:
	_show_banner(title, subtitle)

func _show_banner(title: String, subtitle: String) -> void:
	if title.is_empty() and subtitle.is_empty():
		return
	banner_title.text = title
	banner_subtitle.text = subtitle
	banner_panel.visible = true
	banner_timer.start()

func _on_announcement_timer_timeout() -> void:
	banner_panel.visible = false
