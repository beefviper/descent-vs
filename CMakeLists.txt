cmake_minimum_required(VERSION 3.20)

project(Descent
    VERSION 1.5
    DESCRIPTION "Descent 1 (v1.5) source release"
    LANGUAGES C)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

option(DESCENT_EDITOR "Build the level editor into the game (defines EDITOR)" OFF)
option(DESCENT_TOOLS  "Build the hogfile, pslib, lbmcomp and xcolor tools"     ON)

# The original sources are 32-bit x86 C and MASM. The assembly can only be
# built with the 32-bit MASM (ml.exe), so the assembler is enabled only for a
# 32-bit MSVC build (generate with: cmake -G "Visual Studio 17 2022" -A Win32).
if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    enable_language(ASM_MASM)
    set(DESCENT_HAVE_MASM ON)
else()
    set(DESCENT_HAVE_MASM OFF)
    message(WARNING
        "The assembly sources need 32-bit MASM and are left out of this build. "
        "Use a 32-bit Visual Studio generator (-A Win32) to include them.")
endif()

# ---------------------------------------------------------------------------
# Compiler settings
# ---------------------------------------------------------------------------

# Every library and the game were built with these definitions. RELEASE
# compiles out the debugging aids the editor depends on (slew mode), so it
# is left off for editor builds, as the original editor variant did.
add_compile_definitions(NETWORK NDEBUG)
if(NOT DESCENT_EDITOR)
    add_compile_definitions(RELEASE)
endif()

# Watcom's char is unsigned; the code relies on it.
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/J>)
else()
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-funsigned-char>)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/W3>)
    # Assemble the MASM 5.1-era sources in compatibility mode.
    add_compile_options($<$<COMPILE_LANGUAGE:ASM_MASM>:/Zm>)
endif()

# Headers shared between modules. The assembler include files (*.inc) live
# here as well, so the same path serves C and MASM.
set(DESCENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${DESCENT_INCLUDE_DIR})

# Stand-ins for the Watcom/DOS headers (dos.h, i86.h, bios.h) and a header,
# force-included everywhere, that neutralizes Watcom keywords such as far.
include_directories(BEFORE ${DESCENT_INCLUDE_DIR}/compat)
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/FIcompat.h>)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/wd4068>)    # unknown pragma
else()
    add_compile_options("$<$<COMPILE_LANGUAGE:C>:SHELL:-include compat.h>")
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-unknown-pragmas>)
endif()

# descent_add_library(<name> <dir> <sources>...)
# descent_add_executable(<name> <dir> <sources>...)
#
# Sources are relative to <dir> under src/. The directory is added to the
# target's include path so module-private headers resolve. When MASM is
# unavailable the .asm files stay in the project for browsing but are not
# assembled.
function(_descent_sources out dir)
    set(result)
    foreach(src IN LISTS ARGN)
        set(path ${CMAKE_CURRENT_SOURCE_DIR}/src/${dir}/${src})
        if(src MATCHES "\\.asm$" AND NOT DESCENT_HAVE_MASM)
            set_source_files_properties(${path} PROPERTIES HEADER_FILE_ONLY TRUE)
        endif()
        list(APPEND result ${path})
    endforeach()
    set(${out} ${result} PARENT_SCOPE)
endfunction()

function(descent_add_library name dir)
    _descent_sources(sources ${dir} ${ARGN})
    add_library(${name} STATIC ${sources})
    set_target_properties(${name} PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
    set_target_properties(${name} PROPERTIES FOLDER "Libraries")
endfunction()

function(descent_add_executable name dir)
    _descent_sources(sources ${dir} ${ARGN})
    add_executable(${name} ${sources})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/${dir})
endfunction()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------
# Libraries (one per original subsystem; names match the original .lib files)
# ---------------------------------------------------------------------------

# Implementations of the DOS services declared in include/compat/dos.h
descent_add_library(compat compat
    dos.c)
link_libraries(compat)

descent_add_library(misc misc
    error.c parsarg.c)

descent_add_library(fix fix
    fix.asm)

descent_add_library(div0 div
    div0.asm)

descent_add_library(vecmat vecmat
    vecmat.asm)

descent_add_library(mem mem
    mem.c)

descent_add_library(cfile cfile
    cfile.c)

descent_add_library(iff iff
    iff.c)

