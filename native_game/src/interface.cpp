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
constexpr std::array<Color, 4> CharacterColors{Pink, Blue, Green, Purple};
constexpr const char* Names[]{"VIVIS", "WIKY WIKERMAN", "GIOVANNY", "MALIKETH"};
constexpr const char* ShortNames[]{"VIVIS", "WIKY", "GIOVANNY", "MALIKETH"};

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
    if (index < 0 || index >= menuItemCount(screen, hasSave)) return "";
    switch (screen) {
        case Screen::Title: return hasSave ? saved[index] : title[index];
        case Screen::Paused: return paused[index];
        case Screen::Lost: return lost[index];
        case Screen::Won: return won[index];
        case Screen::Settings: return settings[index];
        case Screen::Controls: return controls[index];
        case Screen::Gallery: return gallery[index];
        default: return "";
    }
}

int drawMenu(const Game& game, const Artwork& art, Screen screen, int selected,
             Vector2 mouse, bool mouseActive, float uiTime, int galleryCharacter) {
    if (screen == Screen::Playing) return -1;
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
        static const char* introduction[]{"Un corazón valiente.", "Un gato con ideas.",
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
        pageHeading(art, "Unos pocos controles. Toda una aventura.", "Muévete, lanza magia y deja que Wiky te eche una pata.");
        pixelFrame({100, 166, 526, 382}, Fade(Blue, .7f), Panel);
        pixelFrame({652, 166, 526, 382}, Fade(Purple, .7f), Panel);
        text(art, "TECLADO", 129, 188, 23, Blue);
        text(art, "MANDO", 681, 188, 23, Purple);
        static const char* keys[]{"WASD / flechas", "Espacio", "E / Enter", "Shift / L", "Esc"};
        static const char* pads[]{"Stick / cruceta", "A / cruz", "X / cuadrado", "B / círculo", "Start"};
        static const char* actions[]{"Mover", "Lanzar magia", "Ayuda de Wiky", "Esquivar", "Pausar"};
        for (int i = 0; i < 5; ++i) {
            const float y = 235.0f + i * 58;
            keycap(art, keys[i], {129, y, 181, 36}, Blue);
            text(art, actions[i], 330, y + 7, 21, Cream, true);
            keycap(art, pads[i], {681, y, 181, 36}, Purple);
            text(art, actions[i], 882, y + 7, 21, Cream, true);
        }
        centered(art, "La magia busca al enemigo más cercano. Concéntrate en moverte.", 640, 568, 20, Green, true);
        button({450, 608, 380, 48}, 0, true, Blue);
        navigationHint(art);
        return hovered;
    }

    if (screen == Screen::Gallery) {
        const int row = std::clamp(galleryCharacter, 0, 3);
        const Color accent = CharacterColors[row];
        static const char* roles[]{"HUMANA  /  AMABLE  /  ENÉRGICA", "GATO  /  CURIOSO  /  ASTUTO",
            "HUMANO  /  EXPLORADOR  /  PRÁCTICO", "GATA  /  MÍSTICA  /  IMPONENTE Y TIERNA"};
        static const char* powers[]{"Magia de corazones", "Magia digital", "Poder natural", "Magia sombría y lunar"};
        static const char* stories[]{
            "Un corazón que ilumina el camino. Junto a Wiky, Vivi convierte cada pequeño gesto de cariño en fuerza para traer a Giovanny de vuelta.",
            "Un paso adelante, siempre otra idea. Este curioso compañero transforma su ingenio en un pulso de magia azul cuando más lo necesitas.",
            "La naturaleza también avanza contigo. Un explorador de bolsillos llenos de hojas e ideas, y una razón muy especial para volver a casa.",
            "Sombras suaves, grandes misterios. Bajo su imponente magia lunar se esconde una gata apachona con una historia que también merece cariño."};
        static const char* mottos[]{"La amabilidad también es una gran aventura.", "La curiosidad también es una forma de valentía.",
            "Un mundo mejor también se cultiva.", "Hay luz incluso en las sombras."};
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
        centered(art, "Tres lugares. Cuatro amigos. Una gran sorpresa.", 640, 165, 19, Soft, true);
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
        wrapped(art, "La aventura todavía te espera. Reintenta esta zona con la vida completa y vuelve al camino junto a Wiky.",
                {490, 287, 626, 81}, 23, Soft, 1.4f, true);
        button({490, 383, 642, 58}, 0, true);
        button({490, 456, 642, 48}, 1, false, Blue);
        centered(art, "Consejo: muévete al lanzar magia y usa la ayuda de Wiky.", 640, 590, 20, Green, true);
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
    text(art, "SORPRESA VIVIS", 32, 17, 16, Pink);
    if (game.preferences.easy) text(art, "MODO TRANQUILO", 224, 20, 12, Green);
    text(art, zoneName(game.stage), 32, 44, 34, Cream);
    text(art, "CAPÍTULOS", 549, 18, 13, Soft);
    for (int stage = 0; stage < 3; ++stage) {
        const bool current = stage == game.stage;
        const bool done = stage < game.stage;
        const Rectangle r{549.0f + stage * 73, 44, 61, 28};
        pixelFrame(r, current ? Green : Line, current ? Raised : Ink, current ? 2 : 1);
        centered(art, done ? "LISTO" : std::to_string(stage + 1), r.x + r.width * .5f, r.y + 5,
                 done ? 12.0f : 19.0f, current || done ? Green : Soft);
    }
    text(art, duration(game.runTime), 786, 50, 21, Soft);
    DrawRectangle(877, 17, 1, 66, Line);
    text(art, "VIDA", 899, 19, 13, Pink);
    for (int i = 0; i < MaxHp; ++i) heart({974.0f + i * 40, 20}, 3, i < game.hp ? Pink : Color{57, 65, 76, 255});
    const float help = std::clamp(1.0f - game.helpCooldown / 5.0f, 0.0f, 1.0f);
    const float dash = std::clamp(1.0f - game.dashCooldown / 2.0f, 0.0f, 1.0f);
    keycap(art, "E", {898, 56, 28, 26}, Blue);
    const std::string helpLabel = game.helpCooldown <= 0 ? "Wiky listo"
        : "Wiky " + std::to_string(int(std::ceil(game.helpCooldown))) + " s";
    text(art, helpLabel, 937, 58, 16, Blue);
    DrawRectangle(938, 81, 121, 3, Line);
    DrawRectangle(938, 81, int(121 * help), 3, Blue);
    keycap(art, "SHIFT", {1082, 56, 59, 26}, Green);
    const std::string dashLabel = game.dashCooldown <= 0 ? "Esquiva"
        : "Esquiva " + std::to_string(int(std::ceil(game.dashCooldown))) + " s";
    text(art, dashLabel, 1150, 58, 16, Green);
    DrawRectangle(1151, 81, 95, 3, Line);
    DrawRectangle(1151, 81, int(95 * dash), 3, Green);
    pixelFrame({28, 102, 1220, 31}, Fade(Green, .42f), Panel, 1);
    spark({44, 117}, Green, 1);
    text(art, objective(game), 61, 107, 20, Green, true);
    const std::string status = game.stage == 2
        ? (game.bossDefeated ? "GIOVANNY TE ESPERA" : (game.collected ? "ESCUDO ROTO" : "ESCUDO LUNAR"))
        : std::to_string(game.enemies.size()) + (game.enemies.size() == 1 ? " PELUSA" : " PELUSAS");
    text(art, status, 1225 - textWidth(art, status, 14), 111, 14, game.stage == 2 ? Purple : Soft);
    if (game.stage == 2 && !game.bossDefeated) {
        int hp = 0;
        for (const auto& enemy : game.enemies) if (enemy.boss) hp = enemy.hp;
        if (hp > 0) {
            pixelFrame({449, 149, 382, 12}, Fade(Purple, .8f), Ink, 1);
            DrawRectangle(454, 153, int(372 * std::clamp(hp / 38.0f, 0.0f, 1.0f)), 4, Purple);
        }
    }
    if (game.noticeTime > 0 && !game.notice.empty()) {
        const float w = std::min(1080.0f, std::max(660.0f, textWidth(art, game.notice, 20, true) + 56));
        pixelFrame({(Width - w) * .5f, 627, w, 35}, Fade(Gold, .75f), Ink, 1);
        const float size = textWidth(art, game.notice, 20, true) <= w - 40 ? 20.0f : 17.0f;
        centered(art, game.notice, Width * .5f, 634, size, Cream, true);
    }
    DrawRectangle(0, 680, Width, 40, Ink);
    DrawRectangle(28, 680, Width - 56, 1, Line);
    text(art, "WASD mover   ESPACIO atacar   E Wiky   SHIFT esquiva", 32, 692, 16, Cream, true);
    text(art, "MANDO   A atacar   X Wiky   B esquiva", 666, 693, 14, Blue, true);
    text(art, "ESC pausa   M sonido", 1067, 693, 14, Soft, true);
}
}
