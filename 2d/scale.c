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
 * $Source: f:/miner/source/2d/rcs/scale.c $
 * $Revision: 1.12 $
 * $Author: john $
 * $Date: 1995/03/14 15:14:11 $
 * 
 * Routines for scaling a bitmap.
 * 
 * $Log: scale.c $
 * Revision 1.12  1995/03/14  15:14:11  john
 * Increased max scanline length to 640.
 * ..
 * 
 * Revision 1.11  1994/11/27  12:56:39  matt
 * Took out unneeded include of 3d.h
 * 
 * Revision 1.10  1994/11/18  22:50:25  john
 * Changed shorts to ints in parameters.
 * 
 * Revision 1.9  1994/11/09  16:35:02  john
 * First version with working RLE bitmaps.
 * 
 * Revision 1.8  1994/06/09  13:15:17  john
 * *** empty log message ***
 * 
 * Revision 1.7  1994/06/07  11:47:02  john
 * Added back in the fast code for scaling up bitmaps.
 * 
 * Revision 1.6  1994/02/18  15:32:36  john
 * *** empty log message ***
 * 
 * Revision 1.5  1994/01/22  14:35:01  john
 * Added transparency as color index 255.
 * 
 * Revision 1.4  1994/01/17  16:59:12  john
 * once again...
 * 
 * Revision 1.3  1994/01/17  16:51:17  john
 * Added check so we don't draw outsibe
 * the source bitmap's v coordinate... kind
 * of a hack, but works.
 * 
 * Revision 1.2  1994/01/12  18:03:26  john
 * The first iteration of fast scaler..
 * 
 * Revision 1.1  1994/01/11  14:48:42  john
 * Initial revision
 * 
 * 
 */


#pragma off (unreferenced)
static char rcsid[] = "$Id: scale.c 1.12 1995/03/14 15:14:11 john Exp $";
#pragma on (unreferenced)

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "mono.h"
#include "fix.h"
#include "gr.h"
#include "error.h"
#include "rle.h"

#define TRANSPARENCY_COLOR 255;

static int Transparency_color = TRANSPARENCY_COLOR;

extern char scale_trans_color;
extern int scale_error_term;
extern int scale_initial_pixel_count;
extern int scale_adj_up;
extern int scale_adj_down;
extern int scale_final_pixel_count;
extern int scale_ydelta_minus_1;
extern int scale_whole_step;
extern ubyte * scale_source_ptr;
extern ubyte * scale_dest_ptr;

// Function Prototypes
void scale_bitmap_cc_asm_rle(grs_bitmap* source_bmp, grs_bitmap* dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0, fix u1, fix v1);
void scale_bitmap_asm_rle(grs_bitmap* source_bmp, grs_bitmap* dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0, fix u1, fix v1);
void scale_bitmap_cc_asm(grs_bitmap* source_bmp, grs_bitmap* dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0, fix u1, fix v1);
void scale_bitmap_asm(grs_bitmap* source_bmp, grs_bitmap* dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0, fix u1, fix v1);

extern void rls_stretch_scanline_asm();

extern void scale_do_cc_scanline();
extern void rls_do_cc_setup_asm();

void rls_stretch_scanline( char * source, char * dest, int XDelta, int YDelta );
void rls_stretch_scanline_setup( int XDelta, int YDelta );

void scale_row_c( ubyte * sbits, ubyte * dbits, int width, fix u, fix du )
{
	int i;
	ubyte c;

	for ( i=0; i<width; i++ )	{
		c = sbits[ f2i(u) ];

		if ( c != Transparency_color )
			*dbits = c;
			
		dbits++;
		u += du;
	}
}

// esi, edi = source, dest	
// ecx = width
// ebx = u
// edx = du

