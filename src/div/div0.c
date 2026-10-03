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
 * Divide overflow handling.
 *
 * C port of div0.asm. The original installed a DPMI handler for the
 * divide exception that either jumped to a registered callback, saturated
 * the result, or broke to DOS. Hardware exception handlers cannot be
 * installed this way here (and the C ports of the divides avoid the
 * overflow), so the handler lists are kept but never consulted.
 */

#include "div0.h"

#define MAX_SIZE 100

int div0_num_handled_by_cblist = 0;
int div0_num_handled_by_satlist = 0;
int div0_num_saturated = 0;

static int Already_Init = 0;
static int DefaultMode = DM_ERROR;		// What to do when not in list.

static void *CB_Source[MAX_SIZE];
static void *CB_Dest[MAX_SIZE];
static int CB_Size = 0;

static void *SAT_Source[MAX_SIZE];
static int SAT_Size = 0;

int div0_init(int mode)
{
	if (Already_Init)
		return 1;

	Already_Init = 1;

	div0_num_handled_by_cblist = 0;
	div0_num_handled_by_satlist = 0;
	div0_num_saturated = 0;

	DefaultMode = mode & 1;

	SAT_Size = 0;
	CB_Size = 0;

	return 1;
}

void div0_close()
{
	Already_Init = 0;
}

void div0_set_mode(int mode)
{
	DefaultMode = mode & 1;
}

// Returns 1 if ok, 0 if there are no more slots.
int div0_set_handler( void *div_addr, void *handler_addr )
{
	if (CB_Size + 1 >= MAX_SIZE)
		return 0;

	CB_Source[CB_Size] = div_addr;
	CB_Dest[CB_Size] = handler_addr;
	CB_Size++;
	return 1;
}

int div0_set_saturate( void *div_addr )
{
	if (SAT_Size + 1 >= MAX_SIZE)
		return 0;

	SAT_Source[SAT_Size] = div_addr;
	SAT_Size++;
	return 1;
}
