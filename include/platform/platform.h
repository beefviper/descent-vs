/*
 * platform.h - the window, display and input backend (SDL2).
 *
 * The DOS game drew into VGA memory and read the keyboard from an
 * interrupt handler. This backend gives it a window instead: the screen
 * is an 8-bit framebuffer in ordinary memory, shown through the palette
 * the game loads into the (emulated) VGA DAC, and SDL key events are fed
 * to the keyboard code as DOS scancodes.
 */

#ifndef _PLATFORM_H
#define _PLATFORM_H

// Opens the window if needed and allocates a w x h screen. The buffer
// holds `pages` screens stacked vertically, for code that flips between
// pages. Returns the framebuffer (rowsize w), or NULL on failure.
unsigned char *plat_video_set_mode(int w, int h, int pages);

// Closes the window.
void plat_video_close(void);

// Shows the screen whose top-left pixel is at `start` in the framebuffer
// (page flipping).
void plat_video_show(unsigned char *start);

// Copies the framebuffer to the window now.
void plat_video_present(void);

// Waits for the next emulated vertical retrace (70 Hz) and presents.
void plat_video_sync(void);

// Sleeps for about `ms` milliseconds.
void plat_delay(int ms);

// Processes pending window and input events. Called from the timer and
// keyboard routines, which the game polls constantly; also presents the
// screen at about 60 Hz so drawing shows up without explicit flips.
void plat_pump_events(void);

// Receives key presses and releases as DOS keycodes (set 1 scancodes,
// with 0x80 added for the E0-prefixed keys; see key.h).
typedef void (*plat_key_handler)(unsigned char keycode, int down);
void plat_set_key_handler(plat_key_handler handler);

// Receives mouse button presses and releases: 0 left, 1 right, 2 middle.
typedef void (*plat_mouse_button_handler)(int button, int down);
void plat_set_mouse_button_handler(plat_mouse_button_handler handler);

// Mouse motion since the last call, in mouse units (like the DOS driver's
// mickeys). While the game keeps reading it (flying with the mouse, the
// editor), the window captures the mouse; it lets go when the reads stop
// (menus, pause) or the window loses focus.
void plat_mouse_get_delta(int *dx, int *dy);

// Where the mouse is over the game screen, in screen pixels.
void plat_mouse_get_pos(int *x, int *y);

// Sound effects. Samples are 8-bit unsigned mono and must stay in memory
// while they play. Volume is 0..0x7fff, with 0x4000 playing a sample at
// its own level; pan is 0 (left) .. 0x8000 (middle) .. 0xffff (right).
#define PLAT_AUDIO_VOICES	32

// Opens the audio device. Returns 1 on success, 0 if there is no sound.
int plat_audio_init(void);
void plat_audio_close(void);

// Starts a sample on voice `voice` (-1 = any free voice), replacing what
// it was playing. Returns the voice, or -1 if none is free.
int plat_audio_start(int voice, const unsigned char *data, int length, int rate,
	int volume, int pan, int loop);
void plat_audio_stop(int voice);
int plat_audio_playing(int voice);
void plat_audio_set_volume(int voice, int volume);
void plat_audio_set_pan(int voice, int pan);

// Music: HMI songs (.hmp, .hmq) or standard MIDI files, played on an
// emulated OPL3 FM synth and mixed in with the sound effects. Needs the
// audio device open. Returns 1 if this build has music.
int plat_music_available(void);

// Starts a song from the file's data (which may be freed afterwards),
// replacing any song playing. `bank` is the melodic bank file the song
// was written for (melodic.bnk, intmelo.bnk, ...) and picks the matching
// FM instruments. Returns 1 on success.
int plat_music_play(const void *data, int length, const char *bank, int loop);
void plat_music_stop(void);

// Music volume, 0..127.
void plat_music_set_volume(int volume);

#endif
