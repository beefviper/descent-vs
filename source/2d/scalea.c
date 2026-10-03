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
 * Run length slice bitmap scanline stretcher, used to scale bitmaps up.
 * C port of scalea.asm.
 *
 * The assembly scaled with self-modifying code: rls_do_cc_setup_asm patched
 * an unrolled loop in scale_do_cc_scanline so that each middle source pixel
 * was drawn scale_whole_step or scale_whole_step+1 times.  Here the setup
 * stores those run lengths in a table instead.
 */

#include "gr.h"
#include "grdef.h"

#define MAX_CC_PIXELS	320		// size of the unrolled loop in the assembly

char scale_trans_color;
int scale_error_term;
int scale_initial_pixel_count;
int scale_adj_up;
int scale_adj_down;
int scale_final_pixel_count;
int scale_ydelta_minus_1;
int scale_whole_step;
ubyte * scale_source_ptr;
ubyte * scale_dest_ptr;

static int scale_cc_count;							// number of middle pixels
static int scale_cc_runs[MAX_CC_PIXELS];		// run length of each middle pixel

// Draws a run of count pixels of color c, unless c is transparent; returns
// the pointer past the run.
static ubyte *scale_run( ubyte *dest, int count, ubyte c )
{
	int i;

	if ( c != (ubyte)scale_trans_color )
		for (i=0; i<count; i++ )
			dest[i] = c;
	return dest + count;
}

// Stretches the scanline at scale_source_ptr to scale_dest_ptr, using the
// run length slice parameters set by rls_stretch_scanline_setup.
void rls_stretch_scanline_asm(void)
{
	ubyte *src = scale_source_ptr;
	ubyte *dest = scale_dest_ptr;
	int error_term = scale_error_term;
	int n = scale_ydelta_minus_1;
	int count;

	dest = scale_run( dest, scale_initial_pixel_count, *src++ );

	do {
		count = scale_whole_step;
		error_term += scale_adj_up;
		if ( error_term > 0 )	{
			count++;
			error_term -= scale_adj_down;
		}
		dest = scale_run( dest, count, *src++ );
	} while ( --n > 0 );

	scale_run( dest, scale_final_pixel_count, *src );
}

// Precomputes the run length of each middle pixel for scale_do_cc_scanline.
void rls_do_cc_setup_asm(void)
{
	int error_term = scale_error_term;
	int i;

	scale_cc_count = scale_ydelta_minus_1;
	if ( scale_cc_count > MAX_CC_PIXELS )
		scale_cc_count = MAX_CC_PIXELS;
	if ( scale_cc_count < 0 )
		scale_cc_count = 0;

	for (i=0; i<scale_cc_count; i++ )	{
		error_term += scale_adj_up;
		if ( error_term > 0 )	{
			scale_cc_runs[i] = scale_whole_step + 1;
			error_term -= scale_adj_down;
		} else
			scale_cc_runs[i] = scale_whole_step;
	}
}

// Stretches the scanline at scale_source_ptr to scale_dest_ptr using the run
// lengths computed by rls_do_cc_setup_asm.
void scale_do_cc_scanline(void)
{
	ubyte *src = scale_source_ptr;
	ubyte *dest = scale_dest_ptr;
	int i;

	// Do the first texture pixel
	dest = scale_run( dest, scale_initial_pixel_count, *src++ );

	// The middle pixels
	for (i=0; i<scale_cc_count; i++ )
		dest = scale_run( dest, scale_cc_runs[i], *src++ );

	scale_run( dest, scale_final_pixel_count, *src );
}
