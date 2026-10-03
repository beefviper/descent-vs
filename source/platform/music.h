/*
 * music.h - glue between the sound effect mixer (audio.c) and the music
 * synth (music.c). Not for the game; it uses platform.h.
 */

#ifndef _MUSIC_H
#define _MUSIC_H

// audio.c: hold off the mixer while the song state changes.
void audio_lock(void);
void audio_unlock(void);

// music.c: called by audio.c with the device open/closed at `rate` Hz.
void music_open(int rate);
void music_close(void);

// music.c: writes `frames` stereo samples of music (silence when nothing
// plays). Called from the mixer with the device locked.
void music_render(short *out, int frames);

#endif
