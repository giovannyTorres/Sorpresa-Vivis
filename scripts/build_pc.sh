#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
game_engine="${GODOT_BIN:-godot}"
mkdir -p build/windows build/linux dist
"$game_engine" --headless --path godot_game --editor --import --quit
"$game_engine" --headless --path godot_game --script res://tests/playthrough.gd
"$game_engine" --headless --path godot_game --export-release Windows
"$game_engine" --headless --path godot_game --export-release Linux
if [[ "$(uname -s)" == Linux ]]; then
  build/linux/Sorpresa-Vivis.x86_64 --headless --quit-after 5
  "$game_engine" --headless --main-pack build/linux/Sorpresa-Vivis.x86_64 --script "$PWD/godot_game/tests/playthrough.gd"
fi
python3 - <<'PY'
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

instructions = '''SORPRESA VIVI’S — JUEGO 2D PARA PC

Windows: abre Sorpresa-Vivis.exe.
Linux: ejecuta ./Sorpresa-Vivis.x86_64.
No necesitas instalar Godot. Funciona sin Internet.

WASD / flechas: mover.
Espacio (mantener): disparar al enemigo más cercano.
E / Enter: ayuda de Wiky (recarga 5 s); rescatar a Giovanny.
Esc: pausa. R: reintentar tras derrota. M: sonido. F11: pantalla completa.

Recoge cada cascabel y elimina las pelusas para abrir la salida derecha.
En la torre, el cascabel rompe el escudo de Maliketh.
Véncela, acércate a Giovanny y pulsa E.
Los corazones curan; avanzar de lugar restaura la vida.
Si pierdes, puedes repetir ese lugar. No hay guardado entre sesiones.
'''
for platform, binary, label in [
    ('windows', 'Sorpresa-Vivis.exe', 'Windows'),
    ('linux', 'Sorpresa-Vivis.x86_64', 'Linux'),
]:
    archive = Path('dist') / f'Sorpresa-Vivis-{label}.zip'
    with ZipFile(archive, 'w', ZIP_DEFLATED) as package:
        package.write(Path('build') / platform / binary, binary)
        package.writestr('LEEME.txt', instructions)
    print(f'{archive} ({archive.stat().st_size / 1024 / 1024:.1f} MiB)')
PY
