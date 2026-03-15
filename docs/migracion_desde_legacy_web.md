# Migración desde legacy web a Godot 4

## Mapeo directo del prototipo
- **Overworld**: movimiento de Vivis/Wiky, límites de mapa, fondo, torre de Maliketh y objetivo de rescate (`legacy_web/game.js` funciones `update`, `drawBackground`, `drawCharacters`).
- **Combate**: generación de ataques, daño aleatorio, contraataque de Maliketh y condición de victoria/derrota (`spawnAttack`, `enemyAttack`, colisión en `update`).
- **Render/UI**: HUD de HP, log narrativo y pantalla final (`legacy_web/index.html`, `legacy_web/styles.css`, DOM + canvas en `game.js`).

## Qué sí se rescata
1. Estadísticas base (HP iniciales y rangos de daño).
2. Identidad de personajes y ataques.
3. Ritmo de loop: exploración breve + combate con jefe.

## Qué se reescribe desde cero en Godot
1. Render pixel-art en canvas y sprites ASCII (`legacy_web/sprites.js`) → migrar a `Sprite2D/AnimatedSprite2D` y assets reales.
2. Input hardcodeado por teclas → Input Map de Godot.
3. Estado global mezclado en un archivo → `GameState`, `EventBus`, `SceneRouter`, sistemas por módulo.
4. UI en DOM/CSS → Control nodes (`dialogue_box`, `pause_menu`, HUD).

## Ruta técnica inmediata
1. Implementar cargadores de `data/actors` y `data/attacks` en runtime.
2. Conectar `EventBus` para transiciones a `combat_scene` y ejecución de diálogo.
3. Sustituir placeholders visuales con tileset/sprites Godot y colisiones reales.
