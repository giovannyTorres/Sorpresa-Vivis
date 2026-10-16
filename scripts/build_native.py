#!/usr/bin/env python3
"""Build the C++17 desktop game, embedding every required asset in one executable.

Python 3.9+ and a Linux x86-64 host are sufficient for the helper. Linux builds
also require g++, the usual desktop X11/OpenGL runtime libraries and their headers.
The optional MinGW bootstrap requires dpkg-deb and Debian 13-compatible host libs.
No command installs system packages or requires administrator privileges.
"""
from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import platform
import shlex
import shutil
import struct
import subprocess
import sys
import tarfile
import urllib.request
from zipfile import ZIP_DEFLATED, ZipFile


ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "native"
DEFAULT_TOOLS = ROOT.parent / ".native-tools" if (ROOT.parent / ".native-tools").exists() else ROOT / ".native-tools"
TOOLS = Path(os.environ.get("VIVIS_TOOL_ROOT", str(DEFAULT_TOOLS))).expanduser().resolve()
RAYLIB_VERSION = "5.5"
RELEASE = f"https://github.com/raysan5/raylib/releases/download/{RAYLIB_VERSION}"
RAYLIB = {
    "linux": ("raylib-5.5_linux_amd64", "raylib-linux.tar.gz", f"{RELEASE}/raylib-5.5_linux_amd64.tar.gz", "3d95ef03d5b38dfa55c0a16ca122d382134b078f0e5b270b52fe7eae0549c000"),
    "windows": ("raylib-5.5_win64_mingw-w64", "raylib-windows.zip", f"{RELEASE}/raylib-5.5_win64_mingw-w64.zip", "fe8feda1b92fc02826e2d19c631d648fd0563b93b144893fe18140e9fe4fb6fd"),
}
DEBIAN = "https://snapshot.debian.org/archive/debian/20260925T181338Z/pool/main"
# Debian trixie x86-64 packages. The pinned snapshot and SHA256s prevent updates
# from silently changing the compiler. Their files are extracted, never installed.
MINGW_PACKAGES = [
    ("g/gcc-mingw-w64/g++-mingw-w64-x86-64-posix_14.2.0-19+27+b1_amd64.deb", "cfa1773c2d0184819050138e587a81606d04a5f92e278b521b72b776d2c256c1"),
    ("g/gcc-mingw-w64/gcc-mingw-w64-x86-64-posix_14.2.0-19+27+b1_amd64.deb", "e32d24cfd1482394f16d9a84c1d0a9f5aa08ee826a79acc383c70cc8c23c4fe5"),
    ("g/gcc-mingw-w64/gcc-mingw-w64-x86-64-posix-runtime_14.2.0-19+27+b1_amd64.deb", "c898d96177574a8a7238564a1e799d8ddfed5a3d2ea005510305f9ad466a6f59"),
    ("g/gcc-mingw-w64/gcc-mingw-w64-base_14.2.0-19+27+b1_amd64.deb", "8d2c64b886ab4a435a78ddc4cd3b510f149b65fbd6be8472e8f460627562718a"),
    ("m/mingw-w64/mingw-w64-x86-64-dev_12.0.0-5_all.deb", "0bf89cf7454cccb49cd2a46a6f7b33896f6ae3a1a2fe33e02c40be6155bbb385"),
    ("m/mingw-w64/mingw-w64-common_12.0.0-5_all.deb", "97dce5d0d8aff1cade4786ad4ef5078345477a598829afcac2e7f2fbc5399990"),
    ("b/binutils-mingw-w64/binutils-mingw-w64-x86-64_2.44-3+12+b1_amd64.deb", "b53f4a091b16a0d0952b8e3bd6027931641386258f96b37db1f0c62c787221d3"),
]
ASSETS = {
    "characters": ROOT / "godot_game/playable/assets/characters.png",
    "emotions": ROOT / "godot_game/playable/assets/character_emotions.png",
    "font": ROOT / "godot_game/playable/assets/pixel_font.ttf",
    "title": ROOT / "native_game/assets/title.png",
    "icon": ROOT / "native_game/assets/icon.png",
}
LINUX_LIBS = ["-l:libGL.so.1", "-l:libX11.so.6", "-lm", "-lpthread", "-ldl", "-lrt"]
WINDOWS_LIBS = ["-lopengl32", "-lgdi32", "-lwinmm", "-static", "-static-libgcc", "-static-libstdc++", "-mwindows"]


def run(command: list[str], **kwargs) -> subprocess.CompletedProcess:
    print("+", shlex.join([str(item) for item in command]), flush=True)
    return subprocess.run([str(item) for item in command], check=True, cwd=ROOT, **kwargs)


