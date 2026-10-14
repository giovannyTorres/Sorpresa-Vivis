extends RefCounted

const HEART := ["0110110", "1221221", "1222221", "1222221", "0122210", "0012100", "0001000"]
const PELUSA := [
	"   o       o    ", "  ohh     oho   ", " oohhooooohhoo  ",
	" ophhhpppphhpo  ", "oophhhppppphhoo ", " ophhpppppppppo ",
	"ooppoyyppoyypoo ", " oppoyeppoyepo  ", " ooppppppppppoo ",
	"  opppphhppppo  ", " ooppppooppppoo ", "  ooppppppppoo  ",
	" oooppppppppooo ", "    oooooooo    "
]

static func pelusa(canvas: Node2D, point: Vector2, flash: bool, time: float) -> void:
	canvas.draw_rect(Rect2(point - Vector2(23, 3), Vector2(46, 8)), Color(0.02, 0.03, 0.06, 0.35))
	var colors := {"o": Color("191526"), "p": Color("49364f"),
		"h": Color("795478"), "y": Color("d8b8ff"), "e": Color("71cddd")}
	for y in range(PELUSA.size()):
		for x in range(PELUSA[y].length()):
			var key: String = PELUSA[y][x]
			if key == " ":
				continue
			var color: Color = Color("ff8db6") if flash else colors[key]
			canvas.draw_rect(Rect2((point + Vector2(x - 8, y - 14) * 3 + Vector2(0, int(sin(time * 5) * 2))).round(), Vector2(3, 3)), color)

static func heart(canvas: Node2D, center: Vector2, pixel: float = 2, color: Color = Color("ff8db6")) -> void:
	for y in range(HEART.size()):
		for x in range(7):
			var value: String = HEART[y][x]
			if value == "0":
				continue
			var fill := color.darkened(0.48) if value == "1" else color
			canvas.draw_rect(Rect2((center + Vector2(x - 3.5, y - 3.5) * pixel).round(), Vector2(pixel, pixel)), fill)
	canvas.draw_rect(Rect2((center + Vector2(-1.5, -2.5) * pixel).round(), Vector2(pixel, pixel)), Color("ffe7ef"))

static func sparkle(canvas: Node2D, point: Vector2, color: Color, size: float = 3) -> void:
	canvas.draw_rect(Rect2(point - Vector2(size / 2, size * 1.5), Vector2(size, size * 3)), color)
	canvas.draw_rect(Rect2(point - Vector2(size * 1.5, size / 2), Vector2(size * 3, size)), color)

static func pink_magic(canvas: Node2D, point: Vector2, velocity: Vector2, time: float) -> void:
	var direction := velocity.normalized()
	for i in range(5):
		var trail := point - direction * (8 + i * 6) + direction.orthogonal() * sin(time * 18 + i) * 4
		canvas.draw_rect(Rect2(trail.round(), Vector2(4 - i * 0.4, 4 - i * 0.4)), Color(1, 0.45, 0.7, 0.7 - i * 0.1))
	heart(canvas, point, 2.0)
	sparkle(canvas, point + Vector2(7, -8), Color("fff3ef"), 1)

static func lunar_magic(canvas: Node2D, point: Vector2, velocity: Vector2, time: float) -> void:
	var direction := velocity.normalized()
	for i in range(4):
		canvas.draw_rect(Rect2((point - direction * (i * 8)).round() - Vector2(4, 4), Vector2(8, 8)), Color(0.45, 0.2, 0.7, 0.5 - i * 0.09))
	canvas.draw_rect(Rect2(point - Vector2(7, 10), Vector2(14, 20)), Color("5c328d"))
	canvas.draw_rect(Rect2(point - Vector2(10, 7), Vector2(20, 14)), Color("a367e5"))
	canvas.draw_rect(Rect2(point + Vector2(-2, -6), Vector2(12, 12)), Color("302039"))
	sparkle(canvas, point + Vector2(sin(time * 9) * 14, -13), Color("ce9fff"), 1)

static func digital_pulse(canvas: Node2D, center: Vector2, progress: float) -> void:
	var radius := 175 * progress
	for i in range(24):
		var point := (center + Vector2.from_angle(i * TAU / 24) * radius).round()
		var size := 4 + i % 3 * 2
		canvas.draw_rect(Rect2(point - Vector2.ONE * size / 2, Vector2.ONE * size), Color(0.27, 0.68, 1.0, 1 - progress))
		if i % 3 == 0:
			canvas.draw_rect(Rect2(point - Vector2.ONE * (size / 2 + 2), Vector2.ONE * (size + 4)), Color(0.6, 0.9, 1, 1 - progress), false, 1)

static func lunar_shield(canvas: Node2D, center: Vector2, time: float) -> void:
	for i in range(30):
		var point := center + Vector2.from_angle(i * TAU / 30 + time * 0.3) * 54
		canvas.draw_rect(Rect2(point.round(), Vector2(3, 3)), Color("b78bea"))
	for i in range(4):
		var point := center + Vector2.from_angle(i * TAU / 4 - time * 0.6) * 59
		sparkle(canvas, point.round(), Color("d5a8ff"), 2)
