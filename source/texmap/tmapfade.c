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
 * Darkened (shaded) scanline: pixels already drawn are translated through
 * the fade table.
 * C port of tmapfade.asm.
 */

#include <stdint.h>

#include "fix.h"
#include "gr.h"
#include "grdef.h"
#include "texmap.h"
#include "texmapl.h"
#include "tmap_inc.h"

// --------------------------------------------------------------------------------------------------
// Enter:
//	fx_xleft	left x coordinate (int)
//	fx_xright	right x coordinate (int)
//	fx_y		y coordinate (int)
//	tmap_flat_shade_value	fade level to darken the pixels with (0..31)
void asm_tmap_scanline_shaded(void)
{
	ubyte	*dest, *fade;
	int	xleft, xright, loop_count;

	if ((unsigned) fx_y >= (unsigned) window_bottom)
		return;

	xleft = fx_xleft;
	if (xleft < 0)
		xleft = 0;
	dest = write_buffer + fx_y * bytes_per_row + xleft;

	xright = fx_xright;
	if (xright >= window_right)
		xright = window_right;
	if (xright <= window_left)
		xright = window_left;

	loop_count = xright - fx_xleft;	// the unclipped left x, as in the assembly
	if (loop_count < 0)
		return;
	if ((unsigned) loop_count > (unsigned) window_width)
		loop_count = window_width;

	fade = &gr_fade_table[(tmap_flat_shade_value & 31) << 8];

	// The assembly jumped into an unrolled loop of NUM_ITERS pixels, drawing
	// loop_count+1 pixels, but never more than NUM_ITERS.
	if (loop_count > NUM_ITERS-1)
		loop_count = NUM_ITERS-1;
	for (loop_count++; loop_count > 0; loop_count--, dest++)
		*dest = fade[*dest];
}
