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
 * Editor texture map "matt" scanline: plots color 1 at every non-transparent
 * pixel of a perspective texture mapped scanline (used to find which texture
 * is under the mouse).
 * C port of tmap_fl1.asm.
 *
 * This is the perspective mapper of tmap_per.asm with the lighting and the
 * texel writes taken out, divides every 2^NBITS = 8 pixels.  Its quirks are
 * kept: the alignment loop of the lighted version ignores transparency, and in
 * the unlighted transparent case, a transparent pixel ends the drawing of its
 * group of 4.
 */

#include <stdint.h>

#include "fix.h"
#include "gr.h"
#include "grdef.h"
#include "texmap.h"
#include "texmapl.h"
#include "tmap_inc.h"

#define	NBITS		3
#define	ZSHIFT	3
#define	NPIXS		(1 << NBITS)

#define	ZONK		1		// color plotted

//	Normal perspective loop, divide per pixel; draws loop_count+1 pixels.
static void matt_loop(ubyte *dest, int loop_count, int32_t u, int32_t v, int32_t z)
{
	do {
		if (pixptr[TMAP_DIV_INDEX(u, v, z)] != 255)
			*dest = ZONK;
		dest++;

		v = ADD32(v, fx_dv_dx);
		u = ADD32(u, fx_du_dx);
		z = ADD32(z, fx_dz_dx);
		if (z == 0)
			return;			// would be dividing by 0, so abort
	} while (--loop_count >= 0);
}

static void matt_fast(ubyte *dest, int loop_count, int32_t u, int32_t v, int32_t z)
{
	int		num_left_over, i;
	int32_t	U0, V0, U1, V1, DU1, DV1, DZ1;
	uint32_t	uv, duv;

	// Getting dword aligned (no transparency check here)
	while ((uintptr_t) dest & 3) {
		*dest++ = ZONK;

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
		matt_loop(dest, loop_count, u, v, z);
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

		if (Transparency_on) {
			for (i = 0; i < NPIXS; i++) {
				if (pixptr[TMAP_UV_INDEX(uv)] != 255)
					*dest = ZONK;
				uv += duv;
				dest++;
			}
		} else {
			for (i = 0; i < NPIXS; i++)
				*dest++ = ZONK;
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

	if (z <= 10)
		z = 10;

	U1 = tmap_pdiv(u, z, ZSHIFT);
	V1 = tmap_pdiv(v, z, ZSHIFT);

	uv = tmap_pack_uv(U0, V0);
	duv = tmap_pack_duv(ADD32(U1, -U0), ADD32(V1, -V0), NBITS);

	for (i = 0; i < NPIXS; i++) {
		if (pixptr[TMAP_UV_INDEX(uv)] != 255)
			dest[i] = ZONK;
		uv += duv;
		if (--num_left_over == 0)
			return;
	}
}

static void matt_fast_nolight(ubyte *dest, int loop_count, int32_t u, int32_t v, int32_t z)
{
	int		num_left_over, i, j;
	int32_t	U0, V0, U1, V1, DU1, DV1, DZ1;
	uint32_t	uv, duv;
	ubyte		pix[4];

	// Getting dword aligned
	while ((uintptr_t) dest & 3) {
		if (pixptr[TMAP_DIV_INDEX(u, v, z)] != 255)
			*dest = ZONK;
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
		matt_loop(dest, loop_count, u, v, z);
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
				// The assembly only advanced to the next pixel of the group
				// after writing one, so the first transparent pixel stops it.
				for (i = 0; i < 4 && pix[i] != 255; i++)
					dest[i] = ZONK;
			} else {
				dest[0] = dest[1] = dest[2] = dest[3] = ZONK;
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
		if (pixptr[TMAP_UV_INDEX(uv)] != 255)
			dest[i] = ZONK;
		uv += duv;
		if (--num_left_over == 0)
			return;
	}
}

// --------------------------------------------------------------------------------------------------
// Enter:
//	fx_xleft, fx_xright, fx_y	scanline (ints, not clipped)
//	pixptr	address of source pixel map
//	fx_u, fx_v, fx_z, fx_du_dx, fx_dv_dx, fx_dz_dx
void asm_tmap_scanline_matt(void)
{
	ubyte	*dest;
	int	loop_count;

	dest = write_buffer + y_pointers[fx_y] + fx_xleft;

	loop_count = fx_xright - fx_xleft;
	if (loop_count < 0)
		return;

	// lighting values to 8.8 format (unused here, but the globals are changed
	// as the assembly did)
	fx_l >>= 8;
	fx_dl_dx >>= 8;
	if (fx_dl_dx < 0)
		fx_dl_dx++;

	if (!per2_flag)
		matt_loop(dest, loop_count, fx_u, fx_v, fx_z);
	else if (!Lighting_on)
		matt_fast_nolight(dest, loop_count, fx_u, fx_v, fx_z);
	else
		matt_fast(dest, loop_count, fx_u, fx_v, fx_z);
}
