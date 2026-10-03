# Descent (Visual Studio)

A cleaned-up copy of Parallax Software's Descent 1 (v1.5) source release,
reorganized so it can be opened and worked on as a modern CMake / Visual
Studio project. The original release notes and license are in
[docs/readme.txt](docs/readme.txt); the code may only be used for
non-commercial purposes.

The code compiles and links for 32-bit and 64-bit Windows, with all of the
original assembly rewritten in C, but it does not run yet: it still needs a
display, input and sound backend in place of the DOS hardware code. See
[docs/porting-notes.md](docs/porting-notes.md).

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
| `src/compat/`, `include/compat/` | Replacements for Watcom/DOS headers and runtime functions |
| `src/tools/`    | `hogfile`, `lbmcomp` and `xcolor` data tools                    |
| `data/`         | `editdata.exe`, a self-extracting archive of the editor's data files |
| `docs/`         | Original readme, build variant notes, porting notes             |

Each library keeps the name of the `.lib` file the original makefiles
produced. Headers used by only one module live next to its sources.

## Generating a Visual Studio solution

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32

Use `-A x64` for a 64-bit build.

Options:

- `DESCENT_EDITOR` (OFF): build the level editor into the game. Editor
  builds define `EDITOR` and leave `RELEASE` and `NDEBUG` off, as the
  original editor variant did.
- `DESCENT_TOOLS` (ON): build the data tools.
