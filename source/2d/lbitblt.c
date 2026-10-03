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
 * Linear bitmap copy and texture merging.
 * C port of lbitblt.asm.
 */

#include <string.h>

#include "gr.h"
#include "grdef.h"

// Copies a height by width block from source's data to dest's data.
// (The assembly aligned the destination of rows 16 or more pixels wide to
// a dword boundary before its rep movsd.)
void gr_lbitblt( grs_bitmap * source, grs_bitmap * dest, int height, int width )
{
	ubyte *sbits = source->bm_data;
	ubyte *dbits = dest->bm_data;

	for (; height > 0; height-- )	{
		memcpy( dbits, sbits, width );
		sbits += (unsigned short)source->bm_rowsize;
		dbits += (unsigned short)dest->bm_rowsize;
	}
}

// Merges two 64x64 textures: dest gets the upper texture's pixel, or the
// lower texture's where the upper one is transparent (255).  The variants
// rotate the upper texture.
//
// case 0:
//    for (y=0; y<64; y++ )
//       for (x=0; x<64; x++ )   {
//          c = top_data[ 64*y+x ];
//          if (c==255)
//             c = bottom_data[ 64*y+x ];
//          *dest_data++ = c;
//       }
void gr_merge_textures( ubyte * lower, ubyte * upper, ubyte * dest )
{
	int i;

	for (i=0; i<64*64; i++ )
		dest[i] = (upper[i] != 255) ? upper[i] : lower[i];
}

// case 1:
//          c = top_data[ 64*x+(63-y) ];
void gr_merge_textures_1( ubyte * lower, ubyte * upper, ubyte * dest )
{
	int x, y;
	ubyte c;

	for (y=0; y<64; y++ )
		for (x=0; x<64; x++ )	{
			c = upper[ 64*x+(63-y) ];
			if (c==255)
				c = *lower;
			lower++;
			*dest++ = c;
		}
}

// case 2:
//          c = top_data[ 64*(63-y)+(63-x) ];
void gr_merge_textures_2( ubyte * lower, ubyte * upper, ubyte * dest )
{
	int i;
	ubyte c;

	for (i=0; i<64*64; i++ )	{
		c = upper[ 64*64-1-i ];
		if (c==255)
			c = lower[i];
		dest[i] = c;
	}
}

// case 3:
//          c = top_data[ 64*(63-x)+y  ];
void gr_merge_textures_3( ubyte * lower, ubyte * upper, ubyte * dest )
{
	int x, y;
	ubyte c;

	for (y=0; y<64; y++ )
		for (x=0; x<64; x++ )	{
			c = upper[ 64*(63-x)+y ];
			if (c==255)
				c = *lower;
			lower++;
			*dest++ = c;
		}
}
