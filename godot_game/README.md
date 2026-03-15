# godot_game

Base técnica inicial para el nuevo juego retro 2D.

## Entry point esperado
- Escena principal: `res://project/main.tscn`
- Bootstrap de arranque: `res://project/bootstrap/game_bootstrap.gd`

## Módulos
- `core/`: estado global y bus de eventos.
- `project/`: bootstrap y router de escenas.
- `overworld/`: mapa base, controlador del jugador y follower de compañero.
- `combat/`: gestores de turnos, resolución de acciones y AI enemiga.
- `narrative/`: runner de diálogos y secuenciador de eventos.
- `ui/`: HUD y escenas de interfaz reutilizables.
- `persistence/`: repositorio de guardado/carga.
- `data/`: JSON inicial de actores, ataques, diálogos y flujo de eventos.

## Estado actual
- El proyecto ya incluye `project.godot`, autoloads y escenas base.

- Arranque temporal: `GameBootstrap` enruta directo a `overworld/scenes/map_pradera_bigotes.tscn` para asegurar visibilidad mientras la intro/cutscene se implementa.
- Falta integrar arte final, escenas jugables completas y wiring de señales entre todos los sistemas.
- La estructura está preparada para escalar por feature sin mezclar lógica en un solo script.

## Para abrirlo en Godot 4
1. Abrir la carpeta `godot_game/` desde Godot 4.2+.
2. Verificar que los autoloads de `project.godot` se carguen correctamente.
3. Ejecutar `project/main.tscn`.
