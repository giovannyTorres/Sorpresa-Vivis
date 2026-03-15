extends Node
class_name TurnManager

signal turn_started(actor_id: String)
signal turn_resolved(actor_id: String)
signal combat_finished(victory: bool)

var _turn_order: Array[String] = []
var _turn_index: int = 0
var _combat_active: bool = false

func start_combat(turn_order: Array[String]) -> void:
	_turn_order = turn_order.duplicate()
	_turn_index = 0
	_combat_active = _turn_order.size() > 0
	if _combat_active:
		turn_started.emit(_turn_order[_turn_index])

func resolve_current_turn(action_result: Dictionary) -> void:
	if not _combat_active:
		return
	var actor_id := _turn_order[_turn_index]
	turn_resolved.emit(actor_id)
	if action_result.get("combat_finished", false):
		_combat_active = false
		combat_finished.emit(action_result.get("victory", false))
		return
	_turn_index = (_turn_index + 1) % _turn_order.size()
	turn_started.emit(_turn_order[_turn_index])
