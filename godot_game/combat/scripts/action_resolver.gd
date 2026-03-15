extends Node
class_name ActionResolver

func resolve_attack(attacker: Dictionary, defender: Dictionary, attack_data: Dictionary) -> Dictionary:
	var min_dmg: int = int(attack_data.get("damage_min", 1))
	var max_dmg: int = int(attack_data.get("damage_max", min_dmg))
	var damage := randi_range(min_dmg, max_dmg)
	defender["hp"] = max(0, int(defender.get("hp", 0)) - damage)
	return {
		"damage": damage,
		"attacker_id": attacker.get("id", ""),
		"defender_id": defender.get("id", ""),
		"defender_hp": defender["hp"]
	}