void scale_row_asm_transparent( ubyte * sbits, ubyte * dbits, int width, fix u, fix du );
#pragma aux scale_row_asm_transparent parm [esi] [edi] [ecx] [ebx] [edx] modify exact [edi eax ebx ecx] = \
"newpixel:	mov	eax, ebx			" \
"				shr	eax, 16			" \
"				mov	al, [esi+eax]	" \
"				cmp	al, 255			" \
"				je		skip_it			" \
"				mov	[edi], al		" \
"skip_it:	add	ebx, edx			" \
"				inc	edi				" \
"				dec	ecx				" \
"				jne	newpixel			"

void scale_row_asm( ubyte * sbits, ubyte * dbits, int width, fix u, fix du );
#pragma aux scale_row_asm parm [esi] [edi] [ecx] [ebx] [edx] modify exact [edi eax ebx ecx] = \
"newpixel1:	mov	eax, ebx			" \
"				shr	eax, 16			" \
"				mov	al, [esi+eax]	" \
"				add	ebx, edx			" \
"				mov	[edi], al		" \
"				inc	edi				" \
"				dec	ecx				" \
"				jne	newpixel1		"


void rep_movsb( ubyte * sbits, ubyte * dbits, int width );
#pragma aux rep_movsb parm [esi] [edi] [ecx] modify exact [esi edi ecx] = \
"rep movsb"

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (fixmuldiv((x)-(x0),(y1)-(y0),(x1)-(x0))+(y0))

// Scales bitmap, bp, into vertbuf[0] to vertbuf[1]
void scale_bitmap(grs_bitmap *bp, grs_point *vertbuf )
{
	grs_bitmap * dbp = &grd_curcanv->cv_bitmap;
	fix x0, y0, x1, y1;
	fix u0, v0, u1, v1;
	fix clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	fix clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	fix xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;
	int dtemp;
	// Set initial variables....

	x0 = vertbuf[0].x; y0 = vertbuf[0].y;
	x1 = vertbuf[2].x; y1 = vertbuf[2].y;

	xmin = 0; ymin = 0;
	xmax = i2f(dbp->bm_w)-fl2f(.5); ymax = i2f(dbp->bm_h)-fl2f(.5);

	u0 = i2f(0); v0 = i2f(0);
	u1 = i2f(bp->bm_w-1); v1 = i2f(bp->bm_h-1);

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<0 ) || (x0>=xmax) ) return;
	if ( (y1<0 ) || (y0>=ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = f2i(clipped_x0); dx1 = f2i(clipped_x1);
	dy0 = f2i(clipped_y0); dy1 = f2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	Assert( dx0>=0 );
	Assert( dy0>=0 );
	Assert( dx1<dbp->bm_w );
	Assert( dy1<dbp->bm_h );
	Assert( f2i(u0)<=f2i(u1) );
	Assert( f2i(v0)<=f2i(v1) );
	Assert( f2i(u0)>=0 );
	Assert( f2i(v0)>=0 );
	Assert( u1<i2f(bp->bm_w) );
	Assert( v1<i2f(bp->bm_h) );

	//mprintf( 0, "(%.2f,%.2f) to (%.2f,%.2f) using (%.2f,%.2f) to (%.2f,%.2f)\n", f2fl(clipped_x0), f2fl(clipped_y0), f2fl(clipped_x1), f2fl(clipped_y1), f2fl(clipped_u0), f2fl(clipped_v0), f2fl(clipped_u1), f2fl(clipped_v1) );

	dtemp = f2i(clipped_u1)-f2i(clipped_u0);

	if ( bp->bm_flags & BM_FLAG_RLE )	{
		if ( (dtemp < (f2i(clipped_x1)-f2i(clipped_x0))) && (dtemp>0) )
			scale_bitmap_cc_asm_rle(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
		else
			scale_bitmap_asm_rle(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
	} else {
		if ( (dtemp < (f2i(clipped_x1)-f2i(clipped_x0))) && (dtemp>0) )
			scale_bitmap_cc_asm(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
		else
			scale_bitmap_asm(bp, dbp, dx0, dy0, dx1, dy1, clipped_u0, clipped_v0, clipped_u1, clipped_v1  );
	}
}


void scale_bitmap_c(grs_bitmap *source_bmp, grs_bitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix u, v, du, dv;
	int x, y;
	ubyte * sbits, * dbits;

	du = (u1-u0) / (x1-x0);
	dv = (v1-v0) / (y1-y0);

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		sbits = &source_bmp->bm_data[source_bmp->bm_rowsize*f2i(v)];
		dbits = &dest_bmp->bm_data[dest_bmp->bm_rowsize*y+x0];
		u = u0; 
		v += dv;
		for (x=x0; x<=x1; x++ )			{
			*dbits++ = sbits[ u >> 16 ];
			u += du;
		}
	}
}

void scale_bitmap_asm(grs_bitmap *source_bmp, grs_bitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix du, dv, v;
	int y;

	du = (u1-u0) / (x1-x0);
	dv = (v1-v0) / (y1-y0);

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		scale_row_asm_transparent( &source_bmp->bm_data[source_bmp->bm_rowsize*f2i(v)], &dest_bmp->bm_data[dest_bmp->bm_rowsize*y+x0], x1-x0+1, u0, du );
		v += dv;
	}
}

ubyte scale_rle_data[640];

void decode_row( grs_bitmap * bmp, int y )
{
	int i, offset=4+bmp->bm_h;
	
	for (i=0; i<y; i++ )
		offset += bmp->bm_data[4+i];
	gr_rle_decode( &bmp->bm_data[offset], scale_rle_data );
}

void scale_bitmap_asm_rle(grs_bitmap *source_bmp, grs_bitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix du, dv, v;
	int y, last_row=-1;

	du = (u1-u0) / (x1-x0);
	dv = (v1-v0) / (y1-y0);

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		if ( f2i(v) != last_row )	{
			last_row = f2i(v);
			decode_row( source_bmp, last_row );
		}
		scale_row_asm_transparent( scale_rle_data, &dest_bmp->bm_data[dest_bmp->bm_rowsize*y+x0], x1-x0+1, u0, du );
		v += dv;
	}
}


void scale_bitmap_cc_asm(grs_bitmap *source_bmp, grs_bitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix dv, v;
	int y;

	dv = (v1-v0) / (y1-y0);
		
	rls_stretch_scanline_setup( (int)(x1-x0), f2i(u1)-f2i(u0) );
	if ( scale_ydelta_minus_1 < 1 ) return;
	rls_do_cc_setup_asm();

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		scale_source_ptr = &source_bmp->bm_data[source_bmp->bm_rowsize*f2i(v)+f2i(u0)];
		scale_dest_ptr = &dest_bmp->bm_data[dest_bmp->bm_rowsize*y+x0];
		scale_do_cc_scanline();
		v += dv;
	}
}

