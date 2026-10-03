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
 * VGA Mode X routines.
 * C port of modex.asm.
 *
 * The assembly programmed the VGA sequencer and CRT controller registers and
 * drew into the planar memory at A0000h.  That hardware is not available on
 * Windows, so these are harmless stubs that do nothing (gr_modex_setmode
 * still returns the mode's dimensions).  A Windows display backend will
 * replace them.
 */

#include "gr.h"
#include "grdef.h"
#include "platform.h"

// Width and height of the Mode X modes, from tweak.inc.
static const short modex_dims[][2] = {
	{ 320, 200 },
	{ 320, 240 },
	{ 360, 200 },
	{ 360, 240 },
	{ 376, 282 },
	{ 320, 400 },
	{ 320, 480 },
	{ 360, 400 },
	{ 360, 480 },
	{ 360, 360 },
	{ 376, 308 },
	{ 376, 564 },
};

#define LAST_X_MODE	11

// Waits for the start of the (emulated) vertical retrace and shows the
// screen. Palette fades step once per call.
void gr_sync_display()
{
	plat_video_sync();
}

// Drew a horizontal line from x1 to x2 at y in planar video memory.
void gr_modex_uscanline( short x1, short x2, short y, unsigned char color )
{
	(void)x1; (void)x2; (void)y; (void)color;
}

// Sets Mode X mode number 'mode' (1 based).  Returns width<<16 | height.
int gr_modex_setmode( short mode )
{
	unsigned int i = (unsigned int)(mode - 1);

	if ( i > LAST_X_MODE )
		i = 0;

	return (modex_dims[i][0] << 16) | modex_dims[i][1];
}

// Selected the plane for reading and writing.
void gr_modex_setplane( short plane )
{
	(void)plane;
}

// Set the display start address, optionally waiting for the vertical retrace.
void gr_modex_setstart( short x, short y, int wait_for_retrace )
{
	(void)x; (void)y; (void)wait_for_retrace;
}

// Drew a line from (modex_line_x1,modex_line_y1) to (modex_line_x2,modex_line_y2)
// in modex_line_Color.
void gr_modex_line()
{
}
