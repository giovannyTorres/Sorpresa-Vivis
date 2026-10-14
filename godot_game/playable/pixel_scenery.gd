class_name PixelScenery
extends RefCounted
## Escenarios de pixel art: todos los detalles respetan el área del juego y sus colisiones.

const FIELD := Rect2(42, 126, 876, 352)
const DRAW_BOUNDS := Rect2(0, 104, 960, 400)
const GATE := Rect2(854, 260, 64, 104)
const INK := Color("0d141e")

const PINE := [
	"            oo            ",
	"           oggo           ",
	"          ogglgo          ",
	"          oggggo          ",
	"         oglgsggo         ",
	"        ogggggssgo        ",
	"       ogglgggsssgo       ",
	"      ogggggsgsssdgo      ",
	"     ogglgggssssdddgo     ",
	"    ooggsgssssssdddoo     ",
	"      oggglgssssdgo       ",
	"     oggggggssssddgo      ",
	"    ogglggsssgssdddgo     ",
	"   oggggggssssssddddgo    ",
	"  ooggsgsssssssssdddooo   ",
	"    oggglgsssgssssddgo    ",
	"   ogggggggssssssdddgo    ",
	"  ogglggssssgsssdddddgo   ",
	" oggggggssssssssddddddgo  ",
	"ooggsgsssgssssddddddddooo ",
	"  oooosssssdddddddoooo    ",
	"      oooooooooooo        ",
	"          otwto          ",
	"          otwto          ",
	"         ottwtto         ",
	"        ottttttto        ",
]

const OAK := [
	"        oooooooo         ",
	"     oooggggsssooo       ",
	"   oogglggggssssssoo     ",
	"  ogggggglggggsssssso    ",
	" ogglgggggggssssddssso   ",
	"oggggglgggsssgssssddso   ",
	"ogglgggggggssssssssddso  ",
	"ogggsgggssssssssddddddo  ",
	" ogggggsssgsssssdddddso  ",
	" oggssgssssssssdddddddo  ",
	"  ogggssssssssdddddddo   ",
	"   oossssdddddddddoo     ",
	"     ooooottwtoooo       ",
	"        ottwtto          ",
	"         otwto           ",
	"         otwto           ",
	"        ottwtto          ",
	"      ootttwtttoo        ",
]

static func draw_world(canvas: Node2D, stage: int, time: float, obstacles: Array[Rect2], gate_open: bool) -> void:
	var colors := _palette(stage)
	_r(canvas, DRAW_BOUNDS, colors["back"])
	_r(canvas, FIELD.grow(4), INK)
	_r(canvas, FIELD, colors["ground"])
	if stage == 2:
		_castle_floor(canvas, colors)
	else:
		_grass_floor(canvas, colors, stage, obstacles)
	_path(canvas, colors, stage)
	if stage == 0:
		_meadow(canvas, colors, time)
	elif stage == 1:
		_twilight(canvas, colors, time)
	else:
		_lunar_tower(canvas, colors, time)
	for i in range(obstacles.size()):
		if stage == 0:
			_pond(canvas, obstacles[i], colors, time, i)
		else:
			_ruin(canvas, obstacles[i], colors, i)
	_frame(canvas, colors, stage)
	if stage < 2:
		_gate(canvas, colors, gate_open, time)
	_sparks(canvas, colors, stage, time)

