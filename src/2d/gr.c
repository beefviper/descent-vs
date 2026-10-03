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
 * $Source: f:/miner/source/2d/rcs/gr.c $
 * $Revision: 1.56 $
 * $Author: john $
 * $Date: 1995/05/08 11:22:47 $
 *
 * Graphical routines for setting video modes, etc.
 *
 * $Log: gr.c $
 * Revision 1.56  1995/05/08  11:22:47  john
 * Added 320x400 3dbios mode.
 *
 * Revision 1.55  1995/02/02  16:44:05  john
 * Again with prev.
 *
 * Revision 1.54  1995/02/02  16:42:18  john
 * Fixed palette with text fading out.
 *
 * Revision 1.53  1995/02/02  14:26:20  john
 * Made palette fades work better with gamma thingy..
 *
 * Revision 1.52  1995/02/02  14:08:59  john
 * Made palette gamma reset to 0 before exiting to Dos.
 *
 * Revision 1.51  1995/01/30  18:06:35  john
 * Added text screen fade in/out, and restored video mode properly.
 *
 * Revision 1.50  1995/01/24  17:58:16  john
 * Added code to return to 80x25 when graphics close.
 *
 * Revision 1.49  1994/11/30  20:16:05  john
 * Fixed bug that the grd_curscreen flags were never initialized.
 *
 * Revision 1.48  1994/11/24  13:24:43  john
 * Made sure that some rep movs had the cld set first.
 * Took some unused functions out.
 *
 * Revision 1.47  1994/11/18  22:50:20  john
 * Changed shorts to ints in parameters.
 *
 * Revision 1.46  1994/11/15  18:28:36  john
 * Made text screen fade in.
 *
 * Revision 1.45  1994/11/15  17:55:11  john
 * Made text palette fade in when game over.
 *
 * Revision 1.44  1994/11/07  12:07:30  john
 * Made save/restore cursor work a bit better.
 *
 * Revision 1.43  1994/11/05  12:46:44  john
 * Changed palette stuff a bit.
 *
 * Revision 1.42  1994/10/26  23:55:50  john
 * Took out roller; Took out inverse table.
 *
 * Revision 1.41  1994/10/10  13:59:50  john
 * *** empty log message ***
 *
 * Revision 1.40  1994/10/10  13:58:50  john
 * Added better VGA detection scheme.
 *
 * Revision 1.39  1994/09/29  10:09:15  john
 * Hacked out VGA card detection for now.
 *
 * Revision 1.38  1994/09/22  17:35:35  john
 * Fixed bug with palette not reloading in
 * gr_set_mode
 *
 * Revision 1.37  1994/09/22  16:08:42  john
 * Fixed some palette stuff.
 *
 * Revision 1.36  1994/09/19  11:44:23  john
 * Changed call to allocate selector to the dpmi module.
 *
 * Revision 1.35  1994/09/12  19:28:11  john
 * Fixed bug with unclipped fonts clipping.
 *
 * Revision 1.34  1994/09/12  18:20:18  john
 * Made text fade out
 *
 * Revision 1.33  1994/09/12  14:40:15  john
 * Neatend.
 *
 * Revision 1.32  1994/08/15  15:01:01  matt
 * Set ptr to NULL after freeing
 *
 * Revision 1.31  1994/07/27  18:30:28  john
 * Took away the blending table.
 *
 * Revision 1.30  1994/06/24  17:26:59  john
 * Made rowsizes bigger than actual screen work with SVGA.
 *
 * Revision 1.29  1994/05/12  17:33:16  john
 * Added circle code.
 *
 * Revision 1.28  1994/05/10  19:51:49  john
 * Added 320x100 mode.
 *
 * Revision 1.27  1994/05/06  12:50:23  john
 * Added supertransparency; neatend things up; took out warnings.
 *
 * Revision 1.26  1994/05/03  19:39:00  john
 * *** empty log message ***
 *
 * Revision 1.25  1994/04/08  16:59:34  john
 * Add fading poly's; Made palette fade 32 instead of 16.
 *
 * Revision 1.24  1994/03/14  16:56:16  john
 * Changed grs_bitmap structure to include bm_flags.
 *
 * Revision 1.23  1994/02/18  15:32:27  john
 * *** empty log message ***
 *
 * Revision 1.22  1994/01/25  11:40:44  john
 * Added gr_check_mode function.
 *
 * Revision 1.21  1993/12/21  19:57:48  john
 * added selector stuff.
 *
 * Revision 1.20  1993/12/21  11:40:32  john
 * *** empty log message ***
 *
 * Revision 1.19  1993/12/09  15:02:13  john
 * Changed palette stuff majorly
 *
 * Revision 1.18  1993/11/16  11:28:36  john
 * *** empty log message ***
 *
 * Revision 1.17  1993/10/26  13:17:53  john
 * *** empty log message ***
 *
 * Revision 1.16  1993/10/15  16:23:42  john
 * y
 *
 * Revision 1.15  1993/09/29  16:15:21  john
 * optimized
 *
 * Revision 1.14  1993/09/28  19:06:51  john
 * made gr_set_mode change the grd_cursreen->sc_mode variable.
 *
 * Revision 1.13  1993/09/28  12:52:12  matt
 * Set aspect ratio of current screen in gr_init() and gr_set_mode().
 *
 * Revision 1.12  1993/09/27  13:00:24  john
 * made gr_set_mode not call mode_reset on fail
 *
 * Revision 1.11  1993/09/27  12:51:27  john
 * fixed gr_set_mode to return values
 *
 * Revision 1.10  1993/09/26  18:59:12  john
 * fade in/out stuff
 *
 * Revision 1.9  1993/09/21  14:00:41  john
 * added code to save 43/50 line modes.
 *
 * Revision 1.8  1993/09/20  14:48:48  john
 * *** empty log message ***
 *
 * Revision 1.7  1993/09/16  17:27:48  john
 * Added code to save/restore video mode.
 *
 * Revision 1.6  1993/09/16  16:30:15  john
 * Made gr_close retore Mode 3 always.
 *
 * Revision 1.5  1993/09/14  18:40:07  john
 * Made it so that gr_setmode doesn't change curcanv font and colors.
 *
 * Revision 1.4  1993/09/14  16:27:01  matt
 * Changes gr_change_mode() to be gr_set_mode()
 * After gr_set_mode(), grd_curcanv is the canvas of the new screen
 * Made gr_set_mode() work with the new grd_curcanv, not the old one
 *
 * Revision 1.3  1993/09/14  13:08:37  john
 * Added gr_changemode
 *
 * Revision 1.2  1993/09/08  17:36:37  john
 * Looking for error for Yuan... Neatened the nested ifs in setmode.
 *
 * Revision 1.1  1993/09/08  11:43:51  john
 * Initial revision
 *
 *
 */

