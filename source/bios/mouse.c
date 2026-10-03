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
 * $Source: f:/miner/source/bios/rcs/mouse.c $
 * $Revision: 1.11 $
 * $Author: john $
 * $Date: 1995/02/10 18:52:17 $
 *
 * Functions to access Mouse and Cyberman...
 *
 * $Log: mouse.c $
 * Revision 1.11  1995/02/10  18:52:17  john
 * Fixed bug with mouse not getting closed.
 *
 * Revision 1.10  1995/02/02  11:10:33  john
 * Changed a bunch of mouse stuff around to maybe get
 * around PS/2 mouse hang.
 *
 * Revision 1.9  1995/01/14  19:19:52  john
 * Fixed signed short error cmp with -1 that caused mouse
 * to break under Watcom 10.0
 *
 * Revision 1.8  1994/12/27  12:38:23  john
 * Made mouse use temporary dos buffer instead of
 *
 * allocating its own.
 *
 *
 * Revision 1.7  1994/12/05  23:54:53  john
 * Fixed bug with mouse_get_delta only returning positive numbers..
 *
 * Revision 1.6  1994/11/18  23:18:18  john
 * Changed some shorts to ints.
 *
 * Revision 1.5  1994/09/13  12:34:02  john
 * Added functions to get down count and state.
 *
 * Revision 1.4  1994/08/29  20:52:19  john
 * Added better cyberman support; also, joystick calibration
 * value return funcctiionn,
 *
 * Revision 1.3  1994/08/24  18:54:32  john
 * *** empty log message ***
 *
 * Revision 1.2  1994/08/24  18:53:46  john
 * Made Cyberman read like normal mouse; added dpmi module; moved
 * mouse from assembly to c. Made mouse buttons return time_down.
 *
 * Revision 1.1  1994/08/24  13:56:37  john
 * Initial revision
 *
 *
 */



#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "fix.h"
#include "mouse.h"
#include "timer.h"
#include "platform.h"

// The DOS version installed an int 33h event handler. Mouse events now come
// from SDL (source/platform/sdl.c): button presses through
// mouse_button_event(), the same bookkeeping the handler did, and motion
// through plat_mouse_get_delta(). There is no Cyberman.

#define MOUSE_MAX_BUTTONS	11

typedef struct mouse_info {
	fix		ctime;
	int		num_buttons;
	ubyte		pressed[MOUSE_MAX_BUTTONS];
	fix		time_went_down[MOUSE_MAX_BUTTONS];
	fix		time_held_down[MOUSE_MAX_BUTTONS];
	uint		num_downs[MOUSE_MAX_BUTTONS];
	uint		num_ups[MOUSE_MAX_BUTTONS];
} mouse_info;

static mouse_info Mouse;

static int Mouse_installed = 0;

static void mouse_button_event(int button, int down)
{
	if (button < 0 || button >= MOUSE_MAX_BUTTONS)
		return;

	Mouse.ctime = timer_get_fixed_secondsX();

	if (down)	{
		if (!Mouse.pressed[button])	{
			Mouse.pressed[button] = 1;
			Mouse.time_went_down[button] = Mouse.ctime;
		}
		Mouse.num_downs[button]++;
	} else {
		if (Mouse.pressed[button])	{
			Mouse.pressed[button] = 0;
			Mouse.time_held_down[button] += Mouse.ctime-Mouse.time_went_down[button];
		}
		Mouse.num_ups[button]++;
	}
}

//--------------------------------------------------------
// returns 0 if no mouse
//           else number of buttons
int mouse_init(int enable_cyberman)
{
	(void)enable_cyberman;

	if (Mouse_installed)
		return Mouse.num_buttons;

	Mouse.num_buttons = 3;
	plat_set_mouse_button_handler( mouse_button_event );

	Mouse_installed = 1;

	atexit( mouse_close );

	mouse_flush();

	return Mouse.num_buttons;
}


void mouse_close(void)
{
	if (Mouse_installed)	{
		Mouse_installed = 0;
		plat_set_mouse_button_handler( NULL );
	}
}


void mouse_set_limits( int x1, int y1, int x2, int y2 )
{
	// The pointer limits only mattered to the DOS driver's own cursor.
	(void)x1; (void)y1; (void)x2; (void)y2;
}

void mouse_get_pos( int *x, int *y)
{
	if (!Mouse_installed) {
		*x = *y = 0;
		return;
	}
	plat_mouse_get_pos( x, y );
}

void mouse_get_delta( int *dx, int *dy )
{
	if (!Mouse_installed) {
		*dx = *dy = 0;
		return;
	}
	plat_mouse_get_delta( dx, dy );
}

int mouse_get_btns(void)
{
	int i;
	uint flag=1;
	int status = 0;

	if (!Mouse_installed)
		return 0;

	plat_pump_events();

	for (i=0; i<MOUSE_MAX_BUTTONS; i++ )	{
		if (Mouse.pressed[i])
			status |= flag;
		flag <<= 1;
	}
	return status;
}

void mouse_set_pos( int x, int y)
{
	// The game draws its own pointer from the motion counters.
	(void)x; (void)y;
}

void mouse_flush(void)
{
	int i;
	fix CurTime;

	if (!Mouse_installed)
		return;

	//Clear the mouse data
	CurTime =timer_get_fixed_secondsX();
	for (i=0; i<MOUSE_MAX_BUTTONS; i++ )	{
		Mouse.pressed[i] = 0;
		Mouse.time_went_down[i] = CurTime;
		Mouse.time_held_down[i] = 0;
		Mouse.num_downs[i]=0;
		Mouse.num_ups[i]=0;
	}
}


// Returns how many times this button has went down since last call.
int mouse_button_down_count(int button)
{
	int count;

	if (!Mouse_installed || button < 0 || button >= MOUSE_MAX_BUTTONS)
		return 0;

	plat_pump_events();

	count = Mouse.num_downs[button];
	Mouse.num_downs[button]=0;

	return count;
}

// Returns 1 if this button is currently down
int mouse_button_state(int button)
{
	if (!Mouse_installed || button < 0 || button >= MOUSE_MAX_BUTTONS)
		return 0;

	plat_pump_events();

	return Mouse.pressed[button];
}


// Returns how long this button has been down since last call.
fix mouse_button_down_time(int button)
{
	fix time_down, time;

	if (!Mouse_installed || button < 0 || button >= MOUSE_MAX_BUTTONS)
		return 0;

	plat_pump_events();

	if ( !Mouse.pressed[button] )	{
		time_down = Mouse.time_held_down[button];
		Mouse.time_held_down[button] = 0;
	} else	{
		time = timer_get_fixed_secondsX();
		time_down =  time - Mouse.time_went_down[button];
		Mouse.time_went_down[button] = time;
	}

	return time_down;
}

void mouse_get_cyberman_pos( int *x, int *y )
{
	*x = *y = 0;
}
