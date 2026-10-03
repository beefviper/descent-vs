/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/
/*
 * Routines for setting and using system timers.
 *
 * C port of timer.asm. The original reprogrammed PIT channel 0 and hooked
 * interrupt 8; here the 1.19 MHz PIT pulse count is derived from the
 * system's high resolution counter (QueryPerformanceCounter on Windows,
 * clock() elsewhere). There is no timer interrupt, so the user function
 * and joystick poller are recorded but never called.
 */

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#undef _disable			// defined by the force-included compat.h
#undef _enable
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "types.h"
#include "fix.h"
#include "timer.h"
#include "platform.h"

static struct {
	int		_timer_cnt;			// PIT count per timer tick (65536 = 18.2 Hz)
	void		(*joystick_poller)();
	void		*user_function;
	int		Installed;
	uint64_t	base;					// counter value at timer_init/timer_set_rate
} TimerData = { 65536, NULL, NULL, 0, 0 };

#ifdef _WIN32
static uint64_t counter_freq;

static uint64_t read_counter(void)
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (uint64_t)c.QuadPart;
}

static void counter_init(void)
{
	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);
	counter_freq = (uint64_t)f.QuadPart;
}
#else
#define counter_freq ((uint64_t)CLOCKS_PER_SEC)

static uint64_t read_counter(void)
{
	return (uint64_t)clock();
}

static void counter_init(void)
{
}
#endif

// Return a 64-bit stamp that is the number of 1.19Mhz pulses
// since the time was initialized.
static uint64_t timer_get_stamp64(void)
{
	uint64_t elapsed;

	// The PIT always runs; start counting on first use if timer_init has
	// not been called yet.
	if (!TimerData.Installed)
		timer_init();

	elapsed = read_counter() - TimerData.base;

	// Split the conversion so the multiply cannot overflow
	return (elapsed / counter_freq) * TIMER_FREQUENCY +
			 (elapsed % counter_freq) * TIMER_FREQUENCY / counter_freq;
}

// Convert pulses to fixed point (16.16) seconds. The original subtracted
// out multiples of 1193180 from the high dword to prevent divide overflow,
// making time wrap at about 9 hours; that equals the 32-bit truncation.
static fix pulses_to_fix(uint64_t pulses)
{
	return (fix)(uint32_t)((pulses << 16) / TIMER_FREQUENCY);
}

// The game polls the clock constantly, in its main loop and in every
// wait loop, so these also give the window a chance to process events.
// The X version is called while handling key events and must not.
fix timer_get_fixed_seconds()
{
	plat_pump_events();
	return pulses_to_fix(timer_get_stamp64());
}

fix timer_get_fixed_secondsX()
{
	return pulses_to_fix(timer_get_stamp64());
}

// Returns the time at the last timer tick (tick_count * _timer_cnt pulses),
// so it is accurate only to one timer period.
fix timer_get_approx_seconds()
{
	uint64_t pulses = timer_get_stamp64();

	plat_pump_events();
	pulses -= pulses % (uint64_t)TimerData._timer_cnt;
	return pulses_to_fix(pulses);
}

// Replacement for the BIOS ticker at 0040:006C (18.2 Hz ticks); see the
// TICKER macro in timer.h.
int timer_get_bios_ticker()
{
	plat_pump_events();
	return (int)(timer_get_stamp64() >> 16);
}

void timer_set_rate(int count_val)
{
	// Make sure count_val is below or equal to 65535 and above 0;
	// if it's not, make it be 65536, which is normal DOS timing.
	if ((unsigned)count_val > 65535 || count_val == 0)
		count_val = 65536;

	// Setting the rate resets the tick count (and so the time) to 0.
	TimerData._timer_cnt = count_val;
	TimerData.base = read_counter();
}

void timer_set_function( void _far * function )
{
	TimerData.user_function = function;
}

void timer_set_joyhandler( void (*joy_handler)() )
{
	TimerData.joystick_poller = joy_handler;
}

void timer_close()
{
	TimerData.Installed = 0;
}

void timer_init()
{
	static int atexit_called = 0;

	if (TimerData.Installed)
		return;

	counter_init();
	TimerData._timer_cnt = 65536;		// Set to BIOS's normal 18.2 Hz
	TimerData.user_function = NULL;
	TimerData.base = read_counter();
	TimerData.Installed = 1;
	if (!atexit_called) {
		atexit_called = 1;
		atexit(timer_close);
	}
}
