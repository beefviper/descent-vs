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
option(DESCENT_MUSIC  "Play the music with libADLMIDI (OPL3 FM synth emulation)" ON)
set(DESCENT_DATA_DIR "" CACHE PATH
    "Folder with descent.hog and descent.pig; Visual Studio starts the game there")


# ---------------------------------------------------------------------------
# Compiler settings
# ---------------------------------------------------------------------------

# Every library and the game were built with these definitions. RELEASE and
# NDEBUG compile out the debugging aids the editor depends on (slew mode,
# object type names, debug marks), so editor builds leave them off, as the
# original editor variant did.
add_compile_definitions(NETWORK)
if(NOT DESCENT_EDITOR)
    add_compile_definitions(RELEASE NDEBUG)
else()
    # CMake's Release configurations add NDEBUG too; take it back out.
    foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
        string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
    endforeach()
endif()

# Watcom's char is unsigned, and it packed structures on byte boundaries
# by default; the code relies on both (the game reads its data files
# straight into structures).
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/J>)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/Zp1>)
else()
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-funsigned-char>)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/W3>)
endif()

# Headers shared between modules, grouped by the libraries they belong to.
set(DESCENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
foreach(group 2d 3d bios cfile math misc platform ui)
    include_directories(${DESCENT_INCLUDE_DIR}/${group})
endforeach()

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
# Sources are relative to <dir> under source/. The directory is added to the
# target's include path so module-private headers resolve.
function(_descent_sources out dir)
    set(result)
    foreach(src IN LISTS ARGN)
        list(APPEND result ${CMAKE_CURRENT_SOURCE_DIR}/source/${dir}/${src})
    endforeach()
    set(${out} ${result} PARENT_SCOPE)
endfunction()

function(descent_add_library name dir)
    _descent_sources(sources ${dir} ${ARGN})
    add_library(${name} STATIC ${sources})
    set_target_properties(${name} PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source/${dir})
    set_target_properties(${name} PROPERTIES FOLDER "Libraries")
endfunction()

function(descent_add_executable name dir)
    _descent_sources(sources ${dir} ${ARGN})
    add_executable(${name} ${sources})
    # Show the subsystem directories as folders in Solution Explorer.
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source/${dir} FILES ${sources})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source/${dir})
endfunction()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------
# SDL2: the window, display and keyboard (source/platform). With vcpkg, the
# vcpkg.json manifest fetches it; otherwise point SDL2_DIR (or
# CMAKE_PREFIX_PATH) at an SDL2 development package.
# ---------------------------------------------------------------------------

find_package(SDL2 REQUIRED CONFIG)

# libADLMIDI: the music (source/platform/music.c). vcpkg.json fetches it too.
# It is C++, so the game links with the C++ runtime when it is in.
if(DESCENT_MUSIC)
    find_package(libADLMIDI CONFIG)
    if(libADLMIDI_FOUND)
        enable_language(CXX)
    else()
        message(WARNING "libADLMIDI not found; building without music. "
            "Point libADLMIDI_DIR at its package, or set DESCENT_MUSIC=OFF.")
    endif()
endif()

# ---------------------------------------------------------------------------
# Libraries (one per original subsystem; names match the original .lib files)
# ---------------------------------------------------------------------------

# Implementations of the DOS services declared in include/compat/dos.h
descent_add_library(compat compat
    dos.c watcom.c)
link_libraries(compat)

descent_add_library(misc misc
    error.c parsarg.c)

descent_add_library(fix fix
    fix.c)

descent_add_library(div0 div
    div0.c)

descent_add_library(vecmat vecmat
    vecmat.c)

descent_add_library(mem mem
    mem.c)

descent_add_library(cfile cfile
    cfile.c)

descent_add_library(iff iff
    iff.c)

# 2d graphics
descent_add_library(gr 2d
    linear.c vesa.c modex.c canvas.c bitmap.c gr.c font.c bitblt.c
    scanline.c pixel.c poly.c disc.c rect.c box.c gpixel.c palette.c line.c
    lbitblt.c scale.c scalea.c circle.c ibitblt.c rle.c pcx.c)

# 3d rendering pipeline
descent_add_library(3d 3d
    globvars.c setup.c matrix.c points.c draw.c clipper.c horizon.c
    instance.c rod.c interp.c)

# Keyboard, joystick, mouse, timer, DPMI and IPX
descent_add_library(io bios
    key.c mouse.c timer.c mono.c joyc.c dpmi.c ipx.c)

# Window, display, keyboard and sound backend
descent_add_library(platform platform
    sdl.c audio.c music.c)
target_link_libraries(platform PUBLIC SDL2::SDL2)
if(DESCENT_MUSIC AND libADLMIDI_FOUND)
    target_compile_definitions(platform PRIVATE DESCENT_HAVE_ADLMIDI)
    target_link_libraries(platform PUBLIC libADLMIDI::ADLMIDI)
    if(MINGW)
        # Keep the game free of the MinGW C++ runtime DLLs.
        target_link_options(platform INTERFACE -static-libgcc -static-libstdc++)
    endif()