static func _palette(stage: int) -> Dictionary:
	if stage == 0:
		return {"back": Color("101c22"), "ground": Color("243f36"), "grass": Color("365747"),
			"dark": Color("1b302e"), "light": Color("55764b"), "path": Color("695b46"),
			"path_dark": Color("4b463b"), "path_light": Color("8b7856"), "accent": Color("93ba71"),
			"leaf": Color("447654"), "leaf_light": Color("71965e"), "leaf_shadow": Color("2c5544"),
			"leaf_dark": Color("1c3c35"), "wood": Color("654939"), "wood_light": Color("a0744c"),
			"flower": Color("e98cb5"), "glow": Color("dfdf96")}
	if stage == 1:
		return {"back": Color("141c2a"), "ground": Color("293b43"), "grass": Color("3b5153"),
			"dark": Color("202c37"), "light": Color("557467"), "path": Color("565360"),
			"path_dark": Color("393c48"), "path_light": Color("76717a"), "accent": Color("88aeb7"),
			"leaf": Color("36564f"), "leaf_light": Color("577767"), "leaf_shadow": Color("263f43"),
			"leaf_dark": Color("1a2d37"), "wood": Color("51474a"), "wood_light": Color("80726a"),
			"flower": Color("b69bce"), "glow": Color("85d4d9")}
	return {"back": Color("141321"), "ground": Color("302e43"), "grass": Color("3a374e"),
		"dark": Color("242333"), "light": Color("565068"), "path": Color("494256"),
		"path_dark": Color("302a40"), "path_light": Color("74617e"), "accent": Color("ab81d4"),
		"leaf": Color("44334e"), "leaf_light": Color("755474"), "leaf_shadow": Color("352a40"),
		"leaf_dark": Color("251e31"), "wood": Color("504053"), "wood_light": Color("887386"),
		"flower": Color("c88cd6"), "glow": Color("c9a4fa")}

static func _r(canvas: Node2D, rect: Rect2, color: Color) -> void:
	var clipped := rect.intersection(DRAW_BOUNDS)
	if clipped.has_area():
		canvas.draw_rect(clipped, color)

static func _hash(x: int, y: int, seed: int = 0) -> int:
	return (x * 73856093 ^ y * 19349663 ^ seed * 83492791) & 0x7fffffff

static func _grass_floor(canvas: Node2D, colors: Dictionary, stage: int, obstacles: Array[Rect2]) -> void:
	for y in range(132, 474, 12):
		for x in range(48, 916, 12):
			var n := _hash(x, y, stage + 2)
			var point := Vector2(x + n % 7, y + (n / 7) % 6)
			var color: Color = colors["grass"] if n % 5 < 3 else colors["dark"]
			if n % 7 < 5:
				_r(canvas, Rect2(point, Vector2(4 + n % 5, 2)), color)
			if n % 11 < 3:
				_r(canvas, Rect2(point + Vector2(3, -3), Vector2(2, 5)), color)
				_r(canvas, Rect2(point + Vector2(-1, -1), Vector2(2, 3)), color)
	for i in range(90):
		var x := 54 + _hash(i, 8, stage) % 854
		var y := 146 + _hash(i, 19, stage) % 317
		if y > 276 and y < 352:
			continue
		var point := Vector2(x, y)
		var occupied := false
		for obstacle in obstacles:
			if obstacle.grow(14).has_point(point):
				occupied = true
		if occupied or GATE.grow(8).has_point(point):
			continue
		if i % 5 == 0:
			_stone(canvas, point, colors, 1 + i % 2)
		elif i % 3 == 0:
			_flower(canvas, point, colors["flower"] if i % 2 == 0 else colors["glow"], colors)
		else:
			_tuft(canvas, point, colors, i % 3)

static func _path(canvas: Node2D, colors: Dictionary, stage: int) -> void:
	_r(canvas, Rect2(44, 282, 868, 64), colors["dark"])
	_r(canvas, Rect2(44, 286, 868, 56), colors["path_dark"])
	_r(canvas, Rect2(44, 289, 868, 50), colors["path"])
	for x in range(44, 912, 10):
		var n := _hash(x, stage, 19)
		_r(canvas, Rect2(x, 283 + n % 4, 6, 3), colors["grass"])
		_r(canvas, Rect2(x, 340 - n % 3, 5, 4), colors["grass"])
		if n % 3 == 0:
			_r(canvas, Rect2(x + 3, 290 + n % 44, 3 + n % 3, 2), colors["path_light"])
	for y in range(294, 336, 12):
		for x in range(52, 904, 24):
			var n := _hash(x, y, stage)
			_r(canvas, Rect2(x + n % 8, y + n % 5, 5, 2), colors["path_dark"])
	if stage == 2:
		_r(canvas, Rect2(74, 298, 779, 31), Color("49354e"))
		_r(canvas, Rect2(74, 299, 779, 2), Color("785971"))
		_r(canvas, Rect2(74, 326, 779, 2), Color("785971"))
		for x in range(84, 852, 24):
			_r(canvas, Rect2(x, 304, 3, 3), Color("af8194"))
			_r(canvas, Rect2(x + 3, 307, 3, 3), Color("81607b"))
			_r(canvas, Rect2(x, 317, 3, 3), Color("af8194"))