void scale_bitmap_cc_asm_rle(grs_bitmap *source_bmp, grs_bitmap *dest_bmp, int x0, int y0, int x1, int y1, fix u0, fix v0,  fix u1, fix v1  )
{
	fix dv, v;
	int y, last_row = -1;

	dv = (v1-v0) / (y1-y0);
		
	rls_stretch_scanline_setup( (int)(x1-x0), f2i(u1)-f2i(u0) );
	if ( scale_ydelta_minus_1 < 1 ) return;
	rls_do_cc_setup_asm();

	v = v0;

	for (y=y0; y<=y1; y++ )			{
		if ( f2i(v) != last_row )	{
			last_row = f2i(v);
			decode_row( source_bmp, last_row );
		}
		//scale_source_ptr = &source_bmp->bm_data[source_bmp->bm_rowsize*f2i(v)+f2i(u0)];
		scale_source_ptr = &scale_rle_data[f2i(u0)];
		scale_dest_ptr = &dest_bmp->bm_data[dest_bmp->bm_rowsize*y+x0];
		scale_do_cc_scanline();
		v += dv;
	}
}



// Run-length slice bitmap scan line stretcher 

void DrawHorizontalRun(char *ScreenPtr, int RunLength, int Color)
{
   int i;

   for (i=0; i<RunLength; i++)
      *ScreenPtr++ = Color;
}

