# Third-party components

The native desktop game is written in C++17. Its executable embeds the game's
artwork and fonts so that no game engine or asset-folder installation is required.

## raylib 5.5

- Source: https://github.com/raysan5/raylib/tree/5.5
- License: zlib/libpng; the release license is included in each package.
- The build statically links the official Linux x86-64 or Windows x86-64 MinGW
  release, including raylib's bundled modules. Consult the release's source for
  its external-library notices.
- The build helper pins release archives and verifies their SHA256 digests.

## Pixelify Sans

- Font file: `godot_game/playable/assets/pixel_font.ttf`.
- License: SIL Open Font License 1.1. The complete project-provided `OFL.txt` is
  included in each desktop package as `licenses/LICENSE-Pixelify-Sans.txt`.

## Compiler and runtime

Linux builds use the host GNU C++ compiler. Windows cross-builds can use a system
MinGW-w64 compiler or the optional, local Debian trixie toolchain downloaded by
`scripts/build_native.py --bootstrap`. Its pinned packages are GCC 14.2,
MinGW-w64 12.0 and binutils 2.44. Compiler packages are extracted into the tool
cache and are not distributed with the game or installed into the host system.

Both targets link libgcc and libstdc++ statically. Those libraries are distributed
under the GNU GPL with the GCC Runtime Library Exception, which permits this
use. Windows system DLLs and Linux system graphics libraries are provided by the
operating system and are not bundled.

- GCC Runtime Library Exception: https://www.gnu.org/licenses/gcc-exception-3.1.html
- MinGW-w64: https://www.mingw-w64.org/license/

The packaged executable includes no Godot runtime. The earlier Godot project and
its files remain in the repository as the source of the existing artwork.