static func _meadow(canvas: Node2D, colors: Dictionary, time: float) -> void:
	# Agua poco profunda; el puente forma parte del sendero transitable.
	for y in range(128, 478, 6):
		var shift := (_hash(2, y, 1) % 3 - 1) * 2
		_r(canvas, Rect2(410 + shift, y, 22, 6), Color("193b48"))
		_r(canvas, Rect2(414 + shift, y, 14, 6), Color("285969"))
		if y % 18 == 2:
			_r(canvas, Rect2(415 + shift, y, 7, 2), Color("45818a"))
	_r(canvas, Rect2(397, 287, 49, 55), INK)
	for y in range(288, 340, 8):
		_r(canvas, Rect2(399, y, 46, 7), Color("7c5d43"))
		_r(canvas, Rect2(401, y, 42, 2), Color("b38e62"))
		_r(canvas, Rect2(407 + y % 3 * 6, y + 4, 21, 1), Color("594738"))
	for x in [398, 442]:
		_r(canvas, Rect2(x, 278, 4, 70), Color("392c29"))
		_r(canvas, Rect2(x + 1, 278, 2, 70), Color("be9564"))
		for y in [279, 335]:
			_r(canvas, Rect2(x - 2, y, 8, 11), Color("594134"))
			_r(canvas, Rect2(x - 1, y, 6, 2), Color("d0ae75"))
	for i in range(10):
		var oak := i % 3 == 0
		_tree(canvas, Vector2(12 + i * 102, 158 + i % 2 * 3), colors, i, 3 if oak else 2, oak)
		_tree(canvas, Vector2(6 + i * 105, 531 + i % 2 * 4), colors, i + 15, 3, i % 2 == 0)
		if not oak:
			_tuft(canvas, Vector2(47 + i * 102, 137), colors, i % 3)
	for point in [Vector2(165, 184), Vector2(608, 205), Vector2(236, 444), Vector2(820, 435)]:
		_mushrooms(canvas, point, colors)
	for i in range(11):
		var x := 91 + i * 77
		_tuft(canvas, Vector2(x, 469 - i % 3 * 4), colors, i % 3)
		_flower(canvas, Vector2(x + 18, 457 - i % 4 * 3), colors["flower"], colors)
	# Reflejos del arroyo, cuantizados para conservar el aspecto pixelado.
	for i in range(10):
		var y := 141 + i * 33
		if y < 278 or y > 350:
			var offset := int(time * 3 + i) % 3 * 2
			_r(canvas, Rect2(415 + offset, y, 8, 2), Color("5596a0"))

static func _twilight(canvas: Node2D, colors: Dictionary, time: float) -> void:
	for i in range(10):
		_tree(canvas, Vector2(13 + i * 102, 157 + i % 3 * 2), colors, i, 2, false)
		_tree(canvas, Vector2(21 + i * 100, 533), colors, i + 24, 3, false)
	for x in [82, 163, 643, 819]:
		_r(canvas, Rect2(x, 129, 47, 5), Color("46505a"))
		for i in range(4):
			_r(canvas, Rect2(x + i * 12, 135, 10, 8 + i % 2 * 5), Color("353d49"))
			_r(canvas, Rect2(x + i * 12, 135, 10, 2), Color("667178"))
	for point in [Vector2(185, 211), Vector2(678, 185), Vector2(731, 454)]:
		_stone(canvas, point, colors, 3)
		_mushrooms(canvas, point + Vector2(14, 2), colors)
	for i in range(4):
		var y := 186 + i * 78
		var drift := int(time * 4 + i * 17) % 40
		var x := 92 + i * 163 + drift
		var mist := Color(0.64, 0.73, 0.76, 0.055)
		_r(canvas, Rect2(x + 12, y, 110, 3), mist)
		_r(canvas, Rect2(x, y + 3, 170, 5), mist)
		_r(canvas, Rect2(x + 45, y + 8, 145, 3), mist)
		_r(canvas, Rect2(x + 88, y + 11, 79, 2), mist)

