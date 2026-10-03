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
 * Source for setup,close,start & end frame routines
 *
 * C port of setup.asm.
 */

#include <stdlib.h>
#include <stdint.h>

#include "3d.h"
#include "div0.h"

extern void init_interface_vars_to_assembler(void);	//in the texture mapper

//sets up divide overflow handler, and registers g3_close() to be called
//at exit
void g3_init(void)
{
	div0_init(DM_ERROR);		//the asm did an int 3 if this failed

	atexit(g3_close);
}

void g3_close(void)
{
}

//start rendering a frame. sets up window vars
void g3_start_frame(void)
{
	fix aspect;

	Canvas_width = (ushort) grd_curcanv->cv_bitmap.bm_w;	//store width
	Canv_w2 = Canvas_width << 15;									//get fixed-point width/2

	Canvas_height = (ushort) grd_curcanv->cv_bitmap.bm_h;	//store height
	Canv_h2 = Canvas_height << 15;								//get fixed-point height/2

	//compute aspect ratio for this canvas
	aspect = (fix) (((int64_t) grd_curscreen->sc_aspect * Canvas_height) / Canvas_width);

	if (aspect > f1_0) {			//scale y
		Window_scale.x = f1_0;
		Window_scale.y = fixdiv(f1_0,aspect);
	}
	else {
		Window_scale.x = aspect;
		Window_scale.y = f1_0;
	}

	Window_scale.z = f1_0;		//always 1

	init_interface_vars_to_assembler();
}

//this doesn't do anything, but is here for completeness
void g3_end_frame(void)
{
}
