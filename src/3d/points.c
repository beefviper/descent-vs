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
 * Source for point definition, rotation, etc.
 *
 * C port of points.asm.
 */

#include "3dlocal.h"

static vms_vector tempv;
static vms_matrix tempm;

//finds clipping codes for a point. fills in p3_codes, and returns codes
ubyte g3_code_point(g3s_point *p)
{
	ubyte cc = 0;		//clear codes
	fix neg_z;

	if (p->x > p->z)
		cc |= CC_OFF_RIGHT;

	if (p->y > p->z)
		cc |= CC_OFF_TOP;

	neg_z = (fix) (0u - (ulong) p->z);	//32-bit negate, like the asm's neg

	if (neg_z >= 0)
		cc |= CC_BEHIND;

	if (p->x < neg_z)
		cc |= CC_OFF_LEFT;

	if (p->y < neg_z)
		cc |= CC_OFF_BOT;

	return p->p3_codes = cc;
}

//rotate a point. don't look at rotated flags. returns codes
ubyte g3_rotate_point(g3s_point *dest,vms_vector *src)
{
	vm_vec_sub(&tempv,src,&View_position);

	vm_vec_rotate(&dest->p3_vec,&tempv,&View_matrix);

	dest->p3_flags = 0;		//not projected

	return g3_code_point(dest);
}

//projects a point
void g3_project_point(g3s_point *p)
{
	int64_t num;

	if ((p->p3_flags & PF_PROJECTED) || (p->p3_codes & CC_BEHIND))
		return;

	num = (int64_t) p->x * Canv_w2;
	if (divcheck(num,p->z)) {
		p->p3_flags = PF_OVERFLOW;
		return;
	}
	p->p3_sx = (fix) (ulong) ((ulong) (fix) (num / p->z) + (ulong) Canv_w2);

	num = (int64_t) p->y * Canv_h2;
	if (divcheck(num,p->z)) {
		p->p3_flags = PF_OVERFLOW;
		return;
	}
	p->p3_sy = (fix) (ulong) ((ulong) Canv_h2 - (ulong) (fix) (num / p->z));

	p->p3_flags |= PF_PROJECTED;	//projected
}

//from a 2d point on the screen, compute the vector in 3-space through that point
void g3_point_2_vec(vms_vector *v,short sx,short sy)
{
	fix t;

	t = (fix) (((ulong) (ushort) sx) << 16);
	t = fixdiv(t - Canv_w2,Canv_w2);
	tempv.x = fixmuldiv(t,Matrix_scale.z,Matrix_scale.x);

	t = (fix) (((ulong) (ushort) sy) << 16);
	t = fixdiv(t - Canv_h2,Canv_h2);
	tempv.y = -fixmuldiv(t,Matrix_scale.z,Matrix_scale.y);

	tempv.z = f1_0;

	vm_vec_normalize(&tempv);		//get normalized rotated vec

	vm_copy_transpose_matrix(&tempm,&Unscaled_matrix);

	vm_vec_rotate(v,&tempv,&tempm);
}

//rotate a delta y vector
vms_vector *g3_rotate_delta_y(vms_vector *dest,fix dy)
{
	dest->x = fixmul(View_matrix.rvec.y,dy);
	dest->y = fixmul(View_matrix.uvec.y,dy);
	dest->z = fixmul(View_matrix.fvec.y,dy);

	return dest;
}

//rotate a delta x vector
vms_vector *g3_rotate_delta_x(vms_vector *dest,fix dx)
{
	dest->x = fixmul(View_matrix.rvec.x,dx);
	dest->y = fixmul(View_matrix.uvec.x,dx);
	dest->z = fixmul(View_matrix.fvec.x,dx);

	return dest;
}

//rotate a delta z vector
vms_vector *g3_rotate_delta_z(vms_vector *dest,fix dz)
{
	dest->x = fixmul(View_matrix.rvec.z,dz);
	dest->y = fixmul(View_matrix.uvec.z,dz);
	dest->z = fixmul(View_matrix.fvec.z,dz);

	return dest;
}

//rotate a delta vector
vms_vector *g3_rotate_delta_vec(vms_vector *dest,vms_vector *src)
{
	vm_vec_rotate(dest,src,&View_matrix);

	return dest;
}

//adds a delta vector to a point. returns codes.
ubyte g3_add_delta_vec(g3s_point *dest,g3s_point *src,vms_vector *deltav)
{
	vm_vec_add(&dest->p3_vec,&src->p3_vec,deltav);

	dest->p3_flags = 0;		//not projected

	return g3_code_point(dest);
}

//calculate the depth of a point - returns the z coord of the rotated point
fix g3_calc_point_depth(vms_vector *pnt)
{
	int64_t q;

	q  = (int64_t) (fix) ((ulong) pnt->x - (ulong) View_position.x) * View_matrix.fvec.x;
	q += (int64_t) (fix) ((ulong) pnt->y - (ulong) View_position.y) * View_matrix.fvec.y;
	q += (int64_t) (fix) ((ulong) pnt->z - (ulong) View_position.z) * View_matrix.fvec.z;

	return (fix) (q >> 16);
}