static func _castle_floor(canvas: Node2D, colors: Dictionary) -> void:
	for y in range(126, 478, 22):
		for x in range(42, 918, 34):
			var shift := 17 if (y - 126) / 22 % 2 == 0 else 0
			var tile := Rect2(x + shift, y, 33, 21).intersection(FIELD)
			var n := _hash(x, y, 7)
			_r(canvas, tile, colors["grass"] if n % 6 == 0 else colors["ground"])
			_r(canvas, Rect2(tile.position, Vector2(tile.size.x, 1)), colors["dark"])
			_r(canvas, Rect2(tile.position, Vector2(1, tile.size.y)), colors["dark"])
			if n % 9 < 2:
				_r(canvas, Rect2(tile.position + Vector2(5, 5), Vector2(7, 1)), colors["light"].darkened(0.22))
			if n % 17 == 0:
				_r(canvas, Rect2(tile.position + Vector2(12, 9), Vector2(1, 5)), colors["dark"])
				_r(canvas, Rect2(tile.position + Vector2(9, 14), Vector2(4, 1)), colors["dark"])
	# Un sello lunar incrustado en el suelo, sin añadir obstáculos.
	var purple := Color("4b3b60")
	for rect in [Rect2(648, 215, 85, 3), Rect2(636, 218, 109, 3), Rect2(626, 221, 129, 3),
		Rect2(617, 227, 3, 157), Rect2(762, 227, 3, 157), Rect2(626, 390, 129, 3),
		Rect2(636, 393, 109, 3), Rect2(648, 396, 85, 3)]:
		_r(canvas, rect, purple)
	for i in range(11):
		_r(canvas, Rect2(627 + i * 12, 223 + i % 2 * 3, 3, 3), Color("655076"))
		_r(canvas, Rect2(627 + i * 12, 383 - i % 2 * 3, 3, 3), Color("655076"))
	_pixel_moon(canvas, Vector2(659, 232), 6, Color("554464"), colors["ground"])
	for point in [Vector2(231, 204), Vector2(402, 402), Vector2(574, 186), Vector2(796, 421)]:
		_r(canvas, Rect2(point + Vector2(3, 0), Vector2(3, 3)), Color("76607e"))
		_r(canvas, Rect2(point + Vector2(0, 3), Vector2(9, 3)), Color("65516d"))
		_r(canvas, Rect2(point + Vector2(3, 6), Vector2(3, 3)), Color("76607e"))

static func _lunar_tower(canvas: Node2D, colors: Dictionary, time: float) -> void:
	# Laterales tallados y muros superiores, fuera del suelo útil.
	for y in range(104, 504, 18):
		for x in [0, 18, 920, 939]:
			_r(canvas, Rect2(x, y, 17, 17), Color("302a41"))
			_r(canvas, Rect2(x, y, 17, 2), Color("52435e"))
			_r(canvas, Rect2(x + 2, y + 3, 2, 8), Color("3d344d"))
	for x in range(42, 918, 32):
		_r(canvas, Rect2(x, 105, 31, 20), Color("352b44"))
		_r(canvas, Rect2(x + 2, 108, 27, 2), Color("615071"))
	for x in [94, 282, 472, 660, 848]:
		_window(canvas, Vector2(x, 107), colors)
	for y in [170, 374]:
		_column(canvas, Vector2(8, y), colors)
		_column(canvas, Vector2(920, y), colors)
	for point in [Vector2(57, 204), Vector2(895, 204), Vector2(57, 408), Vector2(895, 408)]:
		_torch(canvas, point, time, colors)
	# Crescent in the lunar window and pale reflections between the flagstones.
	_pixel_moon(canvas, Vector2(822, 111), 2, colors["glow"], Color("171e32"))
	for i in range(5):
		_r(canvas, Rect2(775 + i * 5, 138 + i * 11, 58 + i * 9, 2), Color(0.64, 0.53, 0.82, 0.09))
	for point in [Vector2(173, 162), Vector2(396, 450), Vector2(843, 430)]:
		_r(canvas, Rect2(point, Vector2(2, 9)), Color("4e4160"))
		_r(canvas, Rect2(point + Vector2(-3, 5), Vector2(8, 2)), Color("4e4160"))

