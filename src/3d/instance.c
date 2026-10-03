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
 * Code for handling instanced 3d objects
 *
 * C port of instance.asm.
 */

#include "3d.h"
#include "error.h"

#define MAX_INSTANCE_DEPTH	5

static struct instance_context {
	vms_vector p;
	vms_matrix m;
} instance_stack[MAX_INSTANCE_DEPTH];

static int instance_depth = 0;

static vms_vector tempv;
static vms_matrix tempm,tempm2;

static vms_matrix instmat;

//start instancing, using a matrix
//if matrix==NULL, don't modify matrix.  This will be like doing an offset
void g3_start_instance_matrix(vms_vector *pos,vms_matrix *orient)
{
	#ifndef NDEBUG
	Assert(instance_depth < MAX_INSTANCE_DEPTH);
	#endif

	//save current context
	instance_stack[instance_depth].p = View_position;
	instance_stack[instance_depth].m = View_matrix;
	instance_depth++;

	//step 1: subtract object position from view position
	vm_vec_sub2(&View_position,pos);

	if (orient) {

		//step 2: rotate view vector through object matrix
		vm_vec_rotate(&tempv,&View_position,orient);
		View_position = tempv;

		//step 3: rotate object matrix through view_matrix (vm = ob * vm)
		vm_copy_transpose_matrix(&tempm2,orient);
		vm_matrix_x_matrix(&tempm,&tempm2,&View_matrix);
		View_matrix = tempm;
	}
}

//start instancing, using angles (called vm_angles_2_matrix)
//if angles==NULL, don't modify matrix.  This will be like doing an offset
void g3_start_instance_angles(vms_vector *pos,vms_angvec *angles)
{
	if (angles == NULL) {
		g3_start_instance_matrix(pos,NULL);	//no new matrix
		return;
	}

	vm_angles_2_matrix(&instmat,angles);

	g3_start_instance_matrix(pos,&instmat);
}

//we are done instancing
void g3_done_instance(void)
{
	instance_depth--;

	#ifndef NDEBUG
	Assert(instance_depth >= 0);	//instance stack underflow
	#endif

	View_position = instance_stack[instance_depth].p;
	View_matrix = instance_stack[instance_depth].m;
}
