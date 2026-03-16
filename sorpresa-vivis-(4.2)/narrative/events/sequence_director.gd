extends Node
class_name SequenceDirector

@export_file("*.json") var event_flow_path: String

var event_flow: Dictionary = {}

func _ready() -> void:
	if event_flow_path.is_empty():
		return
	var file := FileAccess.open(event_flow_path, FileAccess.READ)
	if file:
		event_flow = JSON.parse_string(file.get_as_text())

func play_sequence(sequence_id: String) -> void:
	if not event_flow.has(sequence_id):
		push_warning("Secuencia no encontrada: %s" % sequence_id)
		return
	for step in event_flow[sequence_id].get("steps", []):
		_match_step(step)

func _match_step(step: Dictionary) -> void:
	match step.get("type", ""):
		"dialogue": EventBus.request_dialogue(step.get("id", ""))
		"combat": EventBus.request_combat(step.get("enemy_id", ""), step)
		"flag": GameState.mark_flag(step.get("name", ""), step.get("value", true))
		_:
			push_warning("Paso de secuencia no soportado: %s" % step)