static func _pattern(canvas: Node2D, rows: Array, point: Vector2, unit: int, palette: Dictionary) -> void:
	for y in range(rows.size()):
		var row: String = rows[y]
		var x := 0
		while x < row.length():
			var symbol := row.substr(x, 1)
			var count := 1
			while x + count < row.length() and row.substr(x + count, 1) == symbol:
				count += 1
			if palette.has(symbol):
				_r(canvas, Rect2(point + Vector2(x * unit, y * unit), Vector2(count * unit, unit)), palette[symbol])
			x += count

static func _tree(canvas: Node2D, feet: Vector2, colors: Dictionary, seed: int, unit: int, oak: bool) -> void:
	var rows: Array = OAK if oak else PINE
	var width := 25 * unit
	var height := rows.size() * unit
	_r(canvas, Rect2(feet - Vector2(width * 0.38, 3), Vector2(width * 0.76, 7)), colors["dark"])
	var palette := {"o": INK, "g": colors["leaf"], "l": colors["leaf_light"],
		"s": colors["leaf_shadow"], "d": colors["leaf_dark"], "t": colors["wood"], "w": colors["wood_light"]}
	_pattern(canvas, rows, (feet - Vector2(width / 2, height)).floor(), unit, palette)
	if seed % 4 == 0:
		_r(canvas, Rect2(feet - Vector2(9, 7), Vector2(4, 4)), Color("627247"))

static func _tuft(canvas: Node2D, point: Vector2, colors: Dictionary, variation: int) -> void:
	_r(canvas, Rect2(point + Vector2(-3, 1), Vector2(12, 2)), colors["dark"])
	_r(canvas, Rect2(point + Vector2(0, -3), Vector2(2, 6)), colors["grass"])
	_r(canvas, Rect2(point + Vector2(3, -6 - variation), Vector2(2, 9 + variation)), colors["light"])
	_r(canvas, Rect2(point + Vector2(6, -2), Vector2(2, 5)), colors["grass"])
	_r(canvas, Rect2(point + Vector2(1, -5), Vector2(2, 2)), colors["light"])

static func _flower(canvas: Node2D, point: Vector2, flower: Color, colors: Dictionary) -> void:
	_r(canvas, Rect2(point + Vector2(1, -2), Vector2(2, 7)), colors["leaf_shadow"])
	_r(canvas, Rect2(point + Vector2(3, 1), Vector2(3, 2)), colors["light"])
	_r(canvas, Rect2(point + Vector2(-1, -5), Vector2(6, 2)), flower.darkened(0.22))
	_r(canvas, Rect2(point + Vector2(0, -7), Vector2(4, 6)), flower)
	_r(canvas, Rect2(point + Vector2(-2, -5), Vector2(8, 2)), flower)
	_r(canvas, Rect2(point + Vector2(1, -5), Vector2(2, 2)), Color("f2d5a0"))

static func _stone(canvas: Node2D, point: Vector2, colors: Dictionary, unit: int) -> void:
	var rows := ["  oooo  ", " oaaaao ", "oaabbcco", "obbbccco", " occccco", "  ooooo "]
	_pattern(canvas, rows, point - Vector2(4 * unit, 5 * unit), unit,
		{"o": colors["dark"], "a": colors["light"], "b": colors["grass"], "c": colors["path_dark"]})

static func _mushrooms(canvas: Node2D, point: Vector2, colors: Dictionary) -> void:
	for i in range(2):
		var p := point + Vector2(i * 9, i * 3)
		_r(canvas, Rect2(p + Vector2(2, -2), Vector2(3, 6)), Color("b6ac8c"))
		_r(canvas, Rect2(p + Vector2(-1, -5), Vector2(9, 4)), colors["flower"].darkened(0.34))
		_r(canvas, Rect2(p + Vector2(1, -7), Vector2(5, 2)), colors["flower"])
		_r(canvas, Rect2(p + Vector2(2, -5), Vector2(2, 2)), Color("ebd8b7"))

