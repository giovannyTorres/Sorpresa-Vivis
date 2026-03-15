extends Area2D
class_name Interactable

@export var interaction_id: String = ""
@export var interaction_type: String = "dialogue"
@export var payload: Dictionary = {}

func interact() -> void:
	match interaction_type:
		"dialogue":
			EventBus.request_dialogue(interaction_id)
		"combat":
			EventBus.request_combat(interaction_id, payload)
		"cutscene":
			EventBus.request_cutscene(interaction_id)
		_:
			push_warning("Tipo de interacción no soportado: %s" % interaction_type)
