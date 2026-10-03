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
 * Rod routines
 *
 * C port of rod.asm.
 */

#include "3dlocal.h"

extern void scale_bitmap(grs_bitmap *bp,grs_point *vertbuf);	//in the 2d library

static g3s_point rod_points[4];

static g3s_point *rod_point_list[4] = {&rod_points[0],&rod_points[1],&rod_points[2],&rod_points[3]};

//values here are set to be half a pixel in from the edges so the texture
//mapper doesn't read past the bitmap
static g3s_uvl uvl_list[4] = {
	{0x200,0x200,0},
	{0xfe00,0x200,0},
	{0xfe00,0xfe00,0},
	{0x200,0xfe00,0},
};

fix blob_vertices[4*2];		//2d points from g3_draw_bitmap(); the 4th is not set

static vms_vector delta_vec,rod_norm,temp_vec;

//compute the corners of a rod.  fills in rod_points. returns codes_and
//(non-zero means off screen)
static ubyte calc_rod_corners(g3s_point *bot_point,fix bot_width,g3s_point *top_point,fix top_width)
{
	ubyte codes_and;
	int i;

	//compute vector from one point to other, do cross product with vector
	//from eye to get perpendiclar

	vm_vec_sub(&delta_vec,&bot_point->p3_vec,&top_point->p3_vec);

	//unscale for aspect

	delta_vec.x = fixdiv(delta_vec.x,Matrix_scale.x);
	delta_vec.y = fixdiv(delta_vec.y,Matrix_scale.y);

	//calc perp vector

	//do lots of normalizing to prevent overflowing.  When this code works,
	//it should be optimized

	vm_vec_normalize(&delta_vec);

	temp_vec = top_point->p3_vec;
	vm_vec_normalize(&temp_vec);

	vm_vec_crossprod(&rod_norm,&delta_vec,&temp_vec);

	vm_vec_normalize(&rod_norm);

	//scale for aspect

	rod_norm.x = fixmul(rod_norm.x,Matrix_scale.x);
	rod_norm.y = fixmul(rod_norm.y,Matrix_scale.y);

	//now we have the usable edge.  generate four points

	//top points

	vm_vec_copy_scale(&temp_vec,&rod_norm,top_width);
	temp_vec.z = 0;

	vm_vec_add(&rod_points[0].p3_vec,&top_point->p3_vec,&temp_vec);
	vm_vec_sub(&rod_points[1].p3_vec,&top_point->p3_vec,&temp_vec);

	//bot points

	vm_vec_copy_scale(&temp_vec,&rod_norm,bot_width);
	temp_vec.z = 0;

	vm_vec_sub(&rod_points[2].p3_vec,&bot_point->p3_vec,&temp_vec);
	vm_vec_add(&rod_points[3].p3_vec,&bot_point->p3_vec,&temp_vec);

	//now code the four points

	codes_and = 0xff;

	for (i=0;i<4;i++)
		codes_and &= g3_code_point(&rod_points[i]);

	if (codes_and)
		return codes_and;

	//clear flags for new points (not projected)

	for (i=0;i<4;i++)
		rod_points[i].p3_flags = 0;

	return 0;
}

//draws a polygon that's always facing the viewer
void g3_draw_rod_flat(g3s_point *bot_point,fix bot_width,g3s_point *top_point,fix top_width)
{
	if (calc_rod_corners(bot_point,bot_width,top_point,top_width))
		return;		//off screen

	g3_draw_poly(4,rod_point_list);
}

//draws bitmap that's always facing the viewer
//points must be rotated
void g3_draw_rod_tmap(grs_bitmap *bitmap,g3s_point *bot_point,fix bot_width,g3s_point *top_point,fix top_width,fix light)
{
	//save lighting values
	uvl_list[0].l = uvl_list[1].l = uvl_list[2].l = uvl_list[3].l = light;

	if (calc_rod_corners(bot_point,bot_width,top_point,top_width))
		return;		//off screen

	g3_draw_tmap(4,rod_point_list,uvl_list,bitmap);
}

//draws a bitmap with the specified 3d width & height
void g3_draw_bitmap(vms_vector *pos,fix width,fix height,grs_bitmap *bm)
{
	g3s_point *pnt = &rod_points[0];
	int64_t num;
	fix w,h;

	if (g3_rotate_point(pnt,pos) & CC_BEHIND)
		return;		//off screen

	//we should check if off screen based on rad

	g3_project_point(pnt);
	if (pnt->p3_flags & PF_OVERFLOW)
		return;

	//get 2d width & height

	num = (int64_t) width * Canv_w2;
	if (divcheck(num,pnt->z))
		return;
	w = fixmul((fix) (num / pnt->z),Matrix_scale.x);	//width3d*canv_w/z * scale

	num = (int64_t) height * Canv_h2;
	if (divcheck(num,pnt->z))
		return;
	h = fixmul((fix) (num / pnt->z),Matrix_scale.y);	//height3d*canv_h/z * scale

	//copy 2d points into buffer

	blob_vertices[0] = (fix) ((ulong) pnt->p3_sx - (ulong) w);		//p0.x = x - width
	blob_vertices[2] = blob_vertices[4] = (fix) ((ulong) blob_vertices[0] + (ulong) w * 2);	//p1.x, p2.x = + 2*width

	blob_vertices[1] = blob_vertices[3] = (fix) ((ulong) pnt->p3_sy - (ulong) h);	//p0.y, p1.y = y - height
	blob_vertices[5] = (fix) ((ulong) blob_vertices[1] + (ulong) h * 2);	//p2.y = + 2*height

	//now draw
	scale_bitmap(bm,(grs_point *) blob_vertices);
}