static func _pond(canvas: Node2D, rect: Rect2, colors: Dictionary, time: float, seed: int) -> void:
	# Orillas escalonadas: el agua y las rocas siguen dentro del rectángulo sólido.
	var left_edges := [9, 5, 3, 1, 0, 0, 1, 0, 0, 1, 2, 0, 1, 2, 3, 6, 7, 10]
	var right_edges := [11, 6, 4, 3, 1, 0, 0, 1, 2, 1, 0, 0, 1, 2, 4, 5, 8, 11]
	var row_height := rect.size.y / 18.0
	for i in range(18):
		var left: float = rect.position.x + left_edges[i] * 2
		var right: float = rect.end.x - right_edges[(i + seed * 2) % 18] * 2
		var y := floorf(rect.position.y + i * row_height)
		var height := ceilf(row_height)
		_r(canvas, Rect2(left - 2, y, right - left + 4, height), INK)
		_r(canvas, Rect2(left, y, right - left, height), Color("526b59"))
		if i > 1 and i < 16:
			_r(canvas, Rect2(left + 5, y, right - left - 10, height), Color("244b58"))
		if i > 2 and i < 15:
			_r(canvas, Rect2(left + 9, y, right - left - 18, height), Color("2c6470"))
		if i > 4 and i < 13:
			_r(canvas, Rect2(left + 13, y, right - left - 26, height), Color("337781"))
		if i % 3 == 0:
			_r(canvas, Rect2(left + 1, y, 5, 3), Color("8b9580"))
			_r(canvas, Rect2(right - 5, y + 1, 4, 3), Color("77866c"))
	_stone(canvas, rect.position + Vector2(8, 10), colors, 2)
	_stone(canvas, rect.position + Vector2(rect.size.x - 7, rect.size.y - 2), colors, 2)
	_tuft(canvas, rect.position + Vector2(12, rect.size.y - 2), colors, 1)
	_tuft(canvas, rect.position + Vector2(rect.size.x - 12, 10), colors, 2)
	for i in range(8):
		var x := int(rect.position.x) + 14 + _hash(i, seed, 3) % (int(rect.size.x) - 30)
		var y := int(rect.position.y) + 13 + _hash(i, seed, 12) % (int(rect.size.y) - 28)
		var offset := int(time * 2 + i) % 3 * 2
		_r(canvas, Rect2(x + offset, y, 8, 2), Color("72a3a1") if i % 3 == 0 else Color("468c91"))
	var lily := rect.position + rect.size * Vector2(0.69, 0.62)
	_r(canvas, Rect2(lily, Vector2(11, 5)), Color("567c4c"))
	_r(canvas, Rect2(lily + Vector2(2, -2), Vector2(7, 2)), Color("8aab6a"))
	_r(canvas, Rect2(lily + Vector2(3, 1), Vector2(4, 3)), Color("36594b"))
	_flower(canvas, lily + Vector2(3, -2), colors["flower"], colors)

static func _ruin(canvas: Node2D, rect: Rect2, colors: Dictionary, seed: int) -> void:
	_r(canvas, rect.grow(3), INK)
	_r(canvas, rect, Color("373641"))
	for y in range(int(rect.position.y), int(rect.end.y), 14):
		for x in range(int(rect.position.x), int(rect.end.x), 22):
			var tile := Rect2(x, y, 21, 13).intersection(rect)
			var n := _hash(x, y, seed)
			_r(canvas, tile, Color("5d5863") if n % 4 == 0 else Color("4b4b58"))
			_r(canvas, Rect2(tile.position + Vector2(1, 1), Vector2(tile.size.x - 2, 2)), Color("74747c"))
			if n % 4 == 0:
				_r(canvas, Rect2(tile.position + Vector2(9, 4), Vector2(2, 5)), Color("30343e"))
				_r(canvas, Rect2(tile.position + Vector2(6, 8), Vector2(4, 2)), Color("30343e"))
	_r(canvas, Rect2(rect.position + Vector2(3, 3), Vector2(rect.size.x - 6, 4)), Color("89848b"))
	for i in range(5):
		var x := rect.position.x + 8 + i * 18
		_r(canvas, Rect2(x, rect.position.y + 6, 8, 3 + i % 3 * 2), colors["leaf_shadow"])
		_r(canvas, Rect2(x + 2, rect.position.y + 6, 4, 2), colors["light"])
	_r(canvas, Rect2(rect.position + Vector2(5, rect.size.y - 7), Vector2(rect.size.x - 10, 4)), colors["leaf_dark"])

