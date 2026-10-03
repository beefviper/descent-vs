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

#endif
