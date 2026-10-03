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
 * $Source: f:/miner/source/bios/rcs/mono.c $
 * $Revision: 1.12 $
 * $Author: john $
 * $Date: 1995/02/23 11:59:57 $
 *
 * Library functions for printing to mono card.
 *
 * $Log: mono.c $
 * Revision 1.12  1995/02/23  11:59:57  john
 * Made the windows smaller so they don't overwrite the debug file menus.
 *
 * Revision 1.11  1994/11/27  23:07:50  matt
 * Made changes needed to be able to compile out monochrome debugging code
 *
 * Revision 1.10  1994/10/26  22:23:43  john
 * Limited windows to 2.  Took away saving what was under
 * a window.
 *
 * Revision 1.9  1994/07/14  23:25:44  matt
 * Allow window 0 to be opened; don't allow mono to be initialized twice
 *
 * Revision 1.8  1994/03/09  10:45:38  john
 * Sped up scroll.
 *
 * Revision 1.7  1994/01/26  08:56:55  mike
 * Comment out int3 in mputc.
 *
 * Revision 1.6  1994/01/12  15:56:34  john
 * made backspace do an int3 during mono stuff.
 * .,
 *
 * Revision 1.5  1993/12/07  12:33:23  john
 * *** empty log message ***
 *
 * Revision 1.4  1993/10/15  10:10:25  john
 * *** empty log message ***
 *
 * Revision 1.3  1993/09/14  20:55:13  matt
 * Made minit() and mopen() check for presence of mono card in machine.
 *
 * Revision 1.2  1993/07/22  13:10:21  john
 * *** empty log message ***
 *
 * Revision 1.1  1993/07/10  13:10:38  matt
 * Initial revision
 *
 *
 */


// Debug output. The original drew windows on a second, monochrome display
// adapter (MDA) at B0000h. That output now goes to the console window the
// game runs in (standard output): window text is printed as it arrives,
// and the calls that only position or redraw a window do nothing. Window
// 1 ("Errors & Serious Warnings") goes to standard error, so the two can
// be redirected separately.
//
// Builds with NDEBUG or NMONO compile the mono calls out (see mono.h),
// except mprintf, which maps to file_mprintf below.

#include <stdio.h>
#include <stdarg.h>

#include "mono.h"

#define MAX_NUM_WINDOWS 2

void file_mprintf(int n, char* format, ...);

#if !(defined(NDEBUG) || defined(NMONO))

static int window_open[MAX_NUM_WINDOWS];

static int window_ok( int n )
{
	return n >= 0 && n < MAX_NUM_WINDOWS && window_open[n];
}

static FILE *window_stream( int n )
{
	return n == 1 ? stderr : stdout;
}

void mputc( int n, char c )
{
	if (!window_ok(n)) return;
	fputc(c, window_stream(n));
}

// Positioned output was for status displays that redraw in place; on a
// scrolling console it would only add noise.
void mputc_at( int n, int row, int col, char c )
{
	(void)n; (void)row; (void)col; (void)c;
}

void _mprintf( int n, char * format, ... )
{
	va_list args;

	if (!window_ok(n)) return;

	va_start(args, format);
	vfprintf(window_stream(n), format, args);
	va_end(args);
	fflush(window_stream(n));
}

void _mprintf_at( int n, int row, int col, char * format, ... )
{
	(void)n; (void)row; (void)col; (void)format;
}

void msetcursor( int row, int col )
{
	(void)row; (void)col;
}

void mclear( int n )
{
	(void)n;
}

void mclose( int n )
{
	if (n == 0) {
		window_open[0] = window_open[1] = 0;
		return;
	}
	if (n > 0 && n < MAX_NUM_WINDOWS)
		window_open[n] = 0;
}

void mrefresh( short n )
{
	(void)n;
}

void mopen( int n, int row, int col, int width, int height, char * title )
{
	(void)row; (void)col; (void)width; (void)height;

	if (n < 0 || n >= MAX_NUM_WINDOWS) return;

	window_open[n] = 1;
	if (title && *title)
		printf("--- %s ---\n", title);
}

// Returns true (-1) if the debug output is available, as it always is.
int minit()
{
	window_open[0] = 1;
	return -1;
}

#endif

// mprintf() for builds with the mono code compiled out: prints to the
// console too. (Originally appended to debug.txt - beefviper.)
void file_mprintf(int n, char* format, ...)
{
	va_list args;

	(void)n;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	fflush(stdout);
}
