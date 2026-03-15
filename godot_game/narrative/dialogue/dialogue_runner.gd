extends Node
class_name DialogueRunner

signal dialogue_started(dialogue_id: String)
signal dialogue_line_changed(speaker: String, text: String)
signal dialogue_finished(dialogue_id: String)

var _dialogue_data: Dictionary = {}
var _current_dialogue_id: String = ""
var _line_index: int = -1

func load_dialogue_file(path: String) -> void:
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		push_error("No se pudo abrir diálogo: %s" % path)
		return
	var parsed: Variant = JSON.parse_string(file.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		push_error("JSON de diálogo inválido: %s" % path)
		return
	_dialogue_data = parsed

func start_dialogue(dialogue_id: String) -> void:
	if not _dialogue_data.has(dialogue_id):
		push_warning("Diálogo no encontrado: %s" % dialogue_id)
		return
	_current_dialogue_id = dialogue_id
	_line_index = -1
	dialogue_started.emit(dialogue_id)
	next_line()

func next_line() -> void:
	if _current_dialogue_id.is_empty():
		return
	var lines: Array = _dialogue_data[_current_dialogue_id].get("lines", [])
	_line_index += 1
	if _line_index >= lines.size():
		dialogue_finished.emit(_current_dialogue_id)
		_current_dialogue_id = ""
		return
	var line := lines[_line_index] as Dictionary
	dialogue_line_changed.emit(line.get("speaker", ""), line.get("text", ""))
