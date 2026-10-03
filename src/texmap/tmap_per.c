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
 * Perspective texture mapper inner loop.
 * C port of tmap_per.asm.
 *
 * Divides for u/z and v/z are done once every 2^NBITS pixels and the texture
 * coordinates are interpolated linearly in between, using the packed 6:10,6:10
 * V:U format of the assembly (a carry out of u spills into v, as it did there).
 */

#include <stdint.h>

#include "fix.h"
#include "gr.h"
#include "grdef.h"
#include "texmap.h"
#include "texmapl.h"
#include "tmap_inc.h"

#define	NBITS		4		// 2^NBITS pixels plotted per divide
#define	ZSHIFT	4		// precision used in PDIV
#define	NPIXS		(1 << NBITS)

int	scan_doubling_flag = 0;
int	linear_if_far_flag = 0;

//	Normal perspective texture map loop: one pair of divides per pixel.
//	Draws loop_count+1 pixels.  Lighting is applied before the transparency
//	check, as in the assembly.
static void tmap_per_loop(ubyte *dest, int loop_count, int32_t u, int32_t v, int32_t z)
{
	uint	c;

	do {
		c = pixptr[TMAP_DIV_INDEX(u, v, z)];

		if (Lighting_on) {
			c = TMAP_LIGHT(fx_l, c);
			fx_l = ADD32(fx_l, fx_dl_dx);
		}

		if (c != 255)
			*dest = c;
		dest++;

		v = ADD32(v, fx_dv_dx);
		u = ADD32(u, fx_du_dx);
		z = ADD32(z, fx_dz_dx);
		if (z == 0)
			return;			// would be dividing by 0, so abort
	} while (--loop_count >= 0);
}

