extends RefCounted
## Regiones y pivotes del atlas original, que se conserva sin recortar ni alterar.

const ATLAS_PATH := "res://playable/assets/characters.png"
const EMOTIONS := preload("res://playable/assets/character_emotions.png")
const EMOTION_REGIONS := [
	[Rect2(33, 33, 292, 340), Rect2(359, 27, 284, 356)],
	[Rect2(76, 456, 236, 250), Rect2(372, 417, 283, 289)],
	[Rect2(31, 769, 298, 326), Rect2(391, 717, 280, 385)],
	[Rect2(31, 1175, 289, 221), Rect2(369, 1131, 302, 285)]
]
const EMOTION_PIVOTS := [
	[Vector2(137, 340), Vector2(151, 356)],
	[Vector2(124, 250), Vector2(158, 289)],
	[Vector2(139, 326), Vector2(124, 385)],
	[Vector2(174, 221), Vector2(151, 285)]
]
const PORTRAITS := [Rect2(705, 8, 375, 360), Rect2(713, 377, 345, 329), Rect2(704, 716, 360, 380), Rect2(710, 1101, 353, 330)]
const REGIONS := [
	[Rect2(42, 55, 206, 283), Rect2(323, 56, 223, 282), Rect2(612, 60, 228, 284), Rect2(900, 64, 346, 276)],
	[Rect2(35, 417, 220, 211), Rect2(306, 418, 247, 210), Rect2(581, 429, 253, 199), Rect2(880, 426, 363, 209)],
	[Rect2(41, 658, 206, 306), Rect2(339, 661, 209, 308), Rect2(604, 667, 210, 299), Rect2(874, 683, 369, 281)],
	[Rect2(32, 1010, 241, 215), Rect2(312, 1015, 249, 208), Rect2(593, 1030, 265, 198), Rect2(884, 994, 359, 235)]
]
const PIVOTS := [
	[Vector2(98, 283), Vector2(122, 282), Vector2(118, 284), Vector2(125, 276)],
	[Vector2(150, 211), Vector2(163, 210), Vector2(165, 199), Vector2(144, 209)],
	[Vector2(112, 306), Vector2(106, 308), Vector2(96, 299), Vector2(126, 281)],
	[Vector2(153, 215), Vector2(143, 208), Vector2(158, 198), Vector2(148, 235)]
]

static func draw_character(canvas: Node2D, atlas: Texture2D, row: int, feet: Vector2,
		size: float = 96.0, frame: int = 0, flip: bool = false,
		tint: Color = Color.WHITE) -> void:
	var sheet := atlas if frame < 4 else EMOTIONS
	var source: Rect2 = REGIONS[row][frame] if frame < 4 else EMOTION_REGIONS[row][frame - 4]
	var pivot: Vector2 = PIVOTS[row][frame] if frame < 4 else EMOTION_PIVOTS[row][frame - 4]
	var reference: Rect2 = REGIONS[row][0]
	var scale_value := size / reference.size.y
	var destination := Rect2((-pivot * scale_value).round(), (source.size * scale_value).round())
	# Sombra escalonada, alineada al suelo y no al lienzo transparente del sprite.
	var shadow_width := 42 if row == 0 or row == 2 else 46
	canvas.draw_rect(Rect2(feet + Vector2(-shadow_width / 2, -3), Vector2(shadow_width, 6)), Color(0.03, 0.05, 0.08, 0.3))
	canvas.draw_rect(Rect2(feet + Vector2(-shadow_width / 2 + 5, 3), Vector2(shadow_width - 10, 3)), Color(0.03, 0.05, 0.08, 0.2))
	canvas.draw_set_transform(feet.round(), 0, Vector2(-1 if flip else 1, 1))
	canvas.draw_texture_rect_region(sheet, destination, source, tint)
	canvas.draw_set_transform(Vector2.ZERO)

static func portrait(_atlas: Texture2D, row: int, victory: bool = false) -> AtlasTexture:
	var texture := AtlasTexture.new()
	texture.atlas = EMOTIONS
	texture.region = EMOTION_REGIONS[row][1] if victory else PORTRAITS[row]
	return texture
