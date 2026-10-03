/*
 * audio.c - sound effect mixer on SDL2 audio. See platform.h.
 *
 * Stands in for the digital half of the SOS sound library: a fixed set of
 * voices, each playing an 8-bit unsigned mono sample with its own volume
 * and pan, mixed into a 16-bit stereo stream along with the music
 * (music.c).
 */

#include <stdio.h>
#include <string.h>

#undef _disable
#undef _enable
#undef inp
#undef inpw
#undef outp
#undef outpw
#define SDL_MAIN_HANDLED		// the game keeps its own main()
COMPAT_PACK_DEFAULT_BEGIN	// see compat.h
#include <SDL.h>
COMPAT_PACK_DEFAULT_END

#include "platform.h"
#include "music.h"

#define OUTPUT_RATE		44100
#define FULL_VOLUME		16384		// volume that plays a sample at its own level
#define MIX_CHUNK		1024		// frames of music rendered at a time

typedef struct voice {
	const unsigned char *data;
	Uint32 length;					// in samples
	Uint32 pos;						// 16.16 position in samples
	Uint32 step;					// 16.16 advance per output sample
	int volume, pan;
	int left, right;				// gains, FULL_VOLUME = 1.0
	int loop;
	int playing;
} voice;

static SDL_AudioDeviceID device;
static int output_rate;
static voice voices[PLAT_AUDIO_VOICES];
static short music_buf[MIX_CHUNK * 2];

static void set_gains(voice *v, int volume, int pan)
{
	// pan 0 is full left, 0x8000 the middle, 0xffff full right. Both sides
	// stay at full volume in the middle and fade out toward the far side.
	int l, r;

	if (volume < 0) volume = 0;
	if (pan < 0) pan = 0;
	if (pan > 0xffff) pan = 0xffff;

	l = (0xffff - pan) * 2;
	r = pan * 2;
	if (l > 0xffff) l = 0xffff;
	if (r > 0xffff) r = 0xffff;

	v->volume = volume;
	v->pan = pan;
	v->left = (int)(((Sint64)volume * l) / 0xffff);
	v->right = (int)(((Sint64)volume * r) / 0xffff);
}

static void mix_chunk(Sint16 *out, int frames)
{
	int i, n;

	music_render(music_buf, frames);

	for (i = 0; i < frames; i++) {
		int l = music_buf[i * 2], r = music_buf[i * 2 + 1];

		for (n = 0; n < PLAT_AUDIO_VOICES; n++) {
			voice *v = &voices[n];
			Uint32 idx, frac;
			int s0, s1, s;

			if (!v->playing)
				continue;

			idx = v->pos >> 16;
			if (idx >= v->length) {
				if (v->loop && v->length) {
					v->pos -= v->length << 16;
					idx = v->pos >> 16;
				} else {
					v->playing = 0;
					continue;
				}
			}

			// linear interpolation between neighbouring samples
			frac = v->pos & 0xffff;
			s0 = (int)v->data[idx] - 128;
			if (idx + 1 < v->length)
				s1 = (int)v->data[idx + 1] - 128;
			else
				s1 = v->loop ? (int)v->data[0] - 128 : s0;
			s = s0 * 256 + (((s1 - s0) * 256 * (int)frac) >> 16);

			l += (s * v->left) / FULL_VOLUME;
			r += (s * v->right) / FULL_VOLUME;

			v->pos += v->step;
		}

		if (l > 32767) l = 32767; else if (l < -32768) l = -32768;
		if (r > 32767) r = 32767; else if (r < -32768) r = -32768;
		out[i * 2] = (Sint16)l;
		out[i * 2 + 1] = (Sint16)r;
	}
}

static void SDLCALL mix(void *userdata, Uint8 *stream, int len)
{
	Sint16 *out = (Sint16 *)stream;
	int frames = len / 4;

	(void)userdata;

	while (frames > 0) {
		int n = frames < MIX_CHUNK ? frames : MIX_CHUNK;
		mix_chunk(out, n);
		out += n * 2;
		frames -= n;
	}
}

void audio_lock(void)
{
	if (device)
		SDL_LockAudioDevice(device);
}

void audio_unlock(void)
{
	if (device)
		SDL_UnlockAudioDevice(device);
}

int plat_audio_init(void)
{
	SDL_AudioSpec want, have;

	if (device)
		return 1;

	SDL_SetMainReady();
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "SDL audio init failed: %s\n", SDL_GetError());
		return 0;
	}

	SDL_zero(want);
	want.freq = OUTPUT_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 1024;
	want.callback = mix;

	device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (!device) {
		fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return 0;
	}
	output_rate = have.freq;
	memset(voices, 0, sizeof(voices));
	music_open(output_rate);
	SDL_PauseAudioDevice(device, 0);
	return 1;
}

void plat_audio_close(void)
{
	if (!device)
		return;
	SDL_CloseAudioDevice(device);
	device = 0;
	music_close();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int plat_audio_start(int voice_num, const unsigned char *data, int length, int rate,
	int volume, int pan, int loop)
{
	voice *v;

	if (!device || !data || length <= 0)
		return -1;

	SDL_LockAudioDevice(device);
	if (voice_num < 0) {
		for (voice_num = 0; voice_num < PLAT_AUDIO_VOICES; voice_num++)
			if (!voices[voice_num].playing)
				break;
	}
	if (voice_num >= PLAT_AUDIO_VOICES) {
		SDL_UnlockAudioDevice(device);
		return -1;
	}
	v = &voices[voice_num];
	v->data = data;
	v->length = (Uint32)length;
	v->pos = 0;
	v->step = (Uint32)(((Uint64)rate << 16) / (Uint32)output_rate);
	v->loop = loop;
	set_gains(v, volume, pan);
	v->playing = 1;
	SDL_UnlockAudioDevice(device);

	return voice_num;
}

void plat_audio_stop(int voice_num)
{
	if (!device || voice_num < 0 || voice_num >= PLAT_AUDIO_VOICES)
		return;
	SDL_LockAudioDevice(device);
	voices[voice_num].playing = 0;
	SDL_UnlockAudioDevice(device);
}

int plat_audio_playing(int voice_num)
{
	if (!device || voice_num < 0 || voice_num >= PLAT_AUDIO_VOICES)
		return 0;
	return voices[voice_num].playing;
}

void plat_audio_set_volume(int voice_num, int volume)
{
	if (!device || voice_num < 0 || voice_num >= PLAT_AUDIO_VOICES)
		return;
	SDL_LockAudioDevice(device);
	set_gains(&voices[voice_num], volume, voices[voice_num].pan);
	SDL_UnlockAudioDevice(device);
}

void plat_audio_set_pan(int voice_num, int pan)
{
	if (!device || voice_num < 0 || voice_num >= PLAT_AUDIO_VOICES)
		return;
	SDL_LockAudioDevice(device);
	set_gains(&voices[voice_num], voices[voice_num].volume, pan);
	SDL_UnlockAudioDevice(device);
}
