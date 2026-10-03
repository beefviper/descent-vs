/*
 * sdl.c - window, display and keyboard backend on SDL2. See platform.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SDL's headers pull in the compiler intrinsics, which declare functions
// that the force-included compat.h (via dos.h) defines as macros.
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

#include "vga.h"
#include "key.h"
#include "platform.h"

#define RETRACE_HZ		70		// VGA mode 13h refresh rate
#define PRESENT_MS		16		// present at about 60 Hz from plat_pump_events

static int sdl_ready;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static unsigned char *framebuffer;
static unsigned char *shown;			// top-left of the page on display
static int screen_w, screen_h;
static int texture_w, texture_h;

static Uint32 palette[256];
static unsigned palette_version = (unsigned)-1;

static Uint64 last_present;
static Uint64 next_retrace;
static int in_pump;

static plat_key_handler key_handler;

static void sdl_shutdown(void)
{
	plat_video_close();
	SDL_Quit();
}

static int sdl_init(void)
{
	if (sdl_ready)
		return 1;

	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return 0;
	}
	sdl_ready = 1;
	atexit(sdl_shutdown);
	return 1;
}

static int create_window(void)
{
	if (window)
		return 1;

	window = SDL_CreateWindow("Descent", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		960, 720, SDL_WINDOW_RESIZABLE);
	if (!window) {
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		window = NULL;
		return 0;
	}
	return 1;
}

unsigned char *plat_video_set_mode(int w, int h, int pages)
{
	if (!sdl_init() || !create_window())
		return NULL;

	if (!texture || texture_w != w || texture_h != h) {
		if (texture)
			SDL_DestroyTexture(texture);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, w, h);
		if (!texture) {
			fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
			return NULL;
		}
		texture_w = w;
		texture_h = h;
	}

	free(framebuffer);
	// One spare row: some code addresses its second page one row down.
	framebuffer = (unsigned char *)calloc((size_t)w * (h * pages + 1), 1);
	if (!framebuffer)
		return NULL;

	shown = framebuffer;
	screen_w = w;
	screen_h = h;
	plat_video_present();
	return framebuffer;
}

void plat_video_close(void)
{
	if (texture)
		SDL_DestroyTexture(texture);
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
	texture = NULL;
	renderer = NULL;
	window = NULL;
	free(framebuffer);
	framebuffer = shown = NULL;
}

void plat_video_show(unsigned char *start)
{
	ptrdiff_t offset;

	if (!framebuffer)
		return;

	// Only whole rows can be shown; start may point into a sub-canvas.
	offset = start - framebuffer;
	if (offset < 0)
		offset = 0;
	shown = framebuffer + (offset / screen_w) * screen_w;
	plat_video_present();
}

static void update_palette(void)
{
	int i;
	const unsigned char *dac = compat_vga_dac;

	if (palette_version == compat_vga_dac_version)
		return;
	palette_version = compat_vga_dac_version;

	for (i = 0; i < 256; i++, dac += 3) {
		Uint32 r = (dac[0] << 2) | (dac[0] >> 4);	// 6-bit DAC to 8-bit
		Uint32 g = (dac[1] << 2) | (dac[1] >> 4);
		Uint32 b = (dac[2] << 2) | (dac[2] >> 4);
		palette[i] = 0xff000000u | (r << 16) | (g << 8) | b;
	}
}

// The largest 4:3 rectangle that fits the window: every mode the game
// used was shown on a 4:3 monitor, whatever its pixel count.
static void display_rect(SDL_Rect *rect)
{
	int ww, wh;

	SDL_GetRendererOutputSize(renderer, &ww, &wh);
	if (ww * 3 > wh * 4) {
		rect->h = wh;
		rect->w = wh * 4 / 3;
	} else {
		rect->w = ww;
		rect->h = ww * 3 / 4;
	}
	rect->x = (ww - rect->w) / 2;
	rect->y = (wh - rect->h) / 2;
}

void plat_video_present(void)
{
	void *pixels;
	int pitch, x, y;
	SDL_Rect rect;

	if (!texture || !shown)
		return;

	update_palette();

	if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0) {
		const unsigned char *src = shown;
		for (y = 0; y < screen_h; y++) {
			Uint32 *dest = (Uint32 *)((Uint8 *)pixels + y * pitch);
			for (x = 0; x < screen_w; x++)
				dest[x] = palette[*src++];
		}
		SDL_UnlockTexture(texture);
	}

	display_rect(&rect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_RenderPresent(renderer);

	last_present = SDL_GetPerformanceCounter();
}

void plat_video_sync(void)
{
	Uint64 freq = SDL_GetPerformanceFrequency();
	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 period = freq / RETRACE_HZ;

	if (!sdl_ready)
		return;

	if (next_retrace <= now || next_retrace > now + 2 * period)
		next_retrace = now;		// fell behind, or first call
	while (now < next_retrace) {
		Uint64 left_ms = (next_retrace - now) * 1000 / freq;
		if (left_ms > 1)
			SDL_Delay((Uint32)(left_ms - 1));
		now = SDL_GetPerformanceCounter();
	}
	next_retrace += period;

	plat_video_present();
	plat_pump_events();
}

// SDL scancodes to DOS keycodes. 0 = no equivalent.
static unsigned char dos_keycode(SDL_Scancode sc)
{
	switch (sc) {
	case SDL_SCANCODE_ESCAPE:		return KEY_ESC;
	case SDL_SCANCODE_1:			return KEY_1;
	case SDL_SCANCODE_2:			return KEY_2;
	case SDL_SCANCODE_3:			return KEY_3;
	case SDL_SCANCODE_4:			return KEY_4;
	case SDL_SCANCODE_5:			return KEY_5;
	case SDL_SCANCODE_6:			return KEY_6;
	case SDL_SCANCODE_7:			return KEY_7;
	case SDL_SCANCODE_8:			return KEY_8;
	case SDL_SCANCODE_9:			return KEY_9;
	case SDL_SCANCODE_0:			return KEY_0;
	case SDL_SCANCODE_MINUS:		return KEY_MINUS;
	case SDL_SCANCODE_EQUALS:		return KEY_EQUAL;
	case SDL_SCANCODE_BACKSPACE:	return KEY_BACKSP;
	case SDL_SCANCODE_TAB:			return KEY_TAB;
	case SDL_SCANCODE_Q:			return KEY_Q;
	case SDL_SCANCODE_W:			return KEY_W;
	case SDL_SCANCODE_E:			return KEY_E;
	case SDL_SCANCODE_R:			return KEY_R;
	case SDL_SCANCODE_T:			return KEY_T;
	case SDL_SCANCODE_Y:			return KEY_Y;
	case SDL_SCANCODE_U:			return KEY_U;
	case SDL_SCANCODE_I:			return KEY_I;
	case SDL_SCANCODE_O:			return KEY_O;
	case SDL_SCANCODE_P:			return KEY_P;
	case SDL_SCANCODE_LEFTBRACKET:	return KEY_LBRACKET;
	case SDL_SCANCODE_RIGHTBRACKET:	return KEY_RBRACKET;
	case SDL_SCANCODE_RETURN:		return KEY_ENTER;
	case SDL_SCANCODE_LCTRL:		return KEY_LCTRL;
	case SDL_SCANCODE_A:			return KEY_A;
	case SDL_SCANCODE_S:			return KEY_S;
	case SDL_SCANCODE_D:			return KEY_D;
	case SDL_SCANCODE_F:			return KEY_F;
	case SDL_SCANCODE_G:			return KEY_G;
	case SDL_SCANCODE_H:			return KEY_H;
	case SDL_SCANCODE_J:			return KEY_J;
	case SDL_SCANCODE_K:			return KEY_K;
	case SDL_SCANCODE_L:			return KEY_L;
	case SDL_SCANCODE_SEMICOLON:	return KEY_SEMICOL;
	case SDL_SCANCODE_APOSTROPHE:	return KEY_RAPOSTRO;
	case SDL_SCANCODE_GRAVE:		return KEY_LAPOSTRO;
	case SDL_SCANCODE_LSHIFT:		return KEY_LSHIFT;
	case SDL_SCANCODE_BACKSLASH:	return KEY_SLASH;
	case SDL_SCANCODE_Z:			return KEY_Z;
	case SDL_SCANCODE_X:			return KEY_X;
	case SDL_SCANCODE_C:			return KEY_C;
	case SDL_SCANCODE_V:			return KEY_V;
	case SDL_SCANCODE_B:			return KEY_B;
	case SDL_SCANCODE_N:			return KEY_N;
	case SDL_SCANCODE_M:			return KEY_M;
	case SDL_SCANCODE_COMMA:		return KEY_COMMA;
	case SDL_SCANCODE_PERIOD:		return KEY_PERIOD;
	case SDL_SCANCODE_SLASH:		return KEY_DIVIDE;
	case SDL_SCANCODE_RSHIFT:		return KEY_RSHIFT;
	case SDL_SCANCODE_KP_MULTIPLY:	return KEY_PADMULTIPLY;
	case SDL_SCANCODE_LALT:			return KEY_LALT;
	case SDL_SCANCODE_SPACE:		return KEY_SPACEBAR;
	case SDL_SCANCODE_CAPSLOCK:		return KEY_CAPSLOCK;
	case SDL_SCANCODE_F1:			return KEY_F1;
	case SDL_SCANCODE_F2:			return KEY_F2;
	case SDL_SCANCODE_F3:			return KEY_F3;
	case SDL_SCANCODE_F4:			return KEY_F4;
	case SDL_SCANCODE_F5:			return KEY_F5;
	case SDL_SCANCODE_F6:			return KEY_F6;
	case SDL_SCANCODE_F7:			return KEY_F7;
	case SDL_SCANCODE_F8:			return KEY_F8;
	case SDL_SCANCODE_F9:			return KEY_F9;
	case SDL_SCANCODE_F10:			return KEY_F10;
	case SDL_SCANCODE_F11:			return KEY_F11;
	case SDL_SCANCODE_F12:			return KEY_F12;
	case SDL_SCANCODE_NUMLOCKCLEAR:	return KEY_NUMLOCK;
	case SDL_SCANCODE_SCROLLLOCK:	return KEY_SCROLLOCK;
	case SDL_SCANCODE_KP_7:			return KEY_PAD7;
	case SDL_SCANCODE_KP_8:			return KEY_PAD8;
	case SDL_SCANCODE_KP_9:			return KEY_PAD9;
	case SDL_SCANCODE_KP_MINUS:		return KEY_PADMINUS;
	case SDL_SCANCODE_KP_4:			return KEY_PAD4;
	case SDL_SCANCODE_KP_5:			return KEY_PAD5;
	case SDL_SCANCODE_KP_6:			return KEY_PAD6;
	case SDL_SCANCODE_KP_PLUS:		return KEY_PADPLUS;
	case SDL_SCANCODE_KP_1:			return KEY_PAD1;
	case SDL_SCANCODE_KP_2:			return KEY_PAD2;
	case SDL_SCANCODE_KP_3:			return KEY_PAD3;
	case SDL_SCANCODE_KP_0:			return KEY_PAD0;
	case SDL_SCANCODE_KP_PERIOD:	return KEY_PADPERIOD;
	case SDL_SCANCODE_KP_ENTER:		return KEY_PADENTER;
	case SDL_SCANCODE_RCTRL:		return KEY_RCTRL;
	case SDL_SCANCODE_KP_DIVIDE:	return KEY_PADDIVIDE;
	case SDL_SCANCODE_PRINTSCREEN:	return KEY_PRINT_SCREEN;
	case SDL_SCANCODE_RALT:			return KEY_RALT;
	case SDL_SCANCODE_PAUSE:		return KEY_PAUSE;
	case SDL_SCANCODE_HOME:			return KEY_HOME;
	case SDL_SCANCODE_UP:			return KEY_UP;
	case SDL_SCANCODE_PAGEUP:		return KEY_PAGEUP;
	case SDL_SCANCODE_LEFT:			return KEY_LEFT;
	case SDL_SCANCODE_RIGHT:		return KEY_RIGHT;
	case SDL_SCANCODE_END:			return KEY_END;
	case SDL_SCANCODE_DOWN:			return KEY_DOWN;
	case SDL_SCANCODE_PAGEDOWN:		return KEY_PAGEDOWN;
	case SDL_SCANCODE_INSERT:		return KEY_INSERT;
	case SDL_SCANCODE_DELETE:		return KEY_DELETE;
	default:						return 0;
	}
}

void plat_set_key_handler(plat_key_handler handler)
{
	key_handler = handler;
}

void plat_pump_events(void)
{
	SDL_Event event;
	Uint64 now;

	if (!sdl_ready || in_pump)
		return;
	in_pump = 1;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP: {
			unsigned char keycode = dos_keycode(event.key.keysym.scancode);
			if (keycode && key_handler)
				key_handler(keycode, event.type == SDL_KEYDOWN);
			break;
		}
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
				event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				last_present = 0;	// redraw now
			break;
		}
	}

	now = SDL_GetPerformanceCounter();
	if (now - last_present >= SDL_GetPerformanceFrequency() * PRESENT_MS / 1000)
		plat_video_present();

	in_pump = 0;
}
