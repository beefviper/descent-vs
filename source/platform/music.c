/*
 * music.c - music on libADLMIDI. See platform.h.
 *
 * Stands in for the MIDI half of the SOS sound library with its FM driver:
 * libADLMIDI reads the HMI songs directly and plays them on an emulated
 * OPL3 chip, using its built-in copies of Descent's FM instrument banks.
 * Builds without libADLMIDI (DESCENT_MUSIC off) have no music.
 */

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "music.h"

#ifdef DESCENT_HAVE_ADLMIDI

COMPAT_PACK_DEFAULT_BEGIN	// see compat.h
#include <adlmidi.h>
COMPAT_PACK_DEFAULT_END

static struct ADL_MIDIPlayer *player;
static int playing;
static int volume = 127;			// 0..127

// Descent's songs name the melodic bank they were written for
// (descent.sng). libADLMIDI has each of them built in.
static const struct {
	const char *prefix;				// bank file name starts with this
	const char *name;				// libADLMIDI bank name contains this
} bank_map[] = {
	{ "int",	"Descent:: Int" },
	{ "ham",	"Descent:: Ham" },
	{ "rick",	"Descent:: Rick" },
	{ "",		"HMI (Descent" },		// melodic.bnk
};

static int find_bank(const char *bank_file)
{
	const char *const *names = adl_getBankNames();
	int count = adl_getBanksCount();
	int i, n;

	for (i = 0; i < (int)(sizeof(bank_map) / sizeof(bank_map[0])); i++) {
		size_t len = strlen(bank_map[i].prefix);
		if (bank_file && _strnicmp(bank_file, bank_map[i].prefix, len) != 0)
			continue;
		for (n = 0; n < count; n++)
			if (names[n] && strstr(names[n], bank_map[i].name))
				return n;
	}
	return 0;
}

void music_open(int rate)
{
	if (player)
		return;
	player = adl_init(rate);
	if (!player) {
		fprintf(stderr, "Music disabled: %s\n", adl_errorString());
		return;
	}
	playing = 0;
}

void music_close(void)
{
	if (!player)
		return;
	adl_close(player);
	player = NULL;
	playing = 0;
}

void music_render(short *out, int frames)
{
	int got = 0, i;

	if (player && playing && volume > 0) {
		got = adl_play(player, frames * 2, out);
		if (got < 0)
			got = 0;
		if (adl_atEnd(player))
			playing = 0;
		if (volume < 127)
			for (i = 0; i < got; i++)
				out[i] = (short)(out[i] * volume / 127);
	}
	memset(out + got, 0, ((size_t)frames * 2 - (size_t)got) * sizeof(short));
}

int plat_music_available(void)
{
	return player != NULL;
}

int plat_music_play(const void *data, int length, const char *bank, int loop)
{
	int ok;

	if (!player || !data || length <= 0)
		return 0;

	audio_lock();
	playing = 0;
	adl_setBank(player, find_bank(bank));
	adl_setLoopEnabled(player, loop);
	ok = adl_openData(player, data, (unsigned long)length) == 0;
	if (ok)
		playing = 1;
	audio_unlock();

	if (!ok)
		fprintf(stderr, "Music: %s\n", adl_errorInfo(player));
	return ok;
}

void plat_music_stop(void)
{
	if (!player)
		return;
	audio_lock();
	playing = 0;
	adl_panic(player);
	audio_unlock();
}

void plat_music_set_volume(int vol)
{
	if (vol < 0) vol = 0;
	if (vol > 127) vol = 127;
	audio_lock();
	volume = vol;
	audio_unlock();
}

#else	// no music in this build

void music_open(int rate) { (void)rate; }
void music_close(void) {}
void music_render(short *out, int frames) { memset(out, 0, (size_t)frames * 2 * sizeof(short)); }
int plat_music_available(void) { return 0; }
int plat_music_play(const void *data, int length, const char *bank, int loop)
{
	(void)data; (void)length; (void)bank; (void)loop;
	return 0;
}
void plat_music_stop(void) {}
void plat_music_set_volume(int vol) { (void)vol; }

#endif
