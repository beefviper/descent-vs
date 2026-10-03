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
 * Routines to access linear VGA memory.
 * C port of linear.asm.
 */

#include <stdint.h>
#include <string.h>

#include "gr.h"
#include "grdef.h"

unsigned int gr_var_color;
unsigned char * gr_var_bitmap;
unsigned int gr_var_bwidth;

// Fills a run of n pixels going right (dir = 1) or left (dir = -1), as
// rep stosb with the direction flag clear or set.  Returns the pointer past
// the run.
static ubyte *stos_run( ubyte *dest, int n, int dir, ubyte color )
{
	for (; n > 0; n-- )	{
		*dest = color;
		dest += dir;
	}
	return dest;
}

// Fast run length slice line drawing implementation for mode 0x13, the VGA's
// 320x200 256-color mode.
// Draws a line between the specified endpoints in color gr_var_color.
void gr_linear_line( int x0, int y0, int x1, int y1 )
{
	ubyte *dest;
	ubyte color = (ubyte)gr_var_color;
	int bwidth = (int)gr_var_bwidth;
	int t, xadvance, n;
	uint32_t xdelta, ydelta, whole_step, adj_up, adj_down, error_term, err, runs;
	uint32_t initial_count, final_count;

	// We'll draw top to bottom, to reduce the number of cases we have to handle,
	// and to make lines between the same endpoints always draw the same pixels.
	if ( y0 > y1 )	{
		t = y0; y0 = y1; y1 = t;
		t = x0; x0 = x1; x1 = t;
	}

	// Point dest to the first pixel to draw.
	dest = gr_var_bitmap + (uint32_t)y0 * gr_var_bwidth + x0;

	// Figure out how far we're going vertically (guaranteed to be positive).
	ydelta = (uint32_t)(y1 - y0);

	// Figure out whether we're going left or right, and how far we're going
	// horizontally. In the process, special-case vertical lines, for speed and
	// to avoid nasty boundary conditions and division by 0.
	if ( x1 == x0 )	{
		// vertical line
		n = (int)ydelta;
		do {
			*dest = color;
			dest += bwidth;
		} while ( --n >= 0 );
		return;
	}

	if ( x1 > x0 )	{
		xadvance = 1;
		xdelta = (uint32_t)(x1 - x0);
	} else {
		xadvance = -1;
		xdelta = (uint32_t)(x0 - x1);
	}

	// Special-case horizontal lines.
	if ( ydelta == 0 )	{
		if ( xadvance < 0 )
			dest -= xdelta;		// point to left end so we can go left to right
		memset( dest, color, xdelta + 1 );
		return;
	}

	// Special-case diagonal lines.
	if ( ydelta == xdelta )	{
		n = (int)ydelta;
		do {
			*dest = color;
			dest += bwidth + xadvance;
		} while ( --n >= 0 );
		return;
	}

	if ( xdelta >= ydelta )	{
		// X-major (more horizontal than vertical) line.
		// Right to left lines are drawn backwards.

		whole_step = xdelta / ydelta;		// minimum # of pixels in a run in this line
		// Error term adjust each time Y steps by 1; used to tell when one extra
		// pixel should be drawn as part of a run, to account for fractional
		// steps along the X axis per 1-pixel steps along Y
		adj_up = (xdelta % ydelta) * 2;
		// Error term adjust when the error term turns over, used to factor out
		// the X step made at that time
		adj_down = ydelta * 2;
		// Initial error term; reflects an initial step of 0.5 along the Y axis.
		error_term = (xdelta % ydelta) - adj_down;

		// The initial and last runs are partial, because Y advances only 0.5 for
		// these runs, rather than 1. Divide one full run, plus the initial pixel,
		// between the initial and last runs.
		initial_count = (whole_step >> 1) + 1;
		final_count = initial_count;

		// If the basic run length is even and there's no fractional advance, we have
		// one pixel that could go to either the initial or last partial run, which
		// we'll arbitrarily allocate to the last run.
		// If there is an odd number of pixels per run, we have one pixel that can't
		// be allocated to either the initial or last partial run, so we'll add 0.5 to
		// the error term so this pixel will be handled by the normal full-run loop.
		if ( whole_step & 1 )
			error_term += ydelta;
		else if ( adj_up == 0 )
			initial_count--;

		// Draw the first, partial run of pixels.
		dest = stos_run( dest, initial_count, xadvance, color );
		dest += bwidth;

		// Draw all full runs (ydelta-1 of them).
		if ( ydelta > 1 )	{
			err = error_term - 1;		// adjust error term by -1 so we can use carry test
			for ( runs = ydelta - 1; runs > 0; runs-- )	{
				n = whole_step;
				// advance the error term and add an extra pixel if it turns over (carry)
				if ( err + adj_up < err )	{
					n++;
					err = err + adj_up - adj_down;
				} else
					err += adj_up;
				dest = stos_run( dest, n, xadvance, color );
				dest += bwidth;
			}
		}

		// Draw the final run of pixels.
		stos_run( dest, final_count, xadvance, color );
	} else {
		// Y-major (more vertical than horizontal) line.

		whole_step = ydelta / xdelta;
		adj_up = (ydelta % xdelta) * 2;
		adj_down = xdelta * 2;
		error_term = (ydelta % xdelta) - adj_down;

		initial_count = (whole_step >> 1) + 1;
		final_count = initial_count;

		if ( whole_step & 1 )
			error_term += xdelta;
		else if ( adj_up == 0 )
			initial_count--;

		// Draw the first, partial run of pixels.
		n = initial_count;
		do {
			*dest = color;
			dest += bwidth;
		} while ( --n != 0 );
		dest += xadvance;

		// Draw all full runs (xdelta-1 of them).
		if ( xdelta > 1 )	{
			err = error_term - 1;
			for ( runs = xdelta - 1; runs > 0; runs-- )	{
				n = whole_step;
				if ( err + adj_up < err )	{
					n++;
					err = err + adj_up - adj_down;
				} else
					err += adj_up;
				do {
					*dest = color;
					dest += bwidth;
				} while ( --n != 0 );
				dest += xadvance;
			}
		}

		// Draw the final run of pixels.
		n = final_count;
		do {
			*dest = color;
			dest += bwidth;
		} while ( --n != 0 );
	}
}

// Fills nbytes bytes at dest with color.
void gr_linear_stosd( void * dest, unsigned char color, unsigned short nbytes )
{
	memset( dest, color, nbytes );
}

// Compares the latest source buffer, sbuf1, against an earlier one, sbuf2, a
// dword at a time, and copies the dwords that changed to dbuf.
// As in the assembly, a changed dword is stored one dword further on in dbuf
// than its offset in the source buffers, and the last dword compared is always
// stored.  (Not called anywhere.)
void gr_update_buffer( void * sbuf1, void * sbuf2, void * dbuf, int size )
{
	ubyte *src = sbuf1;
	ubyte *old = sbuf2;
	ubyte *dest = dbuf;
	uint32_t n = (uint32_t)size >> 2;
	uint32_t i = 0;

	if ( n == 0 )
		return;

	do {
		// repe cmpsd
		while ( n )	{
			n--;
			i++;
			if ( memcmp( src + (i-1)*4, old + (i-1)*4, 4 ) )
				break;
		}
		memcpy( dest + i*4, src + (i-1)*4, 4 );
	} while ( n );
}
