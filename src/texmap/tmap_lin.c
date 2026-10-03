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
 * Linear (affine) texture mapper inner loop, no lighting.
 * C port of tmap_lin.asm.
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
//	pixptr	address of source pixel map
//	fx_u		fixed point initial u coordinate
//	fx_v		fixed point initial v coordinate
//	fx_du_dx	fixed point du/dx
//	fx_dv_dx	fixed point dv/dx
void asm_tmap_scanline_lin(void)
{
	ubyte		*dest;
	int		xleft, xright, loop_count;
	uint32_t	u, v, du, dv, idx;
	uint		c;

	if ((unsigned) fx_y > (unsigned) window_bottom)
		return;

	xleft = fx_xleft;
	if (xleft < 0)
		xleft = 0;
	dest = write_buffer + fx_y * bytes_per_row + xleft;

	// (unsigned compares, as in the assembly)
	xright = fx_xright;
	if (!((unsigned) xright < (unsigned) window_right))
		xright = window_right;
	if (!((unsigned) xright > (unsigned) window_left))
		xright = window_left;

	loop_count = xright - fx_xleft;		// the unclipped left x, as in the assembly
	if (loop_count < 0)
		return;
	if ((unsigned) loop_count > (unsigned) window_width)
		loop_count = window_width;

	// u and v in the top 6:10 bits, so the integer part of each is its top 6 bits
	u = (uint32_t) fx_u << 10;
	v = (uint32_t) fx_v << 10;
	du = (uint32_t) fx_du_dx << 10;
	dv = (uint32_t) fx_dv_dx << 10;

	if (!Transparency_on) {
		// The assembly jumped into an unrolled loop of NUM_ITERS pixels, drawing
		// loop_count+1 pixels, but never more than NUM_ITERS.
		if (loop_count > NUM_ITERS-1)
			loop_count = NUM_ITERS-1;
		for (loop_count++; loop_count > 0; loop_count--) {
			idx = ((v >> 26) << 6) | (u >> 26);
			v += dv;
			u += du;
			*dest++ = pixptr[idx];
		}
	} else {
		// if texture map has transparency, use this code (draws loop_count pixels)
		for (; loop_count > 0; loop_count--) {
			idx = ((v >> 26) << 6) | (u >> 26);
			v += dv;
			u += du;
			c = pixptr[idx];
			if (c != 255)
				*dest = c;
			dest++;
		}
	}
}