#include <dos.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "types.h"
#include "mem.h"
#include "gr.h"
#include "grdef.h"
#include "error.h"
#include "mono.h"
#include "dpmi.h"
#include "palette.h"
#include "platform.h"

// Video memory. The game now draws into a framebuffer owned by the
// platform backend (an SDL window), so every screen mode is linear.
unsigned char * gr_video_memory = NULL;

int gr_installed = 0;

int gr_show_screen_info = 0;

// There is no text screen to save or restore.
int gr_save_mode(void)
{
	return 0;
}

void gr_restore_mode(void)
{
	gr_palette_fade_out( gr_palette, 32, 0 );
	gr_palette_set_gamma(0);
}

int gr_close(void)
{
	if (gr_installed==1)
	{
		gr_installed = 0;
		gr_restore_mode();
		plat_video_close();
		gr_video_memory = NULL;
		free(grd_curscreen);
	}

	return 0;
}

// Screen size for a mode, in pixels. Returns 0 for an unknown mode.
static int gr_mode_size( int mode, int *w, int *h )
{
	static const short sizes[][2] = {
		{ 320, 200 },		// SM_320x200C
		{ 320, 200 },		// SM_320x200U
		{ 320, 240 },		// SM_320x240U
		{ 360, 200 },		// SM_360x200U
		{ 360, 240 },		// SM_360x240U
		{ 376, 282 },		// SM_376x282U
		{ 320, 400 },		// SM_320x400U
		{ 320, 480 },		// SM_320x480U
		{ 360, 400 },		// SM_360x400U
		{ 360, 480 },		// SM_360x480U
		{ 360, 360 },		// SM_360x360U
		{ 376, 308 },		// SM_376x308U
		{ 376, 564 },		// SM_376x564U
		{ 640, 400 },		// SM_640x400V
		{ 640, 480 },		// SM_640x480V
		{ 800, 600 },		// SM_800x600V
		{ 1024, 768 },		// SM_1024x768V
	};

	switch (mode) {
	case 19:	*w = 320; *h = 100; return 1;	// mode 13h with doubled rows
	case 20:	*w = 400; *h = 600; return 1;
	case 21:	*w = 160; *h = 100; return 1;
	case 22:	*w = 320; *h = 400; return 1;	// 3dmax 320x400
	}

	// The 15-bit modes (SM_640x480V15, SM_800x600V15) aren't supported.
	if (mode < 0 || mode >= (int)(sizeof(sizes) / sizeof(sizes[0])))
		return 0;

	*w = sizes[mode][0];
	*h = sizes[mode][1];
	return 1;
}

