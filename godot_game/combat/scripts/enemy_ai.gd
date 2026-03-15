extends Node
class_name EnemyAI

func choose_action(enemy: Dictionary, party_state: Dictionary) -> Dictionary:
	var attack_id := "basic_attack"
	if enemy.has("attack_ids") and enemy["attack_ids"].size() > 0:
		attack_id = str(enemy["attack_ids"][0])
	var target_id := _pick_lowest_hp(party_state)
	return {"action": "attack", "attack_id": attack_id, "target_id": target_id}

func _pick_lowest_hp(party_state: Dictionary) -> String:
	var selected := ""
	var min_hp := 999999
	for actor_id in party_state.keys():
		var hp := int(party_state[actor_id].get("hp", 9999))
		if hp < min_hp:
			min_hp = hp
			selected = str(actor_id)
	return selected
