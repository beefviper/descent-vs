# Porting notes

What stands between this tree and a working Visual Studio build.

## Missing from the source release

Parallax removed code it did not own the copyright to. Kevin Bentley added
stand-in headers so the rest would compile, but none of it works:

- **Sound and music.** `src/main/digi.c` was written against the Human
  Machine Interfaces SOS sound library. Its headers (`sos.h`, `sosm.h`,
  `soscomp.h`) are gone; `include/no_sos.h` only declares enough types for
  the file to compile. Digital sound and MIDI need a new backend.
- **Modem and serial play.** `src/main/modem.c` used the Greenleaf CommLib
  (`commlib.h`, `fast.h`, `glfmodem.h`), which is not included;
  `src/main/nocomlib.h` holds dummy values.
- **The Watcom build include `makefile.def`** and the generator for
  `vers_id.h`. The generated `src/main/vers_id.h` for the registered build is
  kept as is.
- **Game data.** The release contains no `descent.hog` or `descent.pig`; a
  copy of the registered game is needed to run anything.

## Written for DOS, Watcom and MASM

- **DOS and hardware access.** About 40 files include `<dos.h>` or
  `<conio.h>`, several use `<i86.h>`, `int386()` and DPMI calls, and the
  graphics code programs VGA, Mode X and VESA registers directly. The
  keyboard, mouse, joystick and timer drivers in `src/bios/` hook DOS
  interrupts. All of it needs replacing (for example with SDL or Win32).
- **Watcom inline assembly.** 136 `#pragma aux` directives in 20 files define
  inline assembly and register calling conventions MSVC does not understand,
  notably the fixed-point helpers in `include/fix.h` and `include/vecmat.h`.
  They need C or MSVC `__asm` replacements.
- **Assembly modules.** 28 `.asm` files (fixed-point math, the 3D pipeline,
  the texture mappers, low-level 2D blitters and drivers) are written for
  MASM 5.1 with the flat model and expect arguments in registers, matching
  the `#pragma aux` declarations. They assemble only with 32-bit MASM and
  must be called through matching prototypes, or rewritten in C.
- **Watcom C extensions.** About 150 files use `#pragma off (unreferenced)`
  around their RCS id strings, and about two dozen use `far`/`_far`
  pointers. Expect these among the first compile errors.
- **Debug macros.** `include/error.h` declares `Assert()` and `Int3()` as
  functions (Kevin Bentley's workaround); the debug branch still relies on
  Watcom pragmas.

## Build variants

The original makefiles built a registered game, rental, Destination Saturn
and editor variants from `.ini` settings; see
[build-variants.md](build-variants.md).
