#include "visuals.hpp"
#include <array>
#include <cstdio>
#include <sstream>

namespace vivis {
namespace {
constexpr Color Ink{12, 21, 29, 255};
constexpr Color Panel{22, 36, 46, 255};
constexpr Color Raised{32, 49, 61, 255};
constexpr Color Line{61, 82, 94, 255};
constexpr Color Cream{250, 232, 207, 255};
constexpr Color Soft{155, 176, 188, 255};
constexpr Color Pink{246, 143, 184, 255};
constexpr Color Blue{133, 208, 239, 255};
constexpr Color Green{172, 215, 139, 255};
constexpr Color Purple{185, 146, 227, 255};
constexpr Color Gold{238, 204, 140, 255};
constexpr std::array<Color, 4> CharacterColors{Pink, Green, Blue, Purple};
constexpr const char* Names[]{"VIVIS", "WISKY", "GIOVANNY", "MALIKETH"};
constexpr const char* ShortNames[]{"VIVIS", "WISKY", "GIOVANNY", "MALIKETH"};

Font face(const Artwork& art, bool small = false) {
    Font font = small ? art.smallFont : art.font;
    return font.texture.id ? font : GetFontDefault();
}

float textWidth(const Artwork& art, const std::string& value, float size, bool small = false) {
    return MeasureTextEx(face(art, small), value.c_str(), size, 0.6f).x;
}

void text(const Artwork& art, const std::string& value, float x, float y, float size,
          Color color = Cream, bool small = false) {
    DrawTextEx(face(art, small), value.c_str(), {std::round(x), std::round(y)}, size, 0.6f, color);
}

void centered(const Artwork& art, const std::string& value, float x, float y,
              float size, Color color = Cream, bool small = false) {
    text(art, value, x - textWidth(art, value, size, small) * .5f, y, size, color, small);
}

void wrapped(const Artwork& art, const std::string& value, Rectangle box, float size,
             Color color = Cream, float leading = 1.4f, bool small = false) {
    std::istringstream words(value);
    std::string word, line;
    float y = box.y;
    while (words >> word) {
        const std::string next = line.empty() ? word : line + " " + word;
        if (!line.empty() && textWidth(art, next, size, small) > box.width) {
            text(art, line, box.x, y, size, color, small);
            line = word;
            y += size * leading;
        } else line = next;
        if (y + size > box.y + box.height) return;
    }
    if (!line.empty()) text(art, line, box.x, y, size, color, small);
}

void pixelFrame(Rectangle r, Color accent = Line, Color fill = Panel, int border = 2) {
    const int x = int(r.x), y = int(r.y), w = int(r.width), h = int(r.height);
    DrawRectangle(x + 4, y, w - 8, h, fill);
    DrawRectangle(x, y + 4, w, h - 8, fill);
    DrawRectangle(x + 6, y, w - 12, border, accent);
    DrawRectangle(x + 6, y + h - border, w - 12, border, accent);
    DrawRectangle(x, y + 6, border, h - 12, accent);
    DrawRectangle(x + w - border, y + 6, border, h - 12, accent);
    for (int side = 0; side < 2; ++side) {
        for (int end = 0; end < 2; ++end) {
            const int cx = side ? x + w - 6 : x + 2;
            const int cy = end ? y + h - 6 : y + 2;
            DrawRectangle(cx, cy, 4, 4, accent);
            DrawRectangle(cx + (side ? 1 : 0), cy + (end ? 1 : 0), 1, 1, Cream);
        }
    }
}

void grid(Rectangle box, int step = 24) {
    for (int x = int(box.x); x < int(box.x + box.width); x += step)
        DrawRectangle(x, int(box.y), 1, int(box.height), Fade(Line, .16f));
    for (int y = int(box.y); y < int(box.y + box.height); y += step)
        DrawRectangle(int(box.x), y, int(box.width), 1, Fade(Line, .16f));
}

void pattern(const char* const* rows, int rowCount, Vector2 point, int scale, Color color) {
    for (int y = 0; y < rowCount; ++y)
        for (int x = 0; rows[y][x]; ++x)
            if (rows[y][x] == '#')
                DrawRectangle(int(point.x) + x * scale, int(point.y) + y * scale, scale, scale, color);
}

void heart(Vector2 p, int scale, Color color) {
    static const char* rows[]{".##.##.", "#######", "#######", ".#####.", "..###..", "...#..."};
    pattern(rows, 6, {p.x, p.y + float(scale)}, scale, Fade(BLACK, .45f));
    pattern(rows, 6, p, scale, color);
    DrawRectangle(int(p.x) + scale * 2, int(p.y) + scale, scale, scale, Fade(Cream, .6f));
}

void paw(Vector2 p, int scale, Color color) {
    static const char* rows[]{"..#.#..", ".##.##.", ".#...#.", ".......", "..###..", ".#####.", "..###.."};
    pattern(rows, 7, p, scale, color);
}

void spark(Vector2 p, Color color, int s = 2) {
    DrawRectangle(int(p.x) - s * 2, int(p.y), s * 5, s, color);
    DrawRectangle(int(p.x), int(p.y) - s * 2, s, s * 5, color);
}

void forest(float uiTime, bool reducedMotion) {
    ClearBackground(Ink);
    DrawRectangleGradientV(0, 0, Width, Height, {24, 43, 54, 255}, Ink);
    for (int layer = 0; layer < 3; ++layer) {
        const Color shade = layer == 0 ? Color{32, 55, 65, 255}
                                      : (layer == 1 ? Color{24, 47, 55, 255} : Color{17, 35, 43, 255});
        for (int i = -1; i < 18; ++i) {
            const int x = i * 89 + layer * 27;
            const int height = 104 + ((i + 4) * 53 + layer * 31) % 150;
            const int base = 610 + layer * 42;
            DrawRectangle(x + 28, base - height, 12, height + 110, shade);
            for (int branch = 0; branch < 5; ++branch) {
                const int width = 20 + branch * 13;
                DrawRectangle(x + 34 - width / 2, base - height + branch * 24, width, 32, shade);
            }
        }
    }
    for (int i = 0; i < 24; ++i) {
        const float a = reducedMotion ? .45f : .35f + .15f * std::sin(uiTime * 1.1f + i);
        DrawRectangle(40 + (i * 173) % 1192, 43 + (i * 83) % 457, 2, 2, Fade(Gold, a));
    }
}

void artBackground(const Game& game, const Artwork& art, float uiTime, bool title) {
    if (title) {
        forest(uiTime, game.preferences.reducedMotion);
        if (art.title.id && art.title.width > 0 && art.title.height > 0) {
            const float scale = std::max(float(Width) / art.title.width, float(Height) / art.title.height);
            const Rectangle source{0, 0, float(art.title.width), float(art.title.height)};
            const Rectangle target{(Width - art.title.width * scale) * .5f, (Height - art.title.height * scale) * .5f,
                                   art.title.width * scale, art.title.height * scale};
            DrawTexturePro(art.title, source, target, {}, 0, WHITE);
            DrawRectangleGradientH(0, 0, Width, Height, Fade(Ink, .20f), Fade(Ink, .84f));
            DrawRectangleGradientV(0, 0, Width, Height, Fade(Ink, .03f), Fade(Ink, .43f));
        }
    } else {
        DrawRectangle(0, 0, Width, Height, Fade(Ink, .95f));
        grid({0, 0, float(Width), float(Height)}, 32);
    }
}

int itemButton(const Artwork& art, Rectangle r, const std::string& label, int index,
               int selected, Vector2 mouse, bool mouseActive, bool primary = false,
               Color accent = Pink) {
    const bool hover = mouseActive && inside(mouse, r);
    const bool active = hover || (!mouseActive && selected == index);
    const Color fill = primary ? (active ? Cream : accent) : (active ? Raised : Panel);
    const Color outline = primary ? (active ? Cream : accent) : (active ? accent : Line);
    DrawRectangle(int(r.x + 2), int(r.y + r.height), int(r.width - 4), 4, Fade(BLACK, .32f));
    pixelFrame(r, outline, fill, active ? 3 : 2);
    const Color ink = primary ? Ink : (active ? Cream : Soft);
    centered(art, label, r.x + r.width * .5f, r.y + (r.height - 25) * .5f - 1, 25, ink);
    if (active) {
        const float cx = r.x + 18, cy = r.y + r.height * .5f;
        DrawRectangle(int(cx), int(cy - 4), 3, 9, ink);
        DrawRectangle(int(cx + 3), int(cy - 2), 3, 5, ink);
    }
    return hover ? index : -1;
}

void navigationHint(const Artwork& art, const std::string& extra = "") {
    centered(art, "Flechas / cruceta: elegir     Enter / A: confirmar     Esc / B: volver",
             Width * .5f, 686, 16, Soft, true);
    if (!extra.empty()) centered(art, extra, Width * .5f, 653, 16, Soft, true);
}

void pageHeading(const Artwork& art, const std::string& title, const std::string& subtitle) {
    centered(art, title, Width * .5f + 2, 61, 46, Fade(Pink, .3f));
    centered(art, title, Width * .5f, 58, 46, Cream);
    centered(art, subtitle, Width * .5f, 113, 18, Soft, true);
}

void keycap(const Artwork& art, const std::string& key, Rectangle r, Color accent) {
    pixelFrame(r, Fade(accent, .7f), Raised, 1);
    centered(art, key, r.x + r.width * .5f, r.y + 7, 18, accent);
}

std::string duration(float time) {
    const int seconds = std::max(0, int(time));
    char buffer[24];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d", seconds / 60, seconds % 60);
    return buffer;
}

const Room* activeRoom(const Game& game) {
    return game.currentRoom >= 0 && game.currentRoom < int(game.rooms.size())
        ? &game.rooms[game.currentRoom] : nullptr;
}

Color roomColor(RoomKind kind) {
    switch (kind) {
        case RoomKind::Start: return Pink;
        case RoomKind::Treasure: case RoomKind::Shop: return Gold;
        case RoomKind::Sanctuary: return Green;
        case RoomKind::Challenge: case RoomKind::Boss: return Purple;
        default: return Soft;
    }
}

void roomIcon(RoomKind kind, Vector2 center, int scale = 2, Color tint = WHITE) {
    static const char* entrance[]{"...#...", "..###..", ".#####.", "..###..", "..###..", ".......", "......."};
    static const char* encounter[]{".......", ".#...#.", "..#.#..", "...#...", "..#.#..", ".#...#.", "......."};
    static const char* treasure[]{".......", ".#####.", ".#...#.", ".#####.", ".##.##.", ".#####.", "......."};
    static const char* shop[]{"...#...", "..####.", ".#.#...", "..###..", "...#.#.", ".####..", "...#..."};
    static const char* sanctuary[]{".......", "...#...", "...#...", ".#####.", "...#...", "...#...", "......."};
    static const char* challenge[]{"..###..", "..###..", "..###..", "...#...", ".......", "...#...", "......."};
    static const char* boss[]{".......", ".#.#.#.", ".#####.", ".#####.", "..###..", ".#####.", "......."};
    const char* const* glyph = encounter;
    switch (kind) {
        case RoomKind::Start: glyph = entrance; break;
        case RoomKind::Treasure: glyph = treasure; break;
        case RoomKind::Shop: glyph = shop; break;
        case RoomKind::Sanctuary: glyph = sanctuary; break;
        case RoomKind::Challenge: glyph = challenge; break;
        case RoomKind::Boss: glyph = boss; break;
        default: break;
    }
    pattern(glyph, 7, {center.x - 3.5f * scale, center.y - 3.5f * scale}, scale, tint);
}

void coin(Vector2 point, int scale = 2) {
    static const char* outer[]{"..###..", ".#####.", "#######", "#######", "#######", ".#####.", "..###.."};
    static const char* inner[]{".......", "...#...", "..##...", "...#...", "...#...", "..###..", "......."};
    pattern(outer, 7, point, scale, Gold);
    pattern(inner, 7, point, scale, {143, 105, 59, 255});
}

void itemEmblem(ItemId id, Rectangle target, Color accent) {
    const int scale = std::max(1, int(std::min(target.width, target.height) / 12));
    const Vector2 origin{std::round(target.x + (target.width - 11 * scale) * .5f),
                         std::round(target.y + (target.height - 11 * scale) * .5f)};
    static const char* boots[]{"...........", "..##..##...", "..##..##...", "..##..##...", "..###.###..", "..###.###..", ".####.####.", ".####.####.", "...........", "...........", "..........."};
    static const char* bow[]{"...........", ".##.....##.", ".###...###.", ".####.####.", "..#######..", "...#####...", "..#######..", "..##...##..", "..#.....#..", "...........", "..........."};
    static const char* shield[]{"...........", "..#######..", ".#########.", ".#########.", ".#########.", "..#######..", "..#######..", "...#####...", "....###....", ".....#.....", "..........."};
    static const char* bottle[]{"....###....", "....###....", "...#####...", "...#####...", "...#####...", "...#####...", "...#####...", "...#####...", "...#####...", "...#####...", "..........."};
    static const char* fang[]{"...........", "..#######..", "..#######..", "...#####...", "...#####...", "....###....", "....###....", ".....##....", ".....#.....", "...........", "..........."};
    static const char* bell[]{".....#.....", "....###....", "...#####...", "...#####...", "..#######..", "..#######..", ".#########.", ".#########.", "....###....", "...........", "..........."};
    static const char* leaf[]{"...........", ".......###.", ".....#####.", "....######.", "...#######.", "..#######..", "..######...", "..#####....", "..###......", "..#........", "..........."};
    static const char* cloud[]{"...........", "....###....", "..#######..", ".#########.", "###########", "###########", ".#########.", "..#######..", "...#...#...", "...........", "..........."};
    static const char* chip[]{"...#.#.#...", "..#######..", ".#########.", "###.....###", ".##.###.##.", "###.###.###", ".##.###.##.", "###.....###", ".#########.", "..#######..", "...#.#.#..."};
    const char* const* glyph = shield;
    switch (id) {
        case ItemId::HeartBrooch: heart({origin.x + scale * 2, origin.y + scale * 2}, scale, accent); return;
        case ItemId::SoftBoots: glyph = boots; break;
        case ItemId::QuickRibbon: glyph = bow; break;
        case ItemId::TwinHearts:
            heart({origin.x, origin.y + scale}, scale, accent);
            heart({origin.x + scale * 4, origin.y + scale * 4}, scale, accent); return;
        case ItemId::Milk: glyph = bottle; break;
        case ItemId::ExtraLife:
            heart({origin.x + scale * 2, origin.y + scale}, scale, Pink);
            spark({origin.x + scale * 8, origin.y + scale * 8}, Gold, std::max(1, scale / 2)); return;
        case ItemId::VenomFang: glyph = fang; break;
        case ItemId::WiskyBell: glyph = bell; break;
        case ItemId::Catnip: glyph = leaf; break;
        case ItemId::ToxicCloud: glyph = cloud; break;
        case ItemId::DigitalChip: glyph = chip; break;
        case ItemId::LuckyWhisker: paw({origin.x + scale * 2, origin.y + scale * 2}, scale, Gold); return;
        default: break;
    }
    pattern(glyph, 11, {origin.x, origin.y + float(scale)}, scale, Fade(BLACK, .4f));
    pattern(glyph, 11, origin, scale, accent);
    if (id == ItemId::Milk) DrawRectangle(int(origin.x + scale * 4), int(origin.y + scale * 4), scale * 3, scale * 3, Pink);
    if (id == ItemId::DigitalChip) DrawRectangle(int(origin.x + scale * 4), int(origin.y + scale * 4), scale * 3, scale * 3, Cream);
}

void hearts(const Game& game, Vector2 origin) {
    static const char* half[]{".##....", "####...", "####...", ".###...", "..##...", "...#..."};
    const int containers = std::clamp((game.maxHp + 1) / 2, 1, 6);
    for (int i = 0; i < containers; ++i) {
        const Vector2 point{origin.x + i * 27, origin.y};
        const int health = game.hp - i * 2;
        heart(point, 3, health >= 2 ? Pink : Color{56, 62, 75, 255});
        if (health == 1) pattern(half, 6, point, 3, Pink);
    }
}

void roomMap(const Game& game, const Artwork& art, Rectangle area, bool large) {
    if (game.rooms.empty()) return;
    std::vector<bool> visible(game.rooms.size(), false);
    for (int i = 0; i < int(game.rooms.size()); ++i) {
        if (!game.rooms[i].visited && i != game.currentRoom) continue;
        visible[i] = true;
        for (const int neighbor : game.rooms[i].neighbors)
            if (neighbor >= 0 && neighbor < int(game.rooms.size())) visible[neighbor] = true;
    }
    int minX = 100000, minY = 100000, maxX = -100000, maxY = -100000;
    for (int i = 0; i < int(game.rooms.size()); ++i) if (visible[i]) {
        minX = std::min(minX, game.rooms[i].x); maxX = std::max(maxX, game.rooms[i].x);
        minY = std::min(minY, game.rooms[i].y); maxY = std::max(maxY, game.rooms[i].y);
    }
    if (minX > maxX) return;
    const float step = std::min(large ? 86.0f : 23.0f,
        std::min(area.width / (maxX - minX + 1), area.height / (maxY - minY + 1)));
    const float roomSize = std::max(large ? 18.0f : 5.0f, step * .66f);
    auto position = [&](const Room& room) {
        return Vector2{area.x + area.width * .5f + (room.x - (maxX + minX) * .5f) * step,
                       area.y + area.height * .5f + (room.y - (maxY + minY) * .5f) * step};
    };
    for (int i = 0; i < int(game.rooms.size()); ++i) if (visible[i]) {
        for (const int neighbor : game.rooms[i].neighbors) {
            if (neighbor <= i || neighbor >= int(game.rooms.size()) || !visible[neighbor]) continue;
            DrawLineEx(position(game.rooms[i]), position(game.rooms[neighbor]), large ? 5 : 2, Fade(Line, .75f));
        }
    }
    for (int i = 0; i < int(game.rooms.size()); ++i) if (visible[i]) {
        const auto& room = game.rooms[i];
        const bool current = i == game.currentRoom;
        const bool visited = room.visited || current;
        const Vector2 center = position(room);
        const Rectangle tile{std::round(center.x - roomSize * .5f), std::round(center.y - roomSize * .5f),
                             std::floor(roomSize), std::floor(roomSize)};
        const Color outline = current ? Pink : (visited ? roomColor(room.kind) : Fade(Soft, .55f));
        DrawRectangleRec(tile, current ? Raised : (room.cleared ? Color{32, 48, 46, 255} : Ink));
        DrawRectangleLinesEx(tile, current ? (large ? 3 : 2) : 1, outline);
        if (large && roomSize >= 25) {
            if (visited) roomIcon(room.kind, center, roomSize >= 45 ? 3 : 2, roomColor(room.kind));
            else centered(art, "?", center.x, center.y - 10, 23, Soft);
            if (room.cleared) DrawRectangle(int(tile.x + tile.width - 7), int(tile.y + tile.height - 7), 4, 4, Green);
        } else if (visited && roomSize >= 8) {
            DrawRectangle(int(center.x - 1), int(center.y - 1), 3, 3, current ? Cream : roomColor(room.kind));
        }
    }
}

int closeOverlay(const Artwork& art, Vector2 mouse, bool mouseActive, const std::string& label) {
    return itemButton(art, {450, 625, 380, 48}, label, 0, 0, mouse, mouseActive, true, Blue);
}

std::string number(float value, int precision = 1) {
    char output[40];
    std::snprintf(output, sizeof(output), precision == 0 ? "%.0f" : "%.1f", value);
    return output;
}
}

int menuItemCount(Screen screen, bool hasSave) {
    switch (screen) {
        case Screen::Title: return hasSave ? 6 : 5;
        case Screen::Paused: return 4;
        case Screen::Lost: return 2;
        case Screen::Won: return 3;
        case Screen::Settings: return 6;
        case Screen::Controls: return 1;
        case Screen::Gallery: return 3;
        case Screen::Reward: return 4;
        default: return 0;
    }
}

const char* menuItem(Screen screen, int index, bool hasSave) {
    static const char* title[]{"Nueva aventura", "Personajes", "Ajustes", "Controles", "Salir"};
    static const char* saved[]{"Continuar", "Nueva aventura", "Personajes", "Ajustes", "Controles", "Salir"};
    static const char* paused[]{"Continuar", "Ajustes", "Controles", "Volver al inicio"};
    static const char* lost[]{"Reintentar", "Volver al inicio"};
    static const char* won[]{"Volver a jugar", "Personajes", "Volver al inicio"};
    static const char* settings[]{"Sonido", "Volumen", "Pantalla completa", "Movimiento reducido", "Modo tranquilo", "Volver"};
    static const char* controls[]{"Volver"};
    static const char* gallery[]{"Anterior", "Siguiente", "Volver"};
    static const char* rewards[]{"Elegir mejora 1", "Elegir mejora 2", "Elegir mejora 3", "Volver"};
    if (index < 0 || index >= menuItemCount(screen, hasSave)) return "";
    switch (screen) {
        case Screen::Title: return hasSave ? saved[index] : title[index];
        case Screen::Paused: return paused[index];
        case Screen::Lost: return lost[index];
        case Screen::Won: return won[index];
        case Screen::Settings: return settings[index];
        case Screen::Controls: return controls[index];
        case Screen::Gallery: return gallery[index];
        case Screen::Reward: return rewards[index];
        default: return "";
    }
}

int drawMenu(const Game& game, const Artwork& art, Screen screen, int selected,
             Vector2 mouse, bool mouseActive, float uiTime, int galleryCharacter) {
    if (screen == Screen::Playing) return -1;
    if (screen == Screen::Reward) return drawReward(game, art, selected, mouse, mouseActive);
    artBackground(game, art, uiTime, screen == Screen::Title);
    int hovered = -1;
    auto button = [&](Rectangle r, int index, bool primary = false, Color accent = Pink) {
        const int hit = itemButton(art, r, menuItem(screen, index, game.hasSave), index,
                                   selected, mouse, mouseActive, primary, accent);
        if (hit >= 0) hovered = hit;
    };

    if (screen == Screen::Title) {
        text(art, "PEQUEÑAS AVENTURAS, GRANDES SORPRESAS", 65, 71, 17, Gold);
        text(art, "SORPRESA", 66, 117, 74, {106, 64, 74, 255});
        text(art, "SORPRESA", 64, 111, 74, Cream);
        text(art, "VIVIS", 64, 182, 112, {111, 65, 82, 255});
        text(art, "VIVIS", 62, 175, 112, Pink);
        heart({382, 213}, 5, Pink);
        paw({442, 211}, 5, Blue);
        DrawRectangle(65, 307, 72, 3, Pink);
        DrawRectangle(143, 307, 122, 1, Fade(Cream, .4f));
        static const char* introduction[]{"Un corazón valiente.", "Un gato lleno de sorpresas.",
                                          "Un pequeño mundo que los necesita."};
        for (int line = 0; line < 3; ++line) {
            const float y = 332.0f + line * 32;
            text(art, introduction[line], 67, y + 2, 25, Ink, true);
            text(art, introduction[line], 65, y, 25, Cream, true);
        }
        text(art, "EXPLORA  /  COMBATE  /  VUELVE A CASA", 65, 448, 15, Green);
        pixelFrame({846, 139, 370, 484}, Fade(Pink, .8f), Fade(Ink, .95f));
        text(art, "TU PRÓXIMA PEQUEÑA AVENTURA", 875, 169, 15, Pink);
        text(art, game.hasSave ? "Tu camino continúa aquí." : "Todo empieza con un paso.",
             875, 196, 19, Cream, true);
        float y = 244;
        for (int index = 0; index < menuItemCount(screen, game.hasSave); ++index) {
            const float h = index == 0 ? 58.0f : 44.0f;
            button({873, y, 316, h}, index, index == 0);
            y += h + 10;
        }
        for (int row = 0; row < 4; ++row) {
            const float x = 65.0f + row * 180;
            pixelFrame({x, 494, 160, 137}, Fade(CharacterColors[row], .6f), Fade(Ink, .78f), 1);
            drawPortrait(art, row, {x + 37, 505, 86, 91});
            centered(art, ShortNames[row], x + 80, 604, 17, CharacterColors[row]);
        }
        text(art, "Una aventura hecha con cariño.", 65, 653, 17, Soft, true);
        navigationHint(art);
        return hovered;
    }

    if (screen == Screen::Settings) {
        pageHeading(art, "A tu manera", "Ajusta la aventura. Los cambios se aplican al instante.");
        pixelFrame({220, 159, 840, 469}, Line, Panel);
        static const char* explanations[]{"Efectos y melodías de la aventura", "Flechas o clic en < > para ajustar",
            "También puedes usar F11", "Reduce sacudidas y efectos de movimiento", "Más margen para disfrutar y explorar"};
        for (int i = 0; i < 5; ++i) {
            const Rectangle row{246, 183.0f + i * 72, 788, 61};
            const bool hover = mouseActive && inside(mouse, row);
            const bool active = hover || (!mouseActive && selected == i);
            pixelFrame(row, active ? Pink : Fade(Line, .7f), active ? Raised : Panel, active ? 2 : 1);
            text(art, menuItem(screen, i, game.hasSave), row.x + 20, row.y + 9, 22, Cream);
            text(art, explanations[i], row.x + 20, row.y + 36, 14, Soft, true);
            std::string value;
            if (i == 0) value = game.preferences.muted ? "DESACTIVADO" : "ACTIVADO";
            if (i == 1) {
                char volume[24];
                std::snprintf(volume, sizeof(volume), "%d%%", int(std::round(game.preferences.volume * 100)));
                value = volume;
            }
            if (i == 2) value = game.preferences.fullscreen ? "ACTIVADA" : "VENTANA";
            if (i == 3) value = game.preferences.reducedMotion ? "ACTIVADO" : "DESACTIVADO";
            if (i == 4) value = game.preferences.easy ? "ACTIVADO" : "DESACTIVADO";
            const Color valueColor = i == 1 ? Blue : (value == "DESACTIVADO" || value == "VENTANA" ? Soft : Green);
            if (hover) hovered = i;
            if (i == 1) {
                centered(art, value, 947, row.y + 20, 19, Blue);
                for (int side = 0; side < 2; ++side) {
                    const Rectangle arrow{side == 0 ? 879.0f : 978.0f, row.y + 13, 36, 35};
                    const bool arrowHover = mouseActive && inside(mouse, arrow);
                    pixelFrame(arrow, arrowHover ? Blue : Line, arrowHover ? Raised : Panel,
                               arrowHover ? 2 : 1);
                    centered(art, side == 0 ? "<" : ">", arrow.x + 18, arrow.y + 4, 25,
                             arrowHover ? Cream : Blue);
                    if (arrowHover) hovered = side == 0 ? 201 : 202;
                }
            } else {
                text(art, value, row.x + row.width - 24 - textWidth(art, value, 19), row.y + 20, 19, valueColor);
            }
        }
        button({450, 557, 380, 47}, 5, false, Blue);
        navigationHint(art, "Volumen: izquierda / derecha     Sonido rápido: M");
        return hovered;
    }

    if (screen == Screen::Controls) {
        pageHeading(art, "Unos pocos controles. Toda una aventura.", "Muévete, lanza magia y deja que Wisky te eche una pata.");
        pixelFrame({100, 166, 526, 382}, Fade(Blue, .7f), Panel);
        pixelFrame({652, 166, 526, 382}, Fade(Purple, .7f), Panel);
        text(art, "TECLADO", 129, 188, 23, Blue);
        text(art, "MANDO", 681, 188, 23, Purple);
        static const char* keys[]{"WASD", "Flechas", "Espacio", "E", "Shift / L", "Tab / I"};
        static const char* pads[]{"Stick izquierdo", "Stick derecho", "A / cruz", "X / cuadrado", "B / círculo", "Start"};
        static const char* actions[]{"Mover", "Apuntar y disparar", "Disparo automático", "Wisky / interactuar", "Esquivar", "Mapa / inventario"};
        static const char* padActions[]{"Mover", "Apuntar y disparar", "Disparo automático", "Wisky / interactuar", "Esquivar", "Pausar"};
        for (int i = 0; i < 6; ++i) {
            const float y = 232.0f + i * 48;
            keycap(art, keys[i], {129, y, 181, 36}, Blue);
            text(art, actions[i], 330, y + 8, 18, Cream, true);
            keycap(art, pads[i], {681, y, 181, 36}, Purple);
            text(art, padActions[i], 882, y + 8, 18, Cream, true);
        }
        centered(art, "Espacio busca al enemigo cercano. Las flechas eligen la dirección. Esc pausa.", 640, 568, 18, Green, true);
        button({450, 608, 380, 48}, 0, true, Blue);
        navigationHint(art);
        return hovered;
    }

    if (screen == Screen::Gallery) {
        const int row = std::clamp(galleryCharacter, 0, 3);
        const Color accent = CharacterColors[row];
        static const char* roles[]{"HUMANA  /  AMABLE  /  ENÉRGICA", "GATO  /  CURIOSO  /  ASTUTO",
            "HUMANO  /  INGENIOSO  /  EXPLORADOR", "GATA CLARA  /  MÍSTICA  /  SÚPER APACHONA"};
        static const char* powers[]{"Magia de corazones", "Veneno y nubes tóxicas", "Magia digital", "Magia lunar y un gran apetito"};
        static const char* stories[]{
            "Un corazón que ilumina el camino. Junto a Wisky, Vivi recorre cuatro pisos llenos de sorpresas y reúne mejoras para traer a Giovanny de vuelta.",
            "Curioso, astuto y un poco travieso. Su veneno debilita a los enemigos mientras Vivi abre camino. Las mejoras pueden volverlo aún más poderoso.",
            "Pelo negro lacio, barba e ideas que se convierten en magia digital. Giovanny aporta un ingenio muy suyo, y una razón especial para volver a casa.",
            "Clara, esponjosa y súper apachona. Guarda grandes misterios tras su magia lunar. Un churu puede abrir una puerta donde la fuerza no basta."};
        static const char* mottos[]{"La amabilidad también es una gran aventura.", "La curiosidad también es una forma de valentía.",
            "Cada buena idea abre un camino.", "Hay luz incluso en las sombras."};
        pageHeading(art, "Amigos extraordinarios", "Izquierda / derecha para conocer a los cuatro compañeros de aventura.");
        pixelFrame({116, 162, 374, 318}, Fade(accent, .8f), Panel);
        grid({122, 168, 362, 306}, 20);
        const int sequence[]{0, 1, 2, 1};
        const int pose = game.preferences.reducedMotion ? 0 : sequence[int(uiTime * 3) % 4];
        drawCharacter(art, row, pose, {301, 448}, 250);
        spark({160, 213}, Fade(accent, .65f), 2);
        spark({439, 398}, Fade(accent, .65f), 2);
        text(art, Names[row], 532, 180, 43, accent);
        text(art, roles[row], 534, 236, 15, Soft);
        text(art, "SU MAGIA", 534, 275, 14, accent);
        text(art, powers[row], 534, 297, 25, Cream);
        wrapped(art, stories[row], {534, 342, 612, 104}, 21, Cream, 1.4f, true);
        text(art, mottos[row], 534, 454, 16, accent, true);
        for (int i = 0; i < 4; ++i) {
            const float x = 116.0f + i * 269;
            const bool chosen = i == row;
            const Rectangle card{x, 497, 241, 98};
            const bool cardHover = mouseActive && inside(mouse, card);
            pixelFrame(card, chosen || cardHover ? CharacterColors[i] : Line,
                       chosen || cardHover ? Raised : Panel, chosen || cardHover ? 3 : 1);
            drawPortrait(art, i, {x + 13, 505, 68, 76});
            text(art, ShortNames[i], x + 94, 521, 19, chosen || cardHover ? CharacterColors[i] : Cream);
            text(art, cardHover ? "VER PERSONAJE" : (chosen ? "ESTÁS AQUÍ" : "SU HISTORIA"),
                 x + 94, 553, 12, chosen || cardHover ? CharacterColors[i] : Soft);
            if (cardHover) hovered = 100 + i;
        }
        button({262, 615, 236, 44}, 0, false, accent);
        button({522, 615, 236, 44}, 1, false, accent);
        button({782, 615, 236, 44}, 2, false, Blue);
        navigationHint(art);
        return hovered;
    }

    if (screen == Screen::Won) {
        centered(art, "¡Volvieron a casa!", 643, 57, 61, Fade(Pink, .32f));
        centered(art, "¡Volvieron a casa!", 640, 52, 61, Cream);
        centered(art, "Para Vivi, con todo el cariño del mundo.", 640, 128, 25, Pink, true);
        centered(art, "Cuatro pisos. Cuatro amigos. Una gran sorpresa.", 640, 165, 19, Soft, true);
        for (int row = 0; row < 4; ++row) {
            const float x = 187.0f + row * 238;
            pixelFrame({x - 65, 223, 192, 224}, Fade(CharacterColors[row], .55f), Panel, 1);
            drawPortrait(art, row, {x - 48, 240, 159, 162}, true);
            centered(art, ShortNames[row], x + 31, 412, 19, CharacterColors[row]);
        }
        centered(art, "Tu aventura: " + duration(game.runTime), 640, 463, 18, Gold);
        button({430, 506, 420, 56}, 0, true);
        button({430, 574, 420, 43}, 1);
        button({430, 629, 420, 43}, 2);
        navigationHint(art);
        return hovered;
    }

    if (screen == Screen::Lost) {
        pixelFrame({98, 184, 310, 361}, Fade(Pink, .65f), Panel);
        drawPortrait(art, 0, {139, 214, 228, 277});
        centered(art, "SEGUIMOS JUNTOS", 253, 510, 16, Pink);
        pixelFrame({440, 184, 742, 361}, Line, Panel);
        text(art, "Otra oportunidad", 486, 220, 46, Cream);
        wrapped(art, "Empiezas otra aventura, sin los objetos anteriores. Reúne nuevas mejoras y vuelve a intentarlo con Wisky.",
                {490, 287, 626, 81}, 23, Soft, 1.4f, true);
        button({490, 383, 642, 58}, 0, true);
        button({490, 456, 642, 48}, 1, false, Blue);
        centered(art, "Consejo: muévete con WASD, dispara con flechas y mejora el veneno de Wisky.", 640, 590, 18, Green, true);
        navigationHint(art);
        return hovered;
    }

    if (screen == Screen::Paused) {
        pixelFrame({350, 113, 580, 506}, Fade(Pink, .75f), Panel);
        centered(art, "Un respiro", 640, 151, 52, Cream);
        centered(art, "La aventura te espera.", 640, 217, 23, Soft, true);
        centered(art, std::string(zoneName(game.stage)) + "   /   " + duration(game.runTime), 640, 257, 17, Green, true);
        float y = 313;
        for (int i = 0; i < 4; ++i) {
            const float height = i == 0 ? 58.0f : 48.0f;
            button({403, y, 474, height}, i, i == 0);
            y += height + 13;
        }
        navigationHint(art);
    }
    return hovered;
}

void drawHud(const Game& game, const Artwork& art) {
    DrawRectangle(0, 0, Width, 145, Ink);
    grid({0, 0, float(Width), 143}, 24);
    DrawRectangle(28, 143, Width - 56, 2, Fade(Pink, .6f));
    text(art, "PISO " + std::to_string(game.stage + 1) + " / " + std::to_string(FloorCount), 28, 14, 14, Pink);
    for (int i = 0; i < FloorCount; ++i) {
        const Color color = i == game.stage ? Pink : (i < game.stage ? Green : Line);
        DrawRectangle(145 + i * 16, 18, 10, 7, color);
    }
    text(art, "SALAS " + std::to_string(game.clearedRooms()) + " / " + std::to_string(game.rooms.size()), 245, 14, 13, Soft);
    const float zoneSize = textWidth(art, zoneName(game.stage), 29) > 382 ? 25.0f : 29.0f;
    text(art, zoneName(game.stage), 28, 38, zoneSize, Cream);
    const Room* room = activeRoom(game);
    std::string roomStatus = room ? roomName(room->kind) : "Una aventura nueva";
    if (!game.enemies.empty()) roomStatus += "  /  " + std::to_string(game.enemies.size()) + " enemigos";
    else if (room) roomStatus += "  /  puertas abiertas";
    for (const auto& enemy : game.enemies) if (enemy.boss) {
        roomStatus = std::string("GUARDIÁN: ") + enemyName(enemy.kind);
        break;
    }
    const float roomSize = textWidth(art, roomStatus, 14, true) > 382 ? 12.0f : 14.0f;
    text(art, roomStatus, 29, 77, roomSize, room ? roomColor(room->kind) : Soft, true);
    DrawRectangle(420, 15, 1, 73, Line);

    text(art, "VIDA " + std::to_string(std::max(0, game.hp)) + "/" + std::to_string(game.maxHp), 441, 14, 13, Pink);
    hearts(game, {442, 43});
    if (game.roomShield > 0 || game.stats.shields > 0)
        text(art, "ESCUDO " + std::to_string(game.roomShield), 443, 77, 12, game.roomShield > 0 ? Purple : Soft);
    text(art, "MONEDAS", 650, 14, 12, Gold);
    coin({651, 44}, 2);
    text(art, std::to_string(game.coins), 674, 37, 26, Cream);
    text(art, "VIDAS EXTRA", 746, 14, 12, Pink);
    heart({748, 44}, 2, Pink);
    text(art, std::to_string(game.extraLives), 773, 37, 26, Cream);
    DrawRectangle(817, 15, 1, 73, Line);

    const float help = game.wiskyCorrupted ? 0 : std::clamp(1.0f - game.helpCooldown / std::max(.1f, game.stats.helpCooldown), 0.0f, 1.0f);
    const float dash = std::clamp(1.0f - game.dashCooldown / 1.6f, 0.0f, 1.0f);
    const Color wiskyColor = game.wiskyCorrupted ? Purple : Green;
    const std::string helpState = game.wiskyCorrupted ? "ALTER EGO"
        : (game.helpCooldown <= 0 ? "LISTO" : number(std::ceil(game.helpCooldown), 0) + " s");
    text(art, game.wiskyCorrupted ? "WISKY" : "WISKY / E", 837, 15, 12, wiskyColor);
    text(art, helpState, 837, 38, 17, wiskyColor);
    DrawRectangle(838, 66, 100, 3, Line);
    DrawRectangle(838, 66, int(100 * help), 3, wiskyColor);
    text(art, "ESQUIVA / SHIFT", 956, 15, 12, Blue);
    text(art, game.dashCooldown <= 0 ? "LISTA" : number(std::ceil(game.dashCooldown), 0) + " s", 956, 38, 17, Blue);
    DrawRectangle(957, 66, 112, 3, Line);
    DrawRectangle(957, 66, int(112 * dash), 3, Blue);
    text(art, duration(game.runTime), 1005, 80, 13, Soft);
    pixelFrame({1090, 8, 160, 77}, Line, Ink, 1);
    roomMap(game, art, {1097, 13, 146, 66}, false);
    centered(art, "TAB: MAPA", 1170, 88, 12, Blue);

    pixelFrame({28, 106, 1220, 29}, Fade(Green, .42f), Panel, 1);
    spark({43, 120}, Green, 1);
    const std::string goal = objective(game);
    const float goalSize = textWidth(art, goal, 19, true) > 1160 ? 16.0f : 19.0f;
    text(art, goal, 59, 111, goalSize, Green, true);
    for (const auto& enemy : game.enemies) if (enemy.boss && enemy.hp > 0 && enemy.maxHp > 0) {
        pixelFrame({430, 149, 420, 12}, Fade(Purple, .8f), Ink, 1);
        DrawRectangle(435, 153, int(410 * std::clamp(enemy.hp / enemy.maxHp, 0.0f, 1.0f)), 4, Purple);
        break;
    }
    if (game.noticeTime > 0 && !game.notice.empty()) {
        const float w = std::min(1100.0f, std::max(660.0f, textWidth(art, game.notice, 19, true) + 56));
        pixelFrame({(Width - w) * .5f, 627, w, 35}, Fade(Gold, .75f), Ink, 1);
        const float size = textWidth(art, game.notice, 19, true) <= w - 40 ? 19.0f : 16.0f;
        centered(art, game.notice, Width * .5f, 634, size, Cream, true);
    }
    DrawRectangle(0, 680, Width, 40, Ink);
    DrawRectangle(28, 680, Width - 56, 1, Line);
    text(art, "WASD mover   FLECHAS disparar   ESPACIO auto   E acción   SHIFT esquiva", 28, 693, 13, Cream, true);
    text(art, "TAB mapa   I objetos", 853, 693, 14, Blue, true);
    text(art, "ESC pausa", 1138, 693, 14, Soft, true);
}

int drawReward(const Game& game, const Artwork& art, int selected, Vector2 mouse, bool mouseActive) {
    artBackground(game, art, game.time, false);
    pageHeading(art, game.rewardIsShop ? "La tienda de pequeñas sorpresas" : "Una sorpresa para tu aventura",
        game.rewardIsShop ? "Intercambia tus monedas por una mejora o vuelve a explorar."
                          : "Elige una mejora. Te acompañará durante esta aventura.");
    coin({1052, 151}, 2);
    text(art, std::to_string(game.coins) + " monedas", 1077, 149, 19, Gold);
    int hovered = -1;
    for (int i = 0; i < 3; ++i) {
        const ItemId id = game.rewardChoices[i];
        const auto& info = itemInfo(id);
        const int cost = std::max(0, game.rewardCosts[i]);
        const bool affordable = cost <= game.coins;
        const Rectangle card{60.0f + i * 390, 196, 380, 378};
        const bool hover = mouseActive && inside(mouse, card);
        const bool active = hover || (!mouseActive && selected == i);
        const Color accent = affordable ? info.color : Soft;
        DrawRectangle(int(card.x + 3), int(card.y + card.height), int(card.width - 6), 5, Fade(BLACK, .3f));
        pixelFrame(card, active ? accent : Line, active ? Raised : Panel, active ? 3 : 1);
        grid({card.x + 8, card.y + 8, card.width - 16, 107}, 16);
        pixelFrame({card.x + 139, 216, 102, 100}, Fade(accent, .4f), Ink, 1);
        itemEmblem(id, {card.x + 149, 226, 82, 80}, accent);
        centered(art, info.wisky ? "PARA WISKY / VENENO" : "PARA TU AVENTURA", card.x + 190, 332, 13,
                 info.wisky ? Green : accent);
        const float nameSize = textWidth(art, info.name, 25) > 328 ? 23.0f : 25.0f;
        centered(art, info.name, card.x + 190, 359, nameSize, Cream);
        wrapped(art, info.description, {card.x + 26, 407, 328, 89}, 19, Soft, 1.4f, true);
        DrawRectangle(int(card.x + 26), 510, 328, 1, Line);
        const std::string price = cost == 0 ? "ELEGIR MEJORA" : std::to_string(cost) + " MONEDAS";
        centered(art, price, card.x + 190, 529, 20, affordable ? (cost ? Gold : accent) : Soft);
        if (!affordable) centered(art, "Te faltan " + std::to_string(cost - game.coins) + " monedas", card.x + 190, 555, 12, Pink, true);
        if (active) {
            DrawRectangle(int(card.x + 16), 361, 3, 10, accent);
            DrawRectangle(int(card.x + 19), 364, 3, 4, accent);
        }
        if (hover) hovered = i;
    }
    const int close = itemButton(art, {450, 615, 380, 48}, "Volver a la habitación", 3, selected,
                                  mouse, mouseActive, false, Blue);
    if (close >= 0) hovered = close;
    navigationHint(art);
    return hovered;
}

int drawMap(const Game& game, const Artwork& art, Vector2 mouse, bool mouseActive) {
    artBackground(game, art, game.time, false);
    pageHeading(art, "El camino que llevas", "La aventura está en pausa. Explora una sala para descubrir sus vecinas.");
    pixelFrame({80, 163, 792, 445}, Line, Panel);
    text(art, "PISO " + std::to_string(game.stage + 1) + " / " + std::to_string(FloorCount), 106, 182, 17, Pink);
    const std::string progress = std::to_string(game.clearedRooms()) + " de " + std::to_string(game.rooms.size()) + " salas limpias";
    text(art, progress, 848 - textWidth(art, progress, 16, true), 183, 16, Soft, true);
    roomMap(game, art, {104, 226, 742, 338}, true);
    centered(art, zoneName(game.stage), 476, 577, 19, Cream);
    pixelFrame({895, 163, 305, 445}, Line, Panel);
    text(art, "HABITACIONES", 920, 183, 19, Cream);
    const RoomKind kinds[]{RoomKind::Start, RoomKind::Combat, RoomKind::Treasure, RoomKind::Shop,
                           RoomKind::Sanctuary, RoomKind::Challenge, RoomKind::Boss};
    for (int i = 0; i < 7; ++i) {
        const float y = 226.0f + i * 41;
        roomIcon(kinds[i], {932, y + 8}, 2, roomColor(kinds[i]));
        text(art, roomName(kinds[i]), 957, y - 2, 21, Cream, true);
    }
    DrawRectangle(918, 520, 258, 1, Line);
    DrawRectangleLinesEx({922, 537, 15, 15}, 2, Pink);
    text(art, "Tu habitación", 950, 536, 15, Pink, true);
    DrawRectangle(925, 563, 10, 10, Green);
    text(art, "Sala limpia", 950, 560, 15, Green, true);
    text(art, "?", 924, 581, 18, Soft);
    text(art, "Por explorar", 950, 583, 15, Soft, true);
    const int hover = closeOverlay(art, mouse, mouseActive, "Volver a la aventura");
    centered(art, "TAB o ESC: cerrar mapa", 640, 691, 16, Soft, true);
    return hover;
}

int drawInventory(const Game& game, const Artwork& art, Vector2 mouse, bool mouseActive) {
    artBackground(game, art, game.time, false);
    pageHeading(art, "Lo que te acompaña", "Mejoras y sorpresas de esta aventura. Al empezar otra, vuelves a reunirlas.");
    pixelFrame({80, 166, 292, 441}, Line, Panel);
    text(art, "ESTA AVENTURA", 104, 188, 20, Cream);
    const std::array<std::string, 7> labels{"Daño de corazones", "Corazones por tiro", "Velocidad", "Daño del veneno",
                                           "Duración del veneno", "Recarga de Wisky", "Escudos por sala"};
    const std::array<std::string, 7> values{number(game.stats.damage), std::to_string(game.stats.multishot),
        "+" + number((game.stats.speed / 270.0f - 1) * 100, 0) + "%", number(game.stats.poisonDps) + "/s",
        number(game.stats.poisonDuration) + " s", number(game.stats.helpCooldown) + " s", std::to_string(game.stats.shields)};
    for (int i = 0; i < 7; ++i) {
        const float y = 237.0f + i * 42;
        text(art, labels[i], 104, y, 15, i >= 3 && i <= 5 ? Green : Soft, true);
        text(art, values[i], 348 - textWidth(art, values[i], 20), y - 2, 20, Cream);
    }
    DrawRectangle(104, 541, 244, 1, Line);
    coin({106, 561}, 2);
    text(art, std::to_string(game.coins), 131, 556, 23, Gold);
    heart({240, 561}, 2, Pink);
    text(art, std::to_string(game.extraLives), 267, 556, 23, Pink);
    std::vector<int> obtained;
    for (int i = 0; i < ItemCount; ++i) if (game.inventory[i] > 0) obtained.push_back(i);
    if (obtained.empty()) {
        pixelFrame({402, 166, 798, 441}, Line, Panel);
        drawPortrait(art, 1, {678, 213, 236, 211});
        centered(art, "Aquí caben muchas sorpresas.", 800, 449, 29, Cream);
        centered(art, "Limpia salas y busca tesoros para reunir mejoras.", 800, 498, 19, Soft, true);
        centered(art, "Hay objetos para Vivi, para Wisky y para los dos.", 800, 529, 18, Green, true);
    } else {
        for (int slot = 0; slot < int(obtained.size()); ++slot) {
            const int index = obtained[slot];
            const ItemId id = ItemId(index);
            const auto& info = itemInfo(id);
            const Rectangle card{402.0f + (slot % 2) * 409, 173.0f + (slot / 2) * 62, 389, 57};
            pixelFrame(card, Fade(info.color, .5f), Panel, 1);
            itemEmblem(id, {card.x + 7, card.y + 8, 40, 40}, info.color);
            const float nameSize = textWidth(art, info.name, 18) > 273 ? 16.0f : 18.0f;
            text(art, info.name, card.x + 57, card.y + 5, nameSize, info.wisky ? Green : Cream);
            text(art, "x" + std::to_string(game.inventory[index]), card.x + 352, card.y + 8, 13, info.color);
            wrapped(art, info.description, {card.x + 57, card.y + 28, 321, 26}, 12, Soft, 1.15f, true);
        }
    }
    const int hover = closeOverlay(art, mouse, mouseActive, "Volver a la aventura");
    centered(art, "I o ESC: cerrar objetos    /    La aventura está en pausa", 640, 691, 16, Soft, true);
    return hover;
}
}