int gr_set_mode(int mode)
{
	int w, h;
	unsigned char *data;

	if (mode == SM_ORIGINAL)
		return 0;

	if (!gr_mode_size( mode, &w, &h ))
		return 11;

	// Two pages, for the code that flips between them (game.c, automap.c).
	data = plat_video_set_mode( w, h, 2 );
	if (data == NULL)
		return 3;
	gr_video_memory = data;

	gr_palette_clear();

	memset( grd_curscreen, 0, sizeof(grs_screen));
	grd_curscreen->sc_mode = mode;
	grd_curscreen->sc_w = w;
	grd_curscreen->sc_h = h;
	grd_curscreen->sc_aspect = fixdiv(grd_curscreen->sc_w*3,grd_curscreen->sc_h*4);
	grd_curscreen->sc_canvas.cv_bitmap.bm_x = 0;
	grd_curscreen->sc_canvas.cv_bitmap.bm_y = 0;
	grd_curscreen->sc_canvas.cv_bitmap.bm_w = w;
	grd_curscreen->sc_canvas.cv_bitmap.bm_h = h;
	grd_curscreen->sc_canvas.cv_bitmap.bm_rowsize = w;
	grd_curscreen->sc_canvas.cv_bitmap.bm_type = BM_LINEAR;
	grd_curscreen->sc_canvas.cv_bitmap.bm_data = data;
	gr_set_current_canvas(NULL);

	return 0;
}

int gr_init(int mode)
{
	int retcode;

	// Only do this function once!
	if (gr_installed==1)
		return 1;

	if (gr_init_A0000())
		return 10;

	// Save the current text screen mode
	if (gr_save_mode()==1)
		return 1;

	//MALLOC( grd_curscreen,grs_screen,1 );//Hack by KRB
	grd_curscreen=(grs_screen*)malloc(1*sizeof(grs_screen));
	if (grd_curscreen == NULL)
		Error("Out of memory");
	memset( grd_curscreen, 0, sizeof(grs_screen));

	// Set the mode.
	if ((retcode=gr_set_mode(mode)))
	{
		gr_restore_mode();
		return retcode;
	}
	//JOHNgr_disable_default_palette_loading();

	// Set all the screen, canvas, and bitmap variables that
	// aren't set by the gr_set_mode call:
	grd_curscreen->sc_canvas.cv_color = 0;
	grd_curscreen->sc_canvas.cv_drawmode = 0;
	grd_curscreen->sc_canvas.cv_font = NULL;
	grd_curscreen->sc_canvas.cv_font_fg_color = 0;
	grd_curscreen->sc_canvas.cv_font_bg_color = 0;
	gr_set_current_canvas( &grd_curscreen->sc_canvas );

	if (!dpmi_allocate_selector( &gr_fade_table, 256*GR_FADE_LEVELS, &gr_fade_table_selector ))
		Error( "Error allocating fade table selector!" );

	if (!dpmi_allocate_selector( &gr_palette, 256*3, &gr_palette_selector ))
		Error( "Error allocating palette selector!" );

//	if (!dpmi_allocate_selector( &gr_inverse_table, 32*32*32, &gr_inverse_table_selector ))
//		Error( "Error allocating inverse table selector!" );


	// Set flags indicating that this is installed.
	gr_installed = 1;
	atexit((void*)gr_close);

	return 0;
}

// Returns 0 if the mode can be set, 11 if it isn't a mode gr.lib knows.
int gr_check_mode(int mode)
{
	int w, h;

	return gr_mode_size( mode, &w, &h ) ? 0 : 11;
}