//	Lighted version, 2^NBITS pixels per divide.
static void tmap_per_fast(ubyte *dest, int loop_count, int32_t u, int32_t v, int32_t z)
{
	int		num_left_over, i;
	int32_t	U0, V0, U1, V1, DU1, DV1, DZ1, l, dl;
	uint32_t	uv, duv;
	uint		c, lidx;

	// Getting dword aligned
	while ((uintptr_t) dest & 3) {
		c = pixptr[TMAP_DIV_INDEX(u, v, z)];
		lidx = fx_l & 0xff00;
		fx_l = ADD32(fx_l, fx_dl_dx);
		if (c != 255)		// this pixel is transparent, so don't write it (or light it)
			*dest = gr_fade_table[lidx | c];
		dest++;

		v = ADD32(v, fx_dv_dx);
		u = ADD32(u, fx_du_dx);
		z = ADD32(z, fx_dz_dx);
		if (z == 0)
			return;
		if (--loop_count < 0)
			return;
	}

	num_left_over = (loop_count + 1) & (NPIXS - 1);
	if (((loop_count + 1) >> NBITS) == 0) {
		tmap_per_loop(dest, loop_count, u, v, z);	// no 2^NBITS chunks, do divide/pixel for whole scanline
		return;
	}
	loop_count = (loop_count + 1) >> NBITS;

	V0 = tmap_pdiv(v, z, ZSHIFT);
	U0 = tmap_pdiv(u, z, ZSHIFT);

	DU1 = (int32_t) ((uint32_t) fx_du_dx << NBITS);
	DV1 = (int32_t) ((uint32_t) fx_dv_dx << NBITS);
	DZ1 = (int32_t) ((uint32_t) fx_dz_dx << NBITS);

	do {
		u = ADD32(u, DU1);
		v = ADD32(v, DV1);
		z = ADD32(z, DZ1);
		if (z == 0)
			return;

		U1 = tmap_pdiv(u, z, ZSHIFT);
		V1 = tmap_pdiv(v, z, ZSHIFT);

		uv = tmap_pack_uv(U0, V0);
		duv = tmap_pack_duv(ADD32(U1, -U0), ADD32(V1, -V0), NBITS);

		// Save U1 and V1 so we don't have to divide on the next iteration
		U0 = U1;
		V0 = V1;

		l = fx_l;
		dl = fx_dl_dx;

		if (Transparency_on) {
			// lighting value only advances on drawn pixels
			for (i = 0; i < NPIXS; i++) {
				c = pixptr[TMAP_UV_INDEX(uv)];
				uv += duv;
				if (c != 255) {
					*dest = TMAP_LIGHT(l, c);
					l = ADD32(l, dl);
				}
				dest++;
			}
		} else {
			for (i = 0; i < NPIXS; i++) {
				c = pixptr[TMAP_UV_INDEX(uv)];
				uv += duv;
				*dest++ = TMAP_LIGHT(l, c);
				l = ADD32(l, dl);
			}
		}

		fx_l = l;
	} while (--loop_count != 0);

	if (num_left_over == 0)
		return;

	// Leftover pixels.  Use the subdivided method only if z does not change
	// too much over the remaining span, else do a divide per pixel.  (The
	// assembly had a new_end flag, always 1, to select the divide per pixel
	// method, "new, but slower, way of finishing off extra pixels on
	// scanline, 01/10/95 --MK"; the old method only remains for this case.)
	{
		int32_t	z_end = ADD32(z, DZ1);
		int		ok_here = 0;

		if (z_end >= 0)
			ok_here = (int32_t) ((uint32_t) z * 3) < (int32_t) ((uint32_t) z_end << 2);

		if (!ok_here) {
			// NewDoEndPixels
			do {
				c = pixptr[TMAP_DIV_INDEX(u, v, z)];
				lidx = fx_l & 0xff00;
				fx_l = ADD32(fx_l, fx_dl_dx);
				if (c != 255)
					*dest = gr_fade_table[lidx | c];
				dest++;

				v = ADD32(v, fx_dv_dx);
				u = ADD32(u, fx_du_dx);
				z = ADD32(z, fx_dz_dx);
				if (z == 0)
					return;
			} while (--num_left_over != 0);
			return;
		}
	}

	u = ADD32(u, DU1);
	v = ADD32(v, DV1);
	z = ADD32(z, DZ1);
	if (z == 0)
		return;
	// (The assembly backed off DU1/DV1/DZ1 in steps here if z went negative.
	// That cannot happen: the test above only gets here with z+DZ1 >= 0.)

	if (z <= (1 << (ZSHIFT+1)))
		z = 1 << (ZSHIFT+1);

	U1 = tmap_pdiv(u, z, ZSHIFT);
	V1 = tmap_pdiv(v, z, ZSHIFT);

	uv = tmap_pack_uv(U0, V0);
	duv = tmap_pack_duv(ADD32(U1, -U0), ADD32(V1, -V0), NBITS);

	l = fx_l;
	dl = fx_dl_dx;

	for (i = 0; i < NPIXS; i++) {
		c = pixptr[TMAP_UV_INDEX(uv)];
		uv += duv;
		lidx = l & 0xff00;
		l = ADD32(l, dl);
		if (c != 255)
			dest[i] = gr_fade_table[lidx | c];
		if (--num_left_over == 0)
			return;
	}
}

