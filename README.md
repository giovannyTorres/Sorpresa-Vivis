# Sorpresa Vivis

**Aventura 2D para PC — versión nativa 0.3.0 en C++17 y raylib.**

Vivi y Wiky atraviesan la Pradera de Bigotes y el Sendero del Umbral para derrotar a Maliketh y rescatar a Giovanny. Tres escenarios, magia de corazones, una gata lunar y un final completo.

## Jugar

**Windows 10/11 de 64 bits:** descomprime `Sorpresa-Vivis-Native-Windows.zip` y abre **Sorpresa-Vivis.exe**. El arte, fuentes y sonido están integrados en ese ejecutable. No necesita instalar Godot, Python, raylib ni descargar archivos al jugar.

**Linux de 64 bits:** descomprime `Sorpresa-Vivis-Native-Linux.zip`, da permiso con `chmod +x Sorpresa-Vivis` si hace falta y abre `./Sorpresa-Vivis`. Requiere un escritorio X11, OpenGL 3.3 y glibc 2.38 o posterior.

El juego funciona sin conexión. Los menús aceptan ratón, teclado y mando. Puedes cambiar el volumen, pantalla completa, movimiento reducido y modo tranquilo dentro de **Ajustes**.

| Acción | Teclado | Mando |
| --- | --- | --- |
| Mover | WASD / flechas | Stick izquierdo / cruceta |
| Magia de corazones | Espacio, mantener | A / botón inferior, mantener |
| Magia de Wiky / rescatar | E o Enter | X / botón izquierdo |
| Esquivar | Mayús o L | B / botón derecho |
| Pausa | Esc | Start |
| Reintentar tras derrota | R o botón del menú | A |
| Pantalla completa | F11 | Ajustes |
| Activar/desactivar sonido | M | Ajustes |

La magia apunta al enemigo visible más cercano. Recoge el cascabel y despeja cada lugar para abrir la salida de la derecha. En la torre, el cascabel rompe el escudo de Maliketh. Cuando se rinda, acércate a Giovanny y pulsa E. La esquiva protege durante un instante y recarga en 2 segundos; Wiky alcanza a los enemigos cercanos y recarga en 5. Los corazones recuperan vida.

## Tu progreso

La partida guarda automáticamente **el inicio de cada zona**. Al volver, **Continuar** te deja en esa zona con la vida restaurada; no conserva los enemigos eliminados dentro de ella. Si pierdes, puedes repetir la misma zona. Los ajustes también se guardan. Al completar el rescate se cierra esa partida y puedes comenzar otra.

En Windows el perfil está en `%LOCALAPPDATA%/SorpresaVivis/profile.txt`. En Linux está en `$XDG_DATA_HOME/sorpresa-vivis/` o `~/.local/share/sorpresa-vivis/`. Se conserva una copia del perfil anterior para recuperar un guardado dañado.

## Personajes y dirección visual

Los diseños siguen la referencia aportada: Vivi con lentes, cabello castaño y sudadera rosa; Wiky como gato blanco y negro de ojos amarillos; Giovanny con lentes, barba, mochila y ropa verde; Maliketh como gata siamesa de ojos azules, pañuelo violeta y dije lunar. La galería permite ver sus animaciones, personalidad y magia.

Hay poses de reposo, movimiento, magia, daño y victoria, retratos, una nueva ilustración de portada e icono propio. Los escenarios incorporan vegetación, agua, ruinas, ventanas lunares, antorchas y profundidad. El combate añade destellos, estelas, aviso del ataque de Maliketh y feedback de daño; movimiento reducido atenúa esos efectos.

![Portada de la versión nativa](docs/previews/native-menu.png)

![Magia y combate en la versión nativa](docs/previews/native-combate.png)

## Compilar y verificar

La versión principal está en **`native_game/`**. El proyecto Godot y el prototipo web se conservan como referencia; el ejecutable nativo no depende de ellos. Sus atlas y la fuente se comparten desde `godot_game/playable/assets/` durante la compilación.

En Linux x86-64, con Python 3.9+, g++, `dpkg-deb` y las bibliotecas de escritorio X11/OpenGL:

```bash
python3 scripts/build_native.py --bootstrap --platform all --test
```

La primera compilación descarga raylib 5.5 y, si falta, un compilador MinGW portable para Windows. Las descargas están fijadas por versión y SHA256. No instala paquetes del sistema ni requiere permisos de administrador. Para las siguientes compilaciones sin red:

```bash
python3 scripts/build_native.py --platform all --test
```

Los binarios quedan en `build/native/{windows,linux}/` y los ZIP con instrucciones y licencias en `dist/`. Puedes compilar solo una plataforma con `--platform windows` o `--platform linux`. La dependencia portable de MinGW necesita un host Linux compatible con Debian 13; consulta las variables de override y licencias en [THIRD_PARTY.md](native_game/THIRD_PARTY.md).

Las **46 pruebas** comprueban movimiento, colisiones, esquiva, combate, pausa, derrota, curación, checkpoints, escudo, rescate, un recorrido completo y recuperación del perfil. Se revisaron las diez pantallas de la aplicación en Linux y se ejecutó el binario Windows con Wine 10, incluyendo captura de esas pantallas. La entrada de mando está implementada; no se probó con un dispositivo físico en este entorno. Para probar el arranque gráfico en un escritorio:

```bash
build/native/linux/Sorpresa-Vivis --smoke-test
```

Para capturar las diez pantallas principales y cerrar automáticamente:

```bash
build/native/linux/Sorpresa-Vivis --capture /tmp/vivis-preview
```

Pixelify Sans se distribuye con su licencia OFL. Raylib y su licencia acompañan cada ZIP. El mapa de archivos de la implementación es: `game.cpp` (reglas), `main.cpp` (ventana, entrada y sonido), `profile.cpp` (guardado), `characters.cpp` (sprites y magia), `world.cpp` (escenarios) e `interface.cpp` (menús y HUD).