# 2d graphics
descent_add_library(gr 2d
    linear.asm vesa.asm modex.asm canvas.c bitmap.c gr.c font.c bitblt.c
    scanline.c pixel.c poly.c disc.c rect.c box.c gpixel.c palette.c line.c
    lbitblt.asm scale.c scalea.asm circle.c ibitblt.c rle.c pcx.c)

# 3d rendering pipeline
descent_add_library(3d 3d
    globvars.asm setup.asm matrix.asm points.asm draw.asm clipper.asm
    horizon.asm instance.asm rod.asm interp.asm)

# Keyboard, joystick, mouse, timer, DPMI and IPX
descent_add_library(io bios
    key.c joy.asm mouse.c timer.asm mono.c joyc.c dpmi.c ipx.c)

descent_add_library(texmap texmap
    ntmap.c tmapflat.c tmap_per.asm tmap_lin.asm tmap_ll.asm tmap_flt.asm
    tmapfade.asm tmap_fl1.asm scanline.c)
# ntmap.c uses the game's texture table (textures.h).
target_include_directories(texmap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/main)

# Editor user interface toolkit
descent_add_library(ui ui
    ui.c mouse.c window.c uidraw.c button.c gadget.c listbox.c message.c
    popup.c scroll.c radio.c checkbox.c inputbox.c userbox.c menu.c
    harderr.c file.c keytrap.c icon.c keypress.c menubar.c keypad.c)

# Compressed file library used by the tools
descent_add_library(cflib pslib
    lzw.c bitio.c library.c)

descent_add_library(readfile pslib
    readfile.c)

# ---------------------------------------------------------------------------
# Level editor
# ---------------------------------------------------------------------------

set(DESCENT_EDITOR_SOURCES
    med.c einit.c eglobal.c mine.c curves.c kcurve.c khelp.c ksegsel.c
    kbuild.c kview.c kmine.c seguvs.c ksegsize.c ksegmove.c ktmap.c kfuncs.c
    autosave.c group.c texture.c fixseg.c eobject.c elight.c texpage.c
    objpage.c medmisc.c medsel.c kgame.c info.c medwall.c eswitch.c
    medrobot.c meddraw.c segment.c ehostage.c centers.c func.c)

if(DESCENT_EDITOR)
    descent_add_library(editor editor ${DESCENT_EDITOR_SOURCES})
    # The editor and the game include each other's headers.
    target_include_directories(editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/main)
    target_compile_definitions(editor PUBLIC EDITOR)
endif()

# ---------------------------------------------------------------------------
# The game
# ---------------------------------------------------------------------------

descent_add_executable(descent main
    inferno.c game.c gauges.c mglobal.c scores.c object.c laser.c physics.c
    bm.c menu.c gamesave.c switch.c effects.c texmerge.c polyobj.c
    gamemine.c fireball.c ai.c aipath.c hostage.c powerup.c fuelcen.c
    digi.c render.c hash.c piggy.c args.c gameseg.c wall.c collide.c
    lighting.c robot.c morph.c vclip.c weapon.c fvi.c newdemo.c titles.c
    vfx.c gameseq.c controls.c automap.c text.c network.c newmenu.c
    netmisc.c victor.c gamefont.c joydefs.c hud.c playsave.c endlevel.c
    terrain.c kconfig.c modem.c multi.c cntrlcen.c credits.c config.c
    soscodec.asm kmatrix.c paging.c mission.c iglasses.c songs.c bmread.c
    state.c multibot.c nocomlib.c slew.c dumpmine.c)

target_include_directories(descent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/editor)
target_link_libraries(descent PRIVATE
    3d gr fix io iff vecmat cfile mem ui div0 misc texmap
    $<$<BOOL:${DESCENT_EDITOR}>:editor>)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT descent)

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------

if(DESCENT_TOOLS)
    # Packs data files into a .hog archive
    descent_add_executable(hogfile tools/hogfile hogfile.c)

    # Compressed library archiver
    descent_add_executable(pslib pslib pslib.c)
    target_link_libraries(pslib PRIVATE cflib misc mem io)

    # Bitmap converter
    descent_add_executable(lbmcomp tools/lbmcomp lbmcomp.c)
    target_link_libraries(lbmcomp PRIVATE gr misc mem iff readfile ui io cflib cfile)

    # Fixes the transparency color of .bbm files
    descent_add_executable(xcolor tools/xcolor xcolor.c)
    target_link_libraries(xcolor PRIVATE gr misc mem iff readfile ui io cflib)

    set_target_properties(hogfile pslib lbmcomp xcolor PROPERTIES FOLDER "Tools")
endif()
