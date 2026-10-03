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
 * Private declarations shared by the files of the 3d library.
 * Replaces the parts of 3d.inc that the C code needs.
 */

#ifndef _3DLOCAL_H
#define _3DLOCAL_H

#include <stdint.h>

#include "3d.h"

//from clipper.c
extern int free_point_num;

//clips a line to the viewing pyramid. takes pointers to the two point
//pointers, which are replaced by the clipped points (one or both new)
void clip_line(g3s_point **p0,g3s_point **p1,ubyte codes_or);

//3d clip a polygon.  src and dest are lists of point pointers, which must
//have room for two extra entries.  takes nv=number of verts & cc=codes,
//and updates both.  returns the list of clipped points, which is either
//src or dest.
g3s_point **clip_polygon(g3s_point **src,g3s_point **dest,int *nv,g3s_codes *cc);

//free a temporary point
void free_temp_point(g3s_point *p);

//Check for overflow before doing a divide (the divcheck macro from 3d.inc).
//num is the 64-bit numerator and den the denominator.  Returns true if the
//divide would overflow, in which case the caller must not do it.  This
//matches the asm test exactly: the high dword of the numerator is made
//positive, shifted left one with the top bit of the low dword, and compared
//unsigned against the denominator.
static __inline int divcheck(int64_t num,fix den)
{
	uint32_t hi = (uint32_t) (num >> 32);
	uint32_t lo = (uint32_t) num;

	if ((int32_t) hi < 0)
		hi = 0u - hi;

	return ((hi << 1) | (lo >> 31)) >= (uint32_t) den;
}

#endif