def download(url: str, destination: Path, expected: str) -> Path:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.exists():
        print(f"Downloading {url}", flush=True)
        temporary = destination.with_suffix(destination.suffix + ".part")
        request = urllib.request.Request(url, headers={"User-Agent": "Sorpresa-Vivis-native-build/1"})
        try:
            with urllib.request.urlopen(request, timeout=120) as source, temporary.open("wb") as output:
                shutil.copyfileobj(source, output)
            temporary.replace(destination)
        finally:
            temporary.unlink(missing_ok=True)
    actual = hashlib.sha256(destination.read_bytes()).hexdigest()
    if actual != expected:
        raise RuntimeError(f"SHA256 mismatch for {destination}: {actual}; expected {expected}. Remove the cached file and retry.")
    return destination


def raylib_dir(target: str) -> Path:
    override = os.environ.get(f"VIVIS_RAYLIB_{target.upper()}")
    return Path(override).expanduser().resolve() if override else TOOLS / RAYLIB[target][0]


def cross_compiler() -> tuple[str, dict[str, str]]:
    override = os.environ.get("VIVIS_WINDOWS_CXX")
    system = shutil.which("x86_64-w64-mingw32-g++") or shutil.which("x86_64-w64-mingw32-g++-posix")
    extracted = TOOLS / "mingw/usr/bin/x86_64-w64-mingw32-g++-posix"
    compiler = override or system or (str(extracted) if extracted.exists() else None)
    if not compiler:
        raise RuntimeError("MinGW C++ compiler missing. Run with --bootstrap --platform windows, or set VIVIS_WINDOWS_CXX.")
    environment = os.environ.copy()
    if Path(compiler).resolve() == extracted:
        environment["PATH"] = str(extracted.parent) + os.pathsep + environment.get("PATH", "")
    return compiler, environment


def bootstrap(targets: list[str]) -> None:
    TOOLS.mkdir(parents=True, exist_ok=True)
    for target in targets:
        directory, filename, url, digest = RAYLIB[target]
        if (raylib_dir(target) / "lib/libraylib.a").exists():
            continue
        archive = download(url, TOOLS / filename, digest)
        if target == "linux":
            with tarfile.open(archive) as package:
                # The verified archive is a pinned upstream release.
                package.extractall(TOOLS)
        else:
            with ZipFile(archive) as package:
                package.extractall(TOOLS)
        if not (raylib_dir(target) / "lib/libraylib.a").exists():
            raise RuntimeError(f"raylib archive did not provide {directory}/lib/libraylib.a")
    if "windows" in targets:
        try:
            cross_compiler()
        except RuntimeError:
            if platform.system() != "Linux" or platform.machine() not in ("x86_64", "amd64"):
                raise RuntimeError("Automatic MinGW setup supports Linux x86-64. Set VIVIS_WINDOWS_CXX to a compatible compiler.")
            if not shutil.which("dpkg-deb"):
                raise RuntimeError("dpkg-deb is needed to unpack the pinned MinGW compiler, or set VIVIS_WINDOWS_CXX.")
            for filename, digest in MINGW_PACKAGES:
                archive = download(f"{DEBIAN}/{filename}", TOOLS / "downloads" / Path(filename).name, digest)
                run(["dpkg-deb", "-x", str(archive), str(TOOLS / "mingw")])
    print(f"Dependencies ready in {TOOLS}. Subsequent builds do not use the network.")


def embed_assets() -> Path:
    for name, asset in ASSETS.items():
        if not asset.is_file():
            raise RuntimeError(f"Required {name} asset is missing: {asset}. Restore/create it before building.")
    generated = BUILD / "generated"
    generated.mkdir(parents=True, exist_ok=True)
    header = generated / "assets_embedded.hpp"
    fingerprint = hashlib.sha256(b"".join(asset.read_bytes() for asset in ASSETS.values())).hexdigest()
    marker = generated / "assets.sha256"
    if header.exists() and marker.exists() and marker.read_text() == fingerprint:
        return generated
    temporary = header.with_suffix(".tmp")
    with temporary.open("w", encoding="utf-8") as output:
        output.write("// Generated by scripts/build_native.py. Do not edit.\n#pragma once\n#include <cstddef>\nnamespace vivis::embedded {\n")
        for name, asset in ASSETS.items():
            data = asset.read_bytes()
            output.write(f"inline constexpr unsigned char {name}[] = {{\n")
            for offset in range(0, len(data), 24):
                output.write(",".join(str(byte) for byte in data[offset:offset + 24]) + ",\n")
            output.write(f"}};\ninline constexpr std::size_t {name}_size = sizeof({name});\n")
        output.write("}\n")
    temporary.replace(header)
    marker.write_text(fingerprint)
    print(f"Embedded {sum(asset.stat().st_size for asset in ASSETS.values()) / 1048576:.2f} MiB of artwork and fonts.")
    return generated