endif()

# The game's headers live next to its sources, grouped by subsystem under
# source/main. The editor and the texture mapper include some of them.
set(DESCENT_GAME_GROUPS core data world objects ai render menus net sound devices)
set(DESCENT_GAME_INCLUDE_DIRS)
foreach(group IN LISTS DESCENT_GAME_GROUPS)
    list(APPEND DESCENT_GAME_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/main/${group})
endforeach()

descent_add_library(texmap texmap
    ntmap.c tmapflat.c tmap_per.c tmap_lin.c tmap_ll.c tmap_flt.c
    tmapfade.c tmap_fl1.c scanline.c)
# ntmap.c uses the game's texture table (textures.h).
target_include_directories(texmap PRIVATE ${DESCENT_GAME_INCLUDE_DIRS})

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

# Dependencies between the libraries, so linkers that resolve static
# libraries in order (GNU ld) see them in a workable sequence.
target_link_libraries(misc   PUBLIC io)
target_link_libraries(mem    PUBLIC misc io)
target_link_libraries(cfile  PUBLIC mem misc)
target_link_libraries(vecmat PUBLIC fix)
target_link_libraries(io     PUBLIC fix mem misc platform)
target_link_libraries(gr     PUBLIC texmap iff cfile fix mem misc io platform)
target_link_libraries(iff    PUBLIC gr cfile mem misc)
target_link_libraries(texmap PUBLIC 3d gr vecmat fix mem misc)
target_link_libraries(3d     PUBLIC texmap gr vecmat fix div0)
target_link_libraries(ui     PUBLIC gr iff cfile io mem misc)

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
    target_include_directories(editor PRIVATE ${DESCENT_GAME_INCLUDE_DIRS})
    target_compile_definitions(editor PUBLIC EDITOR)
    target_link_libraries(editor PUBLIC ui 3d texmap gr iff vecmat fix io cfile mem misc)
endif()

# ---------------------------------------------------------------------------
# The game
# ---------------------------------------------------------------------------

descent_add_executable(descent main
    # Startup, the main loop, game sequencing, settings and saved games
    core/inferno.c core/game.c core/gameseq.c core/mglobal.c core/args.c
    core/controls.c core/slew.c core/config.c core/playsave.c core/state.c
    core/mission.c core/text.c core/newdemo.c core/dumpmine.c
    # Loading bitmaps, sounds, models and animations from the data files
    data/bm.c data/bmread.c data/piggy.c data/paging.c data/texmerge.c
    data/polyobj.c data/vclip.c data/effects.c data/gamefont.c data/hash.c
    # The mine: segments, walls, triggers, fuel and reactor centers
    world/gameseg.c world/gamemine.c world/gamesave.c world/wall.c
    world/switch.c world/fuelcen.c world/cntrlcen.c world/fvi.c
    # Objects and what happens to them
    objects/object.c objects/physics.c objects/collide.c objects/laser.c
    objects/weapon.c objects/powerup.c objects/fireball.c objects/morph.c
    objects/hostage.c
    # Robot behavior and path finding
    ai/ai.c ai/aipath.c ai/robot.c
    # Drawing the world, the automap, cockpit and HUD, and the exit sequence
    render/render.c render/automap.c render/gauges.c render/hud.c
    render/lighting.c render/terrain.c render/endlevel.c
    # Menus and full-screen pages
    menus/menu.c menus/newmenu.c menus/kconfig.c menus/joydefs.c
    menus/credits.c menus/titles.c menus/scores.c menus/kmatrix.c
    # Multiplayer over IPX and modem
    net/network.c net/netmisc.c net/modem.c net/multi.c net/multibot.c
    net/nocomlib.c
    # Sound effects and music
    sound/digi.c sound/songs.c
    # Optional hardware: VR headsets and glasses
    devices/iglasses.c devices/victor.c devices/vfx.c)

target_include_directories(descent PRIVATE ${DESCENT_GAME_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/source/editor)
target_link_libraries(descent PRIVATE
    3d gr fix io iff vecmat cfile mem ui div0 misc texmap
    $<$<BOOL:${DESCENT_EDITOR}>:editor>)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT descent)
# The game opens its data files from the current directory.
if(DESCENT_DATA_DIR)
    set_target_properties(descent PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${DESCENT_DATA_DIR}")
endif()

# descent is a console program: the console shows the debug output (the
# old mono screen, see source/bios/mono.c) while SDL opens the game window.
# Put SDL2.dll (and libADLMIDI's DLL, if shared) next to it so it runs from
# the build directory.
foreach(_dll_target SDL2::SDL2 libADLMIDI::ADLMIDI_shared)
    if(WIN32 AND TARGET ${_dll_target})
        get_target_property(_dll_type ${_dll_target} TYPE)
        if(_dll_type STREQUAL "SHARED_LIBRARY")
            add_custom_command(TARGET descent POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${_dll_target}> $<TARGET_FILE_DIR:descent>)
        endif()
    endif()
endforeach()

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