static func _column(canvas: Node2D, point: Vector2, colors: Dictionary) -> void:
	_r(canvas, Rect2(point, Vector2(30, 93)), INK)
	_r(canvas, Rect2(point + Vector2(5, 8), Vector2(20, 76)), Color("4b3e5b"))
	_r(canvas, Rect2(point + Vector2(7, 8), Vector2(4, 75)), Color("796486"))
	_r(canvas, Rect2(point + Vector2(14, 8), Vector2(3, 75)), Color("5e4c6d"))
	_r(canvas, Rect2(point + Vector2(23, 8), Vector2(2, 76)), Color("302738"))
	for y in [0, 5, 82, 88]:
		_r(canvas, Rect2(point + Vector2(1, y), Vector2(28, 4)), Color("62516e"))
		_r(canvas, Rect2(point + Vector2(2, y), Vector2(26, 1)), colors["accent"].darkened(0.27))

static func _window(canvas: Node2D, point: Vector2, colors: Dictionary) -> void:
	_r(canvas, Rect2(point + Vector2(3, 0), Vector2(35, 23)), INK)
	_r(canvas, Rect2(point + Vector2(0, 5), Vector2(41, 18)), Color("6b567b"))
	_r(canvas, Rect2(point + Vector2(4, 3), Vector2(33, 19)), Color("151e31"))
	_r(canvas, Rect2(point + Vector2(9, 6), Vector2(23, 15)), Color("222b43"))
	_r(canvas, Rect2(point + Vector2(18, 4), Vector2(3, 20)), Color("58455f"))
	_r(canvas, Rect2(point + Vector2(2, 23), Vector2(37, 3)), colors["light"])
	_r(canvas, Rect2(point + Vector2(13, 10), Vector2(2, 2)), colors["glow"])

static func _torch(canvas: Node2D, point: Vector2, time: float, colors: Dictionary) -> void:
	var phase := int(time * 7 + point.y) % 3
	_r(canvas, Rect2(point + Vector2(-13, -25), Vector2(26, 23)), Color(0.57, 0.31, 0.75, 0.065))
	_r(canvas, Rect2(point + Vector2(-8, -32), Vector2(16, 35)), Color(0.65, 0.40, 0.84, 0.08))
	_r(canvas, Rect2(point + Vector2(-4, -5), Vector2(8, 23)), INK)
	_r(canvas, Rect2(point + Vector2(-2, -4), Vector2(4, 20)), Color("716070"))
	_r(canvas, Rect2(point + Vector2(-8, -8), Vector2(16, 5)), Color("997b94"))
	_r(canvas, Rect2(point + Vector2(-6, -18), Vector2(12, 10)), Color("643aa3"))
	_r(canvas, Rect2(point + Vector2(-4, -24 - phase * 2), Vector2(8, 15 + phase * 2)), Color("a669dd"))
	_r(canvas, Rect2(point + Vector2(-1, -20 - phase), Vector2(4, 11 + phase)), colors["glow"])
	_r(canvas, Rect2(point + Vector2(3, -28 - phase), Vector2(2, 5)), Color("a669dd"))

static func _pixel_moon(canvas: Node2D, point: Vector2, unit: int, moon: Color, sky: Color) -> void:
	var rows := ["   mmmmm   ", "  mmmss    ", " mmmsss    ", "mmmssss    ", "mmmssss    ",
		"mmmssss    ", "mmmmsss    ", " mmmmmss m ", "  mmmmmmm  ", "   mmmmm   "]
	_pattern(canvas, rows, point, unit, {"m": moon, "s": sky})