def library(target: str) -> tuple[Path, Path]:
    directory = raylib_dir(target)
    include, archive = directory / "include", directory / "lib/libraylib.a"
    if not (include / "raylib.h").exists() or not archive.exists():
        raise RuntimeError(f"raylib {RAYLIB_VERSION} for {target} missing from {directory}. Run --bootstrap or set VIVIS_RAYLIB_{target.upper()}.")
    return include, archive


def prepare_icon(icon: Path) -> bytes:
    data = icon.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise RuntimeError("native_game/assets/icon.png must be PNG.")
    width, height = struct.unpack(">II", data[16:24])
    if width != height:
        raise RuntimeError("native_game/assets/icon.png must be square.")
    if width <= 256:
        return data
    # Reuse the pinned image codec already supplied by raylib. This conversion
    # does not create a window and needs no Pillow/ImageMagick installation.
    include, archive = library("linux")
    generated = BUILD / "generated"
    helper_source, helper = generated / "resize-icon.cpp", generated / "resize-icon"
    helper_source.write_text('''#include "raylib.h"
int main(int argc,char** argv) {
    if(argc!=3) return 1;
    SetTraceLogLevel(LOG_WARNING);
    Image icon=LoadImage(argv[1]);
    if(!icon.data) return 2;
    ImageResizeNN(&icon,256,256);
    bool ok=ExportImage(icon,argv[2]);
    UnloadImage(icon);
    return ok?0:3;
}
''', encoding="utf-8")
    run([os.environ.get("CXX", "g++"), "-std=c++17", "-O2", str(helper_source), "-I", str(include), str(archive), *LINUX_LIBS, "-o", str(helper)])
    thumbnail = generated / "icon-256.png"
    run([str(helper), str(icon), str(thumbnail)])
    return thumbnail.read_bytes()


def windows_resource(compiler: str, environment: dict[str, str]) -> list[str]:
    icon = ROOT / "native_game/assets/icon.png"
    if not icon.exists():
        return []
    windres = shutil.which("x86_64-w64-mingw32-windres", path=environment.get("PATH"))
    if not windres:
        raise RuntimeError("Windows icon exists but MinGW windres is missing from PATH.")
    data = prepare_icon(icon)
    width, height = struct.unpack(">II", data[16:24])
    generated = BUILD / "generated"
    ico = generated / "game.ico"
    # Windows Vista+ supports a PNG payload in the ICO container.
    ico.write_bytes(struct.pack("<HHH", 0, 1, 1) + struct.pack("<BBBBHHII", width % 256, height % 256, 0, 0, 1, 32, len(data), 22) + data)
    resource = generated / "game.rc"
    resource.write_text('1 ICON "' + str(ico).replace("\\", "/") + '"\n' + '''
1 VERSIONINFO
 FILEVERSION 0,3,0,0
 PRODUCTVERSION 0,3,0,0
 FILEFLAGSMASK 0x3fL
 FILEFLAGS 0x0L
 FILEOS 0x40004L
 FILETYPE 0x1L
 FILESUBTYPE 0x0L
BEGIN
 BLOCK "StringFileInfo"
 BEGIN
  BLOCK "040904b0"
  BEGIN
   VALUE "FileDescription", "Sorpresa Vivis - Aventura 2D\\0"
   VALUE "ProductName", "Sorpresa Vivis\\0"
   VALUE "FileVersion", "0.3.0\\0"
   VALUE "ProductVersion", "0.3.0\\0"
   VALUE "InternalName", "Sorpresa-Vivis\\0"
   VALUE "OriginalFilename", "Sorpresa-Vivis.exe\\0"
  END
 END
 BLOCK "VarFileInfo"
 BEGIN
  VALUE "Translation", 0x0409, 1200
 END
END
''', encoding="utf-8")
    output = generated / "game-resource.o"
    run([windres, f"--preprocessor={compiler}", "--preprocessor-arg=-E", "--preprocessor-arg=-xc", "--preprocessor-arg=-DRC_INVOKED", "--codepage=65001", "-i", str(resource), "-o", str(output)], env=environment)
    return [str(output)]


