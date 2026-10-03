# Descent (Visual Studio)

A cleaned-up copy of Parallax Software's Descent 1 (v1.5) source release,
reorganized so it can be opened and worked on as a modern CMake / Visual
Studio project. The original release notes and license are in
[docs/readme.txt](docs/readme.txt); the code may only be used for
non-commercial purposes.

**The code does not compile yet.** It is DOS code written for Watcom C and
MASM; see [docs/porting-notes.md](docs/porting-notes.md) for what is missing
and what has to change.

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
| `src/tools/`    | `hogfile`, `lbmcomp` and `xcolor` data tools                    |
| `data/`         | `editdata.exe`, a self-extracting archive of the editor's data files |
| `docs/`         | Original readme, build variant notes, porting notes             |

Each library keeps the name of the `.lib` file the original makefiles
produced. Headers used by only one module live next to its sources.

## Generating a Visual Studio solution

The assembly sources are 32-bit x86, so generate a Win32 solution:

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32

Options:

- `DESCENT_EDITOR` (OFF): build the level editor and define `EDITOR`.
- `DESCENT_TOOLS` (ON): build the data tools.

With any other generator or a 64-bit target the `.asm` files are still listed
in the project but are not assembled.
