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
 * Shared definitions for the C texture mapper scanline routines.
 * C counterpart of tmap_inc.asm, which held the externs and constants
 * shared by the assembly inner loops.
 */

#ifndef _TMAP_INC_H
#define _TMAP_INC_H

#include <stdint.h>

#include "gr.h"

#define	MAX_WINDOW_WIDTH	320
#define	NUM_ITERS			MAX_WINDOW_WIDTH		// size of the unrolled loops in the assembly

#define	MAX_Y_POINTERS		480

extern int	y_pointers[MAX_Y_POINTERS];	// byte offset of each row in write_buffer

//	32 bit wrapping add, as the registers in the assembly did.
#define	ADD32(a,b)	((int32_t) ((uint32_t) (a) + (uint32_t) (b)))

//	Signed 32/32 division (idiv with cdq).  The 64 bit intermediate avoids the
//	INT_MIN/-1 trap; the quotient is truncated to 32 bits.
static __inline int32_t tmap_div32(int32_t num, int32_t den)
{
	return (int32_t) ((int64_t) num / den);
}

//	PDIV macro of the perspective mappers: returns num/den in 16.16 format,
//	computed with zshift bits of fraction.  edx:eax = num << zshift, idiv den,
//	then shift the quotient up by 16-zshift.  The original would raise a divide
//	overflow exception if the quotient did not fit in 32 bits; here it is
//	truncated.
static __inline int32_t tmap_pdiv(int32_t num, int32_t den, int zshift)
{
	int64_t	q = ((int64_t) num * ((int64_t) 1 << zshift)) / den;
	return (int32_t) ((uint32_t) q << (16 - zshift));
}

//	Make a packed V:U value in 6:10,6:10 int:frac format from 16.16 u, v.
//	(esi = V0 << 10; si = U0 >> 6)
static __inline uint32_t tmap_pack_uv(int32_t u, int32_t v)
{
	return (((uint32_t) v << 10) & 0xffff0000u) | (((uint32_t) u >> 6) & 0xffffu);
}

//	Make a packed DV:DU value in 6:10,6:10 format for 2^nbits pixel steps.
//	(edx = (V1-V0) << (10-nbits); dx = (U1-U0) sar (nbits+6))
static __inline uint32_t tmap_pack_duv(int32_t du, int32_t dv, int nbits)
{
	return (((uint32_t) dv << (10 - nbits)) & 0xffff0000u) | ((uint32_t) (du >> (nbits + 6)) & 0xffffu);
}

//	Texel offset from a packed V:U value: int(v)*64 + int(u).
//	(eax = esi shr 26; shld ax,si,6)
#define	TMAP_UV_INDEX(uv)	((((uv) >> 26) << 6) | (((uv) >> 10) & 63))

//	Texel offset from per pixel divides: ((v/z) & 63) * 64 + ((u/z) & 63).
#define	TMAP_DIV_INDEX(u,v,z)	(((tmap_div32((v),(z)) & 63) << 6) | (tmap_div32((u),(z)) & 63))

//	Lighting table lookup: the integer part of an 8.8 lighting value selects
//	the fade table row.  (mov ah,bh; mov al,gr_fade_table[eax])
#define	TMAP_LIGHT(l,c)	(gr_fade_table[((l) & 0xff00) | (c)])

#endif