static func _gate(canvas: Node2D, colors: Dictionary, open: bool, time: float) -> void:
	_r(canvas, Rect2(851, 251, 70, 116), INK)
	_r(canvas, Rect2(855, 254, 62, 110), Color("44443c"))
	_r(canvas, Rect2(861, 260, 50, 104), Color("101f27") if open else Color("332c38"))
	for x in [854, 910]:
		_r(canvas, Rect2(x, 251, 7, 116), Color("857959"))
		_r(canvas, Rect2(x, 254, 2, 109), Color("b1a076"))
		for y in range(259, 358, 17):
			_r(canvas, Rect2(x, y, 7, 2), Color("4e4c3e"))
	_r(canvas, Rect2(854, 251, 64, 9), Color("8e845e"))
	_r(canvas, Rect2(857, 252, 58, 2), Color("c4b280"))
	if open:
		for i in range(4):
			var alpha := 0.07 + float(i) * 0.04
			_r(canvas, Rect2(866 + i * 4, 264 + i * 5, 40 - i * 8, 95 - i * 10), Color(0.70, 0.79, 0.49, alpha))
		for i in range(3):
			var y := 287 + i * 24 + int(time * 8) % 12
			_r(canvas, Rect2(879 + i % 2 * 10, y, 3, 3), colors["glow"])
		_r(canvas, Rect2(879, 311, 13, 3), Color("c9de9b"))
		_r(canvas, Rect2(889, 307, 3, 11), Color("c9de9b"))
		_r(canvas, Rect2(892, 310, 3, 5), Color("c9de9b"))
	else:
		for x in range(865, 908, 7):
			_r(canvas, Rect2(x, 262, 5, 100), Color("594a3b"))
			_r(canvas, Rect2(x, 264, 1, 96), Color("847455"))
		for y in [277, 340]:
			_r(canvas, Rect2(862, y, 47, 5), Color("222b30"))
			for x in [867, 901]:
				_r(canvas, Rect2(x, y + 1, 2, 2), Color("a0a098"))
		_r(canvas, Rect2(880, 307, 11, 12), Color("d9bc69"))
		_r(canvas, Rect2(882, 303, 7, 5), Color("907b48"))
		_r(canvas, Rect2(884, 310, 3, 5), Color("654a39"))

static func _frame(canvas: Node2D, colors: Dictionary, stage: int) -> void:
	_r(canvas, Rect2(40, 124, 880, 2), colors["accent"].darkened(0.46))
	_r(canvas, Rect2(40, 478, 880, 2), colors["dark"])
	_r(canvas, Rect2(40, 126, 2, 352), colors["accent"].darkened(0.55))
	_r(canvas, Rect2(918, 126, 2, 352), colors["accent"].darkened(0.55))
	for point in [Vector2(38, 123), Vector2(916, 123), Vector2(38, 476), Vector2(916, 476)]:
		_r(canvas, Rect2(point, Vector2(6, 6)), INK)
		_r(canvas, Rect2(point + Vector2(1, 1), Vector2(3, 3)), colors["accent"])
	if stage < 2:
		for y in range(167, 454, 27):
			_tuft(canvas, Vector2(30, y), colors, y % 3)
			_tuft(canvas, Vector2(926, y + 10), colors, y % 2)

static func _sparks(canvas: Node2D, colors: Dictionary, stage: int, time: float) -> void:
	for i in range(15):
		var x := 75 + _hash(i, stage, 31) % 785
		var y := 149 + _hash(i, stage, 62) % 310
		var flicker := int(time * 3 + i * 0.73) % 5
		if flicker < 2:
			continue
		var drift := int(time * 2 + i) % 3 * 2
		_r(canvas, Rect2(x - 2, y + drift - 2, 6, 6), Color(colors["glow"], 0.055))
		_r(canvas, Rect2(x, y + drift, 2, 2), Color(colors["glow"], 0.65 if flicker == 4 else 0.36))
