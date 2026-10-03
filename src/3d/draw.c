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
 * Source for drawing routines
 *
 * C port of draw.asm.
 */

#include "3dlocal.h"
#include "texmap.h"

extern void gr_upoly_tmap(int nverts,int *vert);	//in the texture mapper

typedef void (*tmap_drawer_func)(grs_bitmap *bp,int nverts,g3s_point **vertbuf);
typedef void (*flat_drawer_func)(int nverts,int *vert);
typedef int (*line_drawer_func)(fix x0,fix y0,fix x1,fix y1);

static vms_vector tempv;

static tmap_drawer_func tmap_drawer_ptr = draw_tmap;
static flat_drawer_func flat_drawer_ptr = gr_upoly_tmap;
static line_drawer_func line_drawer_ptr = gr_line;

//specifies new routines to call to draw polygons
//Passing NULL for any restores the default
void g3_set_special_render(void *tmap_drawer(),void *flat_drawer(),void *line_drawer())
{
	tmap_drawer_ptr = tmap_drawer ? (tmap_drawer_func) tmap_drawer : draw_tmap;
	flat_drawer_ptr = flat_drawer ? (flat_drawer_func) flat_drawer : gr_upoly_tmap;
	line_drawer_ptr = line_drawer ? (line_drawer_func) line_drawer : gr_line;
}

//draws a line. takes two points.  returns true if drew
bool g3_draw_line(g3s_point *p0,g3s_point *p1)
{
	ubyte codes_or;
	bool ret;

	//check codes for reject, clip, or no clip

	if (p0->p3_codes & p1->p3_codes)		//both off same side
		return 0;									//not drawn

	codes_or = p0->p3_codes | p1->p3_codes;

	if (codes_or & CC_BEHIND)		//neg z means must clip
		goto must_clip_line;

	if (!(p0->p3_flags & PF_PROJECTED))
		g3_project_point(p0);
	if (p0->p3_flags & PF_OVERFLOW)
		goto must_clip_line;

	if (!(p1->p3_flags & PF_PROJECTED))
		g3_project_point(p1);
	if (p1->p3_flags & PF_OVERFLOW)
		goto must_clip_line;

	ret = (bool) line_drawer_ptr(p0->p3_sx,p0->p3_sy,p1->p3_sx,p1->p3_sy);

	if (codes_or == 0)		//we know this one is on screen
		ret = 1;					//definitely drew

	return ret;			//else return value from line drawer

must_clip_line:

	clip_line(&p0,&p1,codes_or);		//do the 3d clip

	ret = g3_draw_line(p0,p1);			//try draw again

	//free up temp points
	if (p0->p3_flags & PF_TEMP_POINT)
		free_temp_point(p0);

	if (p1->p3_flags & PF_TEMP_POINT)
		free_temp_point(p1);

	return ret;
}

//returns true if a plane is facing the viewer. takes the unrotated surface
//normal of the plane, and a point on it.  The normal need not be normalized
bool g3_check_normal_facing(vms_vector *v,vms_vector *norm)
{
	vm_vec_sub(&tempv,&View_position,v);		//view vector

	return (vm_vec_dotprod(&tempv,norm) > 0);
}

//takes normal or NULL, list of point ptrs, and a point on the surface
//returns facing?
static bool do_facing_check(vms_vector *norm,g3s_point **pointlist,vms_vector *pnt)
{
	if (norm) {			//normal passed

		//we have the normal. check if facing
		return g3_check_normal_facing(pnt,norm);
	}
	else {

		//normal not specified, so must compute

		//get three points (rotated) and compute normal

		vm_vec_perp(&tempv,&pointlist[0]->p3_vec,&pointlist[1]->p3_vec,&pointlist[2]->p3_vec);

		return (vm_vec_dotprod(&pointlist[1]->p3_vec,&tempv) < 0);
	}
}

//see if face is visible and draw if it is.
//normal can be NULL, which will for compution here (which will be slow).
//returns -1 if not facing, else the value from g3_draw_poly()
bool g3_check_and_draw_poly(int nv,g3s_point **pointlist,vms_vector *norm,vms_vector *pnt)
{
	if (do_facing_check(norm,pointlist,pnt))
		return g3_draw_poly(nv,pointlist);
	else
		return (bool) -1;
}

bool g3_check_and_draw_tmap(int nv,g3s_point **pointlist,g3s_uvl *uvl_list,grs_bitmap *bm,vms_vector *norm,vms_vector *pnt)
{
	if (do_facing_check(norm,pointlist,pnt))
		return g3_draw_tmap(nv,pointlist,uvl_list,bm);
	else
		return (bool) -1;
}

//project the points of a flat-shaded face, make the list of 2d coords and
//draw it.  returns false if a point overflowed, in which case nothing drew
static int draw_poly_2d(g3s_point **pointlist,int nv)
{
	int i;

	//now make list of 2d coords (& check for overflow)

	for (i=0;i<nv;i++) {
		g3s_point *p = pointlist[i];

		if (!(p->p3_flags & PF_PROJECTED))
			g3_project_point(p);

		if (p->p3_flags & PF_OVERFLOW)
			return 0;

		Vertex_list[i*2]   = p->p3_sx;
		Vertex_list[i*2+1] = p->p3_sy;
	}

	flat_drawer_ptr(nv,(int *) Vertex_list);

	return 1;
}