//	Unlighted version, 2^NBITS pixels per divide.
static void tmap_per_fast_nolight(ubyte *dest, int loop_count, int32_t u, int32_t v, int32_t z)
{
	int		num_left_over, i, j;
	int32_t	U0, V0, U1, V1, DU1, DV1, DZ1;
	uint32_t	uv, duv;
	uint		c;
	ubyte		pix[4];

	// Getting dword aligned
	while ((uintptr_t) dest & 3) {
		c = pixptr[TMAP_DIV_INDEX(u, v, z)];
		if (c != 255)
			*dest = c;
		dest++;

		v = ADD32(v, fx_dv_dx);
		u = ADD32(u, fx_du_dx);
		z = ADD32(z, fx_dz_dx);
		if (z == 0)
			return;
		if (--loop_count < 0)
			return;
	}

	num_left_over = loop_count + 1;
	if (((unsigned) num_left_over >> NBITS) == 0) {
		tmap_per_loop(dest, loop_count, u, v, z);	// no 2^NBITS chunks, do divide/pixel for whole scanline
		return;
	}
	loop_count = (unsigned) num_left_over >> NBITS;
	num_left_over -= loop_count << NBITS;

	V0 = tmap_pdiv(v, z, ZSHIFT);
	U0 = tmap_pdiv(u, z, ZSHIFT);

	DU1 = (int32_t) ((uint32_t) fx_du_dx << NBITS);
	DV1 = (int32_t) ((uint32_t) fx_dv_dx << NBITS);
	DZ1 = (int32_t) ((uint32_t) fx_dz_dx << NBITS);

	do {
		u = ADD32(u, DU1);
		v = ADD32(v, DV1);
		z = ADD32(z, DZ1);
		if (z == 0)
			return;

		U1 = tmap_pdiv(u, z, ZSHIFT);
		V1 = tmap_pdiv(v, z, ZSHIFT);

		uv = tmap_pack_uv(U0, V0);
		duv = tmap_pack_duv(ADD32(U1, -U0), ADD32(V1, -V0), NBITS);

		U0 = U1;
		V0 = V1;

		for (j = 0; j < NPIXS/4; j++) {
			for (i = 0; i < 4; i++) {
				pix[i] = pixptr[TMAP_UV_INDEX(uv)];
				uv += duv;
			}

			if (Transparency_on) {
				for (i = 0; i < 4; i++)
					if (pix[i] != 255)
						dest[i] = pix[i];
			} else {
				dest[0] = pix[0];
				dest[1] = pix[1];
				dest[2] = pix[2];
				dest[3] = pix[3];
			}
			dest += 4;
		}
	} while (--loop_count != 0);

	if (num_left_over == 0)
		return;

	// Leftover pixels
	u = ADD32(u, DU1);
	v = ADD32(v, DV1);
	z = ADD32(z, DZ1);
	if (z == 0)
		return;

	U1 = tmap_pdiv(u, z, ZSHIFT);
	V1 = tmap_pdiv(v, z, ZSHIFT);

	uv = tmap_pack_uv(U0, V0);
	duv = tmap_pack_duv(ADD32(U1, -U0), ADD32(V1, -V0), NBITS);

	for (i = 0; i < NPIXS; i++) {
		c = pixptr[TMAP_UV_INDEX(uv)];
		uv += duv;
		if (c != 255)
			dest[i] = c;
		if (--num_left_over == 0)
			return;
	}
}

// --------------------------------------------------------------------------------------------------
// Enter:
//	fx_xleft	left x coordinate (int)
//	fx_xright	right x coordinate (int)
//	fx_y		y coordinate (int)
//	pixptr	address of source pixel map
//	fx_u		fixed point initial u coordinate
//	fx_v		fixed point initial v coordinate
//	fx_z		fixed point initial z coordinate
//	fx_du_dx	fixed point du/dx
//	fx_dv_dx	fixed point dv/dx
//	fx_dz_dx	fixed point dz/dx
//	fx_l, fx_dl_dx	fixed point lighting value and delta
//
//   for (x = (int) xleft; x <= (int) xright; x++) {
//      _setcolor(read_pixel_from_tmap(srcb,((int) (u/z)) & 63,((int) (v/z)) & 63));
//      _setpixel(x,y);
//
//      u += du_dx;
//      v += dv_dx;
//      z += dz_dx;
//   }
void asm_tmap_scanline_per(void)
{
	ubyte	*dest;
	int	xleft, loop_count;

	xleft = fx_xleft;
	if (xleft < 0)
		xleft = 0;
	dest = write_buffer + y_pointers[fx_y] + xleft;

	loop_count = fx_xright - xleft;
	if (loop_count < 0)
		return;

	// lighting values are passed in fixed point, but need to be in 8 bit integer, 8 bit fraction so we can easily
	// get the integer by reading bits 8..15
	fx_l >>= 8;
	fx_dl_dx >>= 8;
	if (fx_dl_dx < 0)
		fx_dl_dx++;		// round towards 0 for negative deltas

	if (!per2_flag)
		tmap_per_loop(dest, loop_count, fx_u, fx_v, fx_z);
	else if (!Lighting_on)
		tmap_per_fast_nolight(dest, loop_count, fx_u, fx_v, fx_z);
	else
		tmap_per_fast(dest, loop_count, fx_u, fx_v, fx_z);
}
