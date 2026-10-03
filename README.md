# Descent (Visual Studio)

A cleaned-up copy of Parallax Software's Descent 1 (v1.5) source release,
reorganized so it can be opened and worked on as a modern CMake / Visual
Studio project. The original release notes and license are in
[docs/readme.txt](docs/readme.txt); the code may only be used for
non-commercial purposes.

The code compiles and links for 32-bit and 64-bit Windows, with all of the
original assembly rewritten in C. It opens an SDL2 window for the graphics
keyboard and sound effects, and a console window that shows the debug
output the original sent to a monochrome monitor. Mouse, music and
networking are not done yet. See [docs/porting-notes.md](docs/porting-notes.md).

## Layout

| Path            | Contents                                                        |
|-----------------|-----------------------------------------------------------------|
| `include/`      | Headers shared between modules, grouped below                   |
| `source/main/`  | The game (`descent`), grouped below                             |
| `source/editor/` | The level editor (built into the game with `DESCENT_EDITOR=ON`) |
| `source/2d/`    | 2D graphics library (`gr`)                                      |
| `source/3d/`    | 3D rendering pipeline (`3d`)                                    |
| `source/texmap/` | Texture mapper (`texmap`)                                      |
| `source/bios/`  | Keyboard, mouse, joystick, timer, DPMI and IPX (`io`)           |
| `source/fix/`, `source/vecmat/`, `source/div/` | Fixed-point math, vectors and matrices, divide overflow handling |
| `source/cfile/`, `source/iff/`, `source/mem/`, `source/misc/` | File access, IFF images, memory and error helpers |
| `source/ui/`    | The editor's user interface toolkit (`ui`)                      |
| `source/pslib/` | Compressed library archiver and its `cflib`/`readfile` libraries |
| `source/platform/` | SDL2 window, display, keyboard and sound backend (`platform`) |
| `source/compat/`, `include/compat/` | Replacements for Watcom/DOS headers and runtime functions |
| `source/tools/` | `hogfile`, `lbmcomp` and `xcolor` data tools                    |
| `data/`         | `editdata.exe`, a self-extracting archive of the editor's data files |
| `docs/`         | Original readme, build variant notes, porting notes             |

Each library keeps the name of the `.lib` file the original makefiles
produced. Headers used by only one module live next to its sources.

The library directories under `source/` are the directories of the
original makefiles. The game itself was one flat directory of about 150
files; it is now split by subsystem, each `.c` file with its `.h`:

| `source/main/` | Contents |
|----------------|----------|
| `core/`    | Startup, main loop, level sequencing, options, players' save files, missions, demos, version ids |
| `data/`    | Bitmap and sound tables, the pig file, texture caching, polygon models, animations, fonts |
| `world/`   | Mine geometry and segments, level loading and saving, walls, triggers, fuel centers, reactor, collision tracing |
| `objects/` | Objects, physics, collisions, weapons, powerups, explosions, hostages, players |
| `ai/`      | Robot AI, pathfinding, robot definitions |
| `render/`  | Mine renderer, automap, cockpit gauges, HUD, lighting, terrain and exit sequence |
| `menus/`   | Menus, controls setup, credits, briefings, high scores, kill matrix |
| `net/`     | IPX and serial/modem multiplayer |
| `sound/`   | Sound effects and music |
| `devices/` | Head-mounted displays, 3D glasses, arcade and CD-ROM hooks |

| `include/` | Contents |
|------------|----------|
| `2d/`       | Graphics library, palettes, PCX/IFF/RLE bitmaps |
| `3d/`       | 3D pipeline and texture mapper |
| `math/`     | Fixed-point math, vectors and matrices |
| `bios/`     | Keyboard, mouse, joystick, timer, DPMI, IPX, mono debug output |
| `cfile/`    | File and library archive access |
| `misc/`     | Errors, memory, basic types, argument parsing |
| `ui/`       | Editor UI toolkit |
| `platform/` | SDL2 backend interface |
| `compat/`   | Watcom/DOS header replacements |

Every group directory is on the include path, so the sources still use
plain `#include "gr.h"` as the originals did.

## Generating a Visual Studio solution

The game needs SDL2. The easiest way to get it is vcpkg, which comes with
Visual Studio 2022: the `vcpkg.json` manifest makes CMake fetch and build
SDL2 automatically.

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 ^
        -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

Use `-A x64` for a 64-bit build. Without vcpkg, download the SDL2 VC
development package from https://github.com/libsdl-org/SDL/releases and
pass `-DSDL2_DIR=<path>/cmake` instead of the toolchain file. The build
copies `SDL2.dll` next to `descent.exe`.

Run `descent.exe` from the directory holding the game data
(`descent.hog`, `descent.pig`): it opens them from the current directory.
For Visual Studio, pass `-DDESCENT_DATA_DIR=<that folder>` when generating
the solution and the debugger will start the game there.

Options:

- `DESCENT_EDITOR` (OFF): build the level editor into the game. Editor
  builds define `EDITOR` and leave `RELEASE` and `NDEBUG` off, as the
  original editor variant did.
- `DESCENT_TOOLS` (ON): build the data tools.
- `DESCENT_DATA_DIR` (empty): the game data folder, used as the Visual
  Studio debugger's working directory.
