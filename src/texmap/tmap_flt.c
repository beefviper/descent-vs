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
 * Flat shaded scanline, solid color or through a color translation table.
 * C port of tmap_flt.asm.
 */

#include <stdint.h>
#include <string.h>

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
//	tmap_flat_color		color to draw with, or
//	tmap_flat_cthru_table	if not NULL, table to translate the existing pixels through
void asm_tmap_scanline_flat(void)
{
	ubyte	*dest;
	int	xleft, xright, count;

	if ((unsigned) fx_y > (unsigned) window_bottom)
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

	count = xright - fx_xleft;		// the unclipped left x, as in the assembly
	if (count < 0)
		return;
	if ((unsigned) count > (unsigned) window_width)
		count = window_width - 1;

	if (tmap_flat_cthru_table == NULL) {
		// solid color: count+1 pixels
		memset(dest, tmap_flat_color, count + 1);
	} else {
		// color through: count pixels
		for (; count > 0; count--, dest++)
			*dest = tmap_flat_cthru_table[*dest];
	}
}