def build(target: str, generated: Path) -> Path:
    include, archive = library(target)
    sources = sorted((ROOT / "native_game/src").glob("*.cpp"))
    if not sources:
        raise RuntimeError("No native_game/src/*.cpp sources exist yet.")
    destination = BUILD / target
    destination.mkdir(parents=True, exist_ok=True)
    executable = destination / ("Sorpresa-Vivis.exe" if target == "windows" else "Sorpresa-Vivis")
    common = ["-std=c++17", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Wpedantic", "-I", str(include), "-I", str(generated), "-I", str(ROOT / "native_game/src")]
    if target == "windows":
        compiler, environment = cross_compiler()
        resources = windows_resource(compiler, environment)
        run([compiler, *common, *map(str, sources), *resources, str(archive), *WINDOWS_LIBS, "-s", "-o", str(executable)], env=environment)
    else:
        compiler = os.environ.get("CXX", "g++")
        run([compiler, *common, *map(str, sources), str(archive), *LINUX_LIBS, "-static-libgcc", "-static-libstdc++", "-s", "-o", str(executable)])
        executable.chmod(0o755)
    print(f"Built {executable} ({executable.stat().st_size / 1048576:.2f} MiB)")
    return executable


def test() -> None:
    sources = sorted((ROOT / "native_game/tests").glob("*.cpp"))
    if not sources:
        raise RuntimeError("--test requested, but native_game/tests has no C++ test files.")
    include, archive = library("linux")
    BUILD.mkdir(parents=True, exist_ok=True)
    executable = BUILD / "model-tests"
    model = [ROOT / "native_game/src/game.cpp"]
    profile = ROOT / "native_game/src/profile.cpp"
    if profile.exists():
        model.append(profile)
    run([os.environ.get("CXX", "g++"), "-std=c++17", "-O1", "-g", "-Wall", "-Wextra", "-I", str(include), "-I", str(ROOT / "native_game/src"), *map(str, sources), *map(str, model), str(archive), *LINUX_LIBS, "-o", str(executable)])
    run([str(executable)])


INSTRUCTIONS = """SORPRESA VIVI’S — AVENTURA 2D PARA PC

Windows: descomprime todo y abre Sorpresa-Vivis.exe.
Linux: descomprime y ejecuta ./Sorpresa-Vivis; si hace falta, chmod +x Sorpresa-Vivis.
Incluye personajes, escenarios, tipografías y sonido en un solo ejecutable.
Funciona sin Internet. No necesitas instalar un motor de videojuegos.

WASD / flechas: mover. Espacio (mantener): magia al enemigo más cercano.
E / Enter: ayuda de Wiky e interacción. Shift: esquivar.
Esc: pausa / volver. R: reintentar. M: sonido. F11: pantalla completa.
Los menús funcionan con ratón o flechas y Enter.

Recoge el cascabel, elimina las pelusas y busca la salida del escenario.
La partida guarda un punto de control al avanzar; continúa desde el menú.
Consulta los controles y ajustes dentro del juego.

Windows: equipo de 64 bits con OpenGL 3.3. Compatible con Windows 10/11.
Linux: x86-64, glibc 2.38 o posterior, X11 y OpenGL 3.3.
No se incluyen controladores de video ni componentes del sistema operativo.
"""


def package(target: str, executable: Path) -> None:
    label = "Windows" if target == "windows" else "Linux"
    output = ROOT / "dist" / f"Sorpresa-Vivis-Native-{label}.zip"
    output.parent.mkdir(parents=True, exist_ok=True)
    with ZipFile(output, "w", ZIP_DEFLATED, compresslevel=9) as archive:
        archive.write(executable, executable.name)
        archive.writestr("LEEME.txt", INSTRUCTIONS)
        archive.write(ROOT / "godot_game/playable/assets/OFL.txt", "licenses/LICENSE-Pixelify-Sans.txt")
        archive.write(raylib_dir(target) / "LICENSE", "licenses/LICENSE-raylib.txt")
        archive.write(ROOT / "native_game/THIRD_PARTY.md", "licenses/THIRD_PARTY.txt")
    print(f"Packaged {output} ({output.stat().st_size / 1048576:.2f} MiB)")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--platform", choices=("linux", "windows", "all"), default="all")
    parser.add_argument("--bootstrap", action="store_true", help="Allow verified dependency downloads; never installs system packages")
    parser.add_argument("--bootstrap-only", action="store_true", help="Prepare dependencies without compiling (requires --bootstrap)")
    parser.add_argument("--test", action="store_true", help="Compile and run native_game/tests against the game model")
    args = parser.parse_args()
    if args.bootstrap_only and not args.bootstrap:
        parser.error("--bootstrap-only requires --bootstrap")
    targets = ["linux", "windows"] if args.platform == "all" else [args.platform]
    if args.bootstrap:
        # Linux raylib also supplies the image codec for Windows icon generation.
        bootstrap(list(dict.fromkeys(targets + (["linux"] if args.test or "windows" in targets else []))))
    if args.bootstrap_only:
        return
    if args.test:
        test()
    generated = embed_assets()
    for target in targets:
        executable = build(target, generated)
        package(target, executable)


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, OSError, subprocess.CalledProcessError) as error:
        print(f"Build failed: {error}", file=sys.stderr)
        sys.exit(1)