void rep_stosb(char *ScreenPtr, int RunLength, int Color);
#pragma aux rep_stosb = \
"				rep	stosb"	\
parm [EDI] [ECX] [EAX]\
modify [];

void rls_stretch_scanline( char * source, char * dest, int XDelta, int YDelta )
{
	   int AdjUp, AdjDown, ErrorTerm;
   	int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;

      /* X major line */
      /* Minimum # of pixels in a run in this line */
      WholeStep = XDelta / YDelta;

      /* Error term adjust each time Y steps by 1; used to tell when one
         extra pixel should be drawn as part of a run, to account for
         fractional steps along the X axis per 1-pixel steps along Y */
      AdjUp = (XDelta % YDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the X step made at that time */
      AdjDown = YDelta * 2;

      /* Initial error term; reflects an initial step of 0.5 along the Y
         axis */
      ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

      /* The initial and last runs are partial, because Y advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      InitialPixelCount = (WholeStep / 2) + 1;
      FinalPixelCount = InitialPixelCount;

      /* If the basic run length is even and there's no fractional
         advance, we have one pixel that could go to either the initial
         or last partial run, which we'll arbitrarily allocate to the
         last run */
      if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
      {
         InitialPixelCount--;
      }
     /* If there're an odd number of pixels per run, we have 1 pixel that can't
     be allocated to either the initial or last partial run, so we'll add 0.5
     to error term so this pixel will be handled by the normal full-run loop */
      if ((WholeStep & 0x01) != 0)
      {
         ErrorTerm += YDelta;
      }
      /* Draw the first, partial run of pixels */
		//if ( *source != Transparency_color )
      	rep_stosb(dest, InitialPixelCount, *source );
		dest += InitialPixelCount;
		source++;

      /* Draw all full runs */
      for (i=0; i<(YDelta-1); i++)
      {
         RunLength = WholeStep;  /* run is at least this long */

         /* Advance the error term and add an extra pixel if the error term so indicates */
         if ((ErrorTerm += AdjUp) > 0)
         {
            RunLength++;
            ErrorTerm -= AdjDown;   /* reset the error term */
         }
         /* Draw this scan line's run */

			//if ( *source != Transparency_color )
	      	rep_stosb(dest, RunLength, *source );
			dest += RunLength;
			source++;

      }

      /* Draw the final run of pixels */
		//if ( *source != Transparency_color )
	      rep_stosb(dest, FinalPixelCount, *source );

      return;
}




void rls_stretch_scanline_setup( int XDelta, int YDelta )
{
		scale_trans_color = Transparency_color & 0xFF;
		scale_ydelta_minus_1 = YDelta - 1;

      /* X major line */
      /* Minimum # of pixels in a run in this line */
      scale_whole_step = XDelta / YDelta;

      /* Error term adjust each time Y steps by 1; used to tell when one
         extra pixel should be drawn as part of a run, to account for
         fractional steps along the X axis per 1-pixel steps along Y */
      scale_adj_up = (XDelta % YDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the X step made at that time */
      scale_adj_down = YDelta * 2;

      /* Initial error term; reflects an initial step of 0.5 along the Y
         axis */
      scale_error_term = (XDelta % YDelta) - (YDelta * 2);

      /* The initial and last runs are partial, because Y advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      scale_initial_pixel_count = (scale_whole_step / 2) + 1;
      scale_final_pixel_count = scale_initial_pixel_count;

      /* If the basic run length is even and there's no fractional
         advance, we have one pixel that could go to either the initial
         or last partial run, which we'll arbitrarily allocate to the
         last run */
      if ((scale_adj_up == 0) && ((scale_whole_step & 0x01) == 0))
      {
         scale_initial_pixel_count--;
      }
     /* If there're an odd number of pixels per run, we have 1 pixel that can't
     be allocated to either the initial or last partial run, so we'll add 0.5
     to error term so this pixel will be handled by the normal full-run loop */
      if ((scale_whole_step & 0x01) != 0)
      {
         scale_error_term += YDelta;
      }

}

