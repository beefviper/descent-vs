# Descent (Visual Studio)

A cleaned-up copy of Parallax Software's Descent 1 (v1.5) source release,
reorganized so it can be opened and worked on as a modern CMake / Visual
Studio project. The original release notes and license are in
[docs/readme.txt](docs/readme.txt); the code may only be used for
non-commercial purposes.

The code compiles and links for 32-bit and 64-bit Windows, with all of the
original assembly rewritten in C. It opens an SDL2 window for the graphics
and keyboard, and a console window that shows the debug output the
original sent to a monochrome monitor. Mouse, sound and networking are
not done yet. See [docs/porting-notes.md](docs/porting-notes.md).

## Layout

| Path            | Contents                                                        |
|-----------------|-----------------------------------------------------------------|
| `include/`      | Headers and assembler include files shared between modules      |
| `src/main/`     | The game (`descent`)                                            |
| `src/editor/`   | The level editor (built into the game with `DESCENT_EDITOR=ON`) |
| `src/2d/`       | 2D graphics library (`gr`)                                      |
| `src/3d/`       | 3D rendering pipeline (`3d`)                                    |
| `src/texmap/`   | Texture mapper (`texmap`)                                       |
| `src/bios/`     | Keyboard, mouse, joystick, timer, DPMI and IPX (`io`)           |
| `src/fix/`, `src/vecmat/`, `src/div/` | Fixed-point math, vectors and matrices, divide overflow handling |
| `src/cfile/`, `src/iff/`, `src/mem/`, `src/misc/` | File access, IFF images, memory and error helpers |
| `src/ui/`       | The editor's user interface toolkit (`ui`)                      |
| `src/pslib/`    | Compressed library archiver and its `cflib`/`readfile` libraries |
| `src/platform/` | SDL2 window, display and keyboard backend (`platform`)         |
| `src/compat/`, `include/compat/` | Replacements for Watcom/DOS headers and runtime functions |
| `src/tools/`    | `hogfile`, `lbmcomp` and `xcolor` data tools                    |
| `data/`         | `editdata.exe`, a self-extracting archive of the editor's data files |
| `docs/`         | Original readme, build variant notes, porting notes             |

Each library keeps the name of the `.lib` file the original makefiles
produced. Headers used by only one module live next to its sources.

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
(`descent.hog`, `descent.pig`). In Visual Studio, set the debugger's
working directory to that folder.

Options:

- `DESCENT_EDITOR` (OFF): build the level editor into the game. Editor
  builds define `EDITOR` and leave `RELEASE` and `NDEBUG` off, as the
  original editor variant did.
- `DESCENT_TOOLS` (ON): build the data tools.
