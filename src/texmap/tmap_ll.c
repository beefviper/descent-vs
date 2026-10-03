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
 * Linear (affine) texture mapper inner loop with intensity lighting.
 * C port of tmap_ll.asm.
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
//	fx_l		fixed point lighting value
//	fx_dl_dx	fixed point dl/dx
//
//	The assembly also computed alternating dithered lighting deltas (1/2 above
//	and below fx_dl_dx, when dither_intensity_lighting was set), but the loop it
//	actually assembled (loop_test = 1) did not use them, so they are omitted.
void asm_tmap_scanline_lin_lighted(void)
{
	ubyte		*dest;
	int		xleft, xright, loop_count;
	uint32_t	uv, duv;
	int32_t	l, dl;
	uint		c;

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

	loop_count = xright - xleft;
	if (loop_count < 0)
		return;
	if ((unsigned) loop_count > (unsigned) window_width)
		loop_count = window_width;

	// u, v and their deltas packed in 6:10,6:10 int:frac format
	uv = tmap_pack_uv(fx_u, fx_v);
	duv = tmap_pack_uv(fx_du_dx, fx_dv_dx);

	fx_dl_dx >>= 8;
	if (fx_dl_dx < 0)
		fx_dl_dx++;		// round towards 0 for negative deltas

	// lighting values are passed in fixed point, but need to be in 8 bit integer, 8 bit fraction so we can easily
	// get the integer by reading bits 8..15
	l = fx_l >> 8;
	dl = fx_dl_dx;

	if (!Transparency_on) {
		for (loop_count++; loop_count > 0; loop_count--) {
			c = pixptr[TMAP_UV_INDEX(uv)];
			uv += duv;
			*dest++ = TMAP_LIGHT(l, c);
			l = ADD32(l, dl);
		}
	} else {
		// lighting value only advances on drawn pixels
		for (loop_count++; loop_count > 0; loop_count--) {
			c = pixptr[TMAP_UV_INDEX(uv)];
			uv += duv;
			if (c != 255) {
				*dest = TMAP_LIGHT(l, c);
				l = ADD32(l, dl);
			}
			dest++;
		}
	}
}
