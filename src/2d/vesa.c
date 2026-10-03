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
 * VESA SuperVGA routines.
 * C port of vesa.asm.
 *
 * The assembly called the VESA BIOS (int 10h) through DPMI and drew into the
 * banked 64K video window at A0000h.  None of that exists on Windows, so these
 * are harmless stubs: mode checks report that no VESA driver is present, and
 * the drawing and bank switching routines do nothing.  A Windows display
 * backend will replace them.
 */

#include "gr.h"
#include "grdef.h"

unsigned short _A0000;		// selector for the A0000h screen segment (unused)

// Allocated a selector for A0000h.  Returns 0 on success.
int gr_init_A0000()
{
	_A0000 = 0;
	return 0;
}

//  Returns 0 if the VESA mode is supported, else an error code
//  (5 = No VESA driver found).
int gr_vesa_checkmode( int mode )
{
	(void)mode;
	return 5;
}

// Allocated real mode DOS memory.  Returns its address, or 0 if failed.
int gr_get_dos_mem( int size )
{
	(void)size;
	return 0;
}

// Sets a VESA mode.  Returns 0 if OK, else an error code (6 = Bad Status after VESA call).
int gr_vesa_setmodea( int mode )
{
	(void)mode;
	return 6;
}

// Selected 64K page of video memory in the A0000h window.
void gr_vesa_setpage( int page )
{
	(void)page;
}

void gr_vesa_setaddress( int address )
{
	(void)address;
}

void gr_vesa_incpage()
{
}

// Set the display start address.
void gr_vesa_setstart( short x, short y )
{
	(void)x; (void)y;
}

// Set the logical scanline width.  Returns the width.
int gr_vesa_setlogical( int pixels_per_scanline )
{
	return pixels_per_scanline;
}

int gr_vesa_set_logical( int pixels_per_scanline )
{
	return pixels_per_scanline;
}

// Drew a horizontal line of color from x1 to x2 at y in banked video memory.
void gr_vesa_scanline( short x1, short x2, short y, unsigned char color )
{
	(void)x1; (void)x2; (void)y; (void)color;
}

// Wrote one pixel at offset into banked video memory.
void gr_vesa_pixel( unsigned char color, unsigned int offset )
{
	(void)color; (void)offset;
}

// Copied a height by width block from source_ptr to banked video memory.
void gr_vesa_bitblt( unsigned char * source_ptr, unsigned int vesa_address, int height, int width )
{
	(void)source_ptr; (void)vesa_address; (void)height; (void)width;
}

// Copied a linear bitmap to (x,y) of an SVGA bitmap.
void gr_vesa_bitmap( grs_bitmap * source, grs_bitmap * dest, int x, int y )
{
	(void)source; (void)dest; (void)x; (void)y;
}

// Copied the pixels of source1 that differ from source2 to an SVGA bitmap.
void gr_vesa_update( grs_bitmap * source1, grs_bitmap * dest, grs_bitmap * source2 )
{
	(void)source1; (void)dest; (void)source2;
}