//draw a flat-shaded face.
//returns 0 if called 2d, 1 if all points off screen
bool g3_draw_poly(int nv,g3s_point **pointlist)
{
	int i;
	g3s_codes cc;
	g3s_point **bufptr;
	bool ret;

	cc.or = 0;			//init codes
	cc.and = 0xff;

	for (i=0;i<nv;i++) {
		g3s_point *p = pointlist[i];

		Vbuf0[i] = p;			//store in ptr array

		cc.and &= p->p3_codes;	//update codes_and
		cc.or  |= p->p3_codes;	//update codes_or
	}

	if (cc.and)				//not visible at all
		return 1;			//no draw

	if (cc.or == 0) {		//all on screen
		if (draw_poly_2d(Vbuf0,nv))
			return 0;		//say it drew
		//else a point overflowed, so must clip
	}

	//we require a 3d clip

	bufptr = clip_polygon(Vbuf0,Vbuf1,&nv,&cc);

	//like the asm, return the low byte of the vertex count if clipped away
	ret = (bool) nv;

	if (nv && !cc.and && !(cc.or & CC_BEHIND)) {	//not clipped away, and no points behind eye
		if (draw_poly_2d(bufptr,nv))
			ret = 0;		//say it drew
		else
			ret = 1;		//shouldn't overflow after clip
	}

	//free temp points
	for (i=0;i<nv;i++)
		if (bufptr[i]->p3_flags & PF_TEMP_POINT)
			free_temp_point(bufptr[i]);

	return ret;
}

//make sure all points of a texture-mapped face are projected, then call
//the texture mapper.  returns false if a point overflowed (no draw)
static int draw_tmap_2d(g3s_point **pointlist,int nv,grs_bitmap *bm)
{
	int i;

	for (i=0;i<nv;i++) {
		g3s_point *p = pointlist[i];

		if (!(p->p3_flags & PF_PROJECTED))
			g3_project_point(p);

		if (p->p3_flags & PF_OVERFLOW)		//should not overflow after clip
			return 0;
	}

	//now call the texture mapper
	tmap_drawer_ptr(bm,nv,pointlist);

	return 1;
}

//draw a texture-mapped face.
//returns 0 if called 2d, 1 if all points off screen
bool g3_draw_tmap(int nv,g3s_point **pointlist,g3s_uvl *uvl_list,grs_bitmap *bm)
{
	int i;
	g3s_codes cc;
	g3s_point **bufptr;
	bool ret;

	//loop to check codes, make list of point ptrs, and copy uvl's into points

	cc.or = 0;			//init codes
	cc.and = 0xff;

	for (i=0;i<nv;i++) {
		g3s_point *p = pointlist[i];

		Vbuf0[i] = p;			//store in ptr array

		cc.and &= p->p3_codes;	//update codes_and
		cc.or  |= p->p3_codes;	//update codes_or

		p->p3_u = uvl_list[i].u;
		p->p3_v = uvl_list[i].v;
		p->p3_l = uvl_list[i].l;

		p->p3_flags |= PF_UVS + PF_LVS;	//this point's got em
	}

	if (cc.and)				//not visible at all
		return 1;			//no draw

	if (cc.or == 0) {		//all on screen
		if (draw_tmap_2d(Vbuf0,nv,bm))
			return 0;		//say it drew
		else
			return 1;
	}

	//we require a 3d clip

	bufptr = clip_polygon(Vbuf0,Vbuf1,&nv,&cc);

	//like the asm, return the low byte of the vertex count if clipped away
	ret = (bool) nv;

	if (nv && !cc.and && !(cc.or & CC_BEHIND)) {	//not clipped away, and no points behind eye
		if (draw_tmap_2d(bufptr,nv,bm))
			ret = 0;		//say it drew
		else
			ret = 1;
	}

	//free temp points
	for (i=nv-1;i>=0;i--)
		if (bufptr[i]->p3_flags & PF_TEMP_POINT)
			free_temp_point(bufptr[i]);

	return ret;
}

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
void g3_draw_sphere(g3s_point *pnt,fix rad)
{
	int64_t num;

	if (pnt->p3_codes & CC_BEHIND)
		return;

	if (!(pnt->p3_flags & PF_PROJECTED))
		g3_project_point(pnt);

	if (pnt->p3_flags & PF_OVERFLOW)
		return;

	//calc radius.  since disk doesn't take width & height, let's just
	//say the the radius is the width

	num = (int64_t) fixmul(rad,Matrix_scale.x) * Canv_w2;

	if (divcheck(num,pnt->z))
		return;

	gr_disk(pnt->p3_sx,pnt->p3_sy,(fix) (num / pnt->z));
}
