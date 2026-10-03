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
 * Source for clipper
 *
 * C port of clipper.asm.
 */

#include "3dlocal.h"

//buffer of temp points for when clipping creates a new point
static g3s_point temp_points[MAX_POINTS_IN_POLY];

static g3s_point *free_points[MAX_POINTS_IN_POLY] = {
	&temp_points[0], &temp_points[1], &temp_points[2], &temp_points[3], &temp_points[4],
	&temp_points[5], &temp_points[6], &temp_points[7], &temp_points[8], &temp_points[9],
	&temp_points[10],&temp_points[11],&temp_points[12],&temp_points[13],&temp_points[14],
	&temp_points[15],&temp_points[16],&temp_points[17],&temp_points[18],&temp_points[19],
	&temp_points[20],&temp_points[21],&temp_points[22],&temp_points[23],&temp_points[24],
	&temp_points[25],&temp_points[26],&temp_points[27],&temp_points[28],&temp_points[29],
	&temp_points[30],&temp_points[31],&temp_points[32],&temp_points[33],&temp_points[34],
	&temp_points[35],&temp_points[36],&temp_points[37],&temp_points[38],&temp_points[39],
	&temp_points[40],&temp_points[41],&temp_points[42],&temp_points[43],&temp_points[44],
	&temp_points[45],&temp_points[46],&temp_points[47],&temp_points[48],&temp_points[49],
	&temp_points[50],&temp_points[51],&temp_points[52],&temp_points[53],&temp_points[54],
	&temp_points[55],&temp_points[56],&temp_points[57],&temp_points[58],&temp_points[59],
	&temp_points[60],&temp_points[61],&temp_points[62],&temp_points[63],&temp_points[64],
	&temp_points[65],&temp_points[66],&temp_points[67],&temp_points[68],&temp_points[69],
	&temp_points[70],&temp_points[71],&temp_points[72],&temp_points[73],&temp_points[74],
	&temp_points[75],&temp_points[76],&temp_points[77],&temp_points[78],&temp_points[79],
	&temp_points[80],&temp_points[81],&temp_points[82],&temp_points[83],&temp_points[84],
	&temp_points[85],&temp_points[86],&temp_points[87],&temp_points[88],&temp_points[89],
	&temp_points[90],&temp_points[91],&temp_points[92],&temp_points[93],&temp_points[94],
	&temp_points[95],&temp_points[96],&temp_points[97],&temp_points[98],&temp_points[99],
};

int free_point_num = 0;

//get a temporary point
static g3s_point *get_temp_point(void)
{
	g3s_point *p;

	p = free_points[free_point_num++];

	p->p3_flags = PF_TEMP_POINT;		//clear proj,set temp

	return p;
}

//free a temporary point
void free_temp_point(g3s_point *p)
{
	free_points[--free_point_num] = p;
}

//clip a particular value (eg. x, y, u).
//takes start & end values, and the fraction num/den.  returns new value
static __inline fix clip_value(fix start,fix end,fix num,fix den)
{
	fix delta = (fix) ((ulong) end - (ulong) start);

	return (fix) ((ulong) (fix) (((int64_t) delta * num) / den) + (ulong) start);
}

//clips an edge against one plane.
//takes on, off=points, plane flag (1,2,4,8 = left,right,bot,top)
//returns the new point, with its codes set
static g3s_point *clip_edge(int plane,g3s_point *on_pnt,g3s_point *off_pnt)
{
	fix a,b,num,den;
	g3s_point *tmp;

	//compute clipping value k = (xs-zs) / (xs-xe-zs+ze)
	//use x or y as appropriate, and negate x/y value as appropriate

	if (plane & (CC_OFF_TOP|CC_OFF_BOT)) {		//top or bot (y)?
		a = on_pnt->y;
		b = off_pnt->y;
	}
	else {
		a = on_pnt->x;
		b = off_pnt->x;
	}

	if (plane & (CC_OFF_LEFT|CC_OFF_BOT)) {		//left or bot (neg)?
		a = (fix) (0u - (ulong) a);
		b = (fix) (0u - (ulong) b);
	}

	num = (fix) ((ulong) a - (ulong) on_pnt->z);					//xs-zs
	den = (fix) ((ulong) num - (ulong) b + (ulong) off_pnt->z);	//xs-xe-zs+ze

	//now frac=num/den

	tmp = get_temp_point();

	tmp->x = clip_value(on_pnt->x,off_pnt->x,num,den);
	tmp->z = tmp->x;		//assume z=x

	tmp->y = clip_value(on_pnt->y,off_pnt->y,num,den);

	if (plane & (CC_OFF_TOP|CC_OFF_BOT))		//top or bot (y)?
		tmp->z = tmp->y;		//z=y

	//check if uv values present, and clip if so
	if (on_pnt->p3_flags & PF_UVS) {
		tmp->p3_u = clip_value(on_pnt->p3_u,off_pnt->p3_u,num,den);
		tmp->p3_v = clip_value(on_pnt->p3_v,off_pnt->p3_v,num,den);
		tmp->p3_flags |= PF_UVS;		//new point has uv set
	}

	//check if lv values present, and clip if so
	if (on_pnt->p3_flags & PF_LVS) {
		tmp->p3_l = clip_value(on_pnt->p3_l,off_pnt->p3_l,num,den);
		tmp->p3_flags |= PF_LVS;		//new point has lv set
	}

	//negate z if clipping against left or bot
	if (plane & (CC_OFF_LEFT|CC_OFF_BOT))
		tmp->z = (fix) (0u - (ulong) tmp->z);		//z=-x (or -y)

	g3_code_point(tmp);

	return tmp;
}

//clips a line to the viewing pyramid.
//takes pointers to the two point pointers, and codes_or
//on return, the point pointers point at the clipped points, one or both new
void clip_line(g3s_point **p0,g3s_point **p1,ubyte codes_or)
{
	int plane;
	g3s_point *on_pnt = *p0,*off_pnt = *p1,*old_pnt;
	ubyte new_codes;

	for (plane=1;plane<16;plane<<=1) {

		if (codes_or & plane) {

			if (on_pnt->p3_codes & plane) {	//this one on?
				g3s_point *t = on_pnt;		//..nope
				on_pnt = off_pnt;
				off_pnt = t;
			}

			old_pnt = off_pnt;		//save old off-screen point

			off_pnt = clip_edge(plane,on_pnt,off_pnt);
			new_codes = off_pnt->p3_codes;

			//see if must free rejected point
			if (old_pnt->p3_flags & PF_TEMP_POINT)
				free_temp_point(old_pnt);

			codes_or = on_pnt->p3_codes;

			if (codes_or & new_codes)		//clipped away?
				break;

			codes_or |= new_codes;		//get new codes_or
		}
	}

	*p0 = on_pnt;
	*p1 = off_pnt;
}

//clip a polygon against one plane.  src must have room for 2 extra entries.
//returns new vertex count, and fills in new codes
static int clip_plane(int plane,g3s_point **src,g3s_point **dest,int nv,g3s_codes *cc)
{
	int i;
	g3s_point **dest_start = dest;
	g3s_point *cur,*prev,*next,*new_pnt;

	//copy first two verts to end
	src[nv]   = src[0];
	src[nv+1] = src[1];

	cc->or = 0;			//initialize codes
	cc->and = 0xff;

	//go though list of points.

	for (i=1;i<=nv;i++) {

		cur = src[i];

		if (cur->p3_codes & plane) {	//cur point off?

			//cur point is off. check prev and next

			prev = src[i-1];

			if (!(prev->p3_codes & plane)) {		//prev on, so clip
				new_pnt = clip_edge(plane,prev,cur);		//clip cur (off screen) to prev (on screen)
				*dest++ = new_pnt;
				cc->or  |= new_pnt->p3_codes;
				cc->and &= new_pnt->p3_codes;
			}

			next = src[i+1];

			if (!(next->p3_codes & plane)) {		//next on, so clip
				new_pnt = clip_edge(plane,next,cur);		//clip cur (off screen) to next (on screen)
				*dest++ = new_pnt;
				cc->or  |= new_pnt->p3_codes;
				cc->and &= new_pnt->p3_codes;
			}

			//see if must free discarded point
			if (cur->p3_flags & PF_TEMP_POINT)
				free_temp_point(cur);
		}
		else {
			*dest++ = cur;			//store cur in dest buffer
			cc->or  |= cur->p3_codes;
			cc->and &= cur->p3_codes;
		}
	}

	return (int) (dest - dest_start);
}

//3d clip a polygon.
//takes src list, dest list, nv=nverts, cc=codes
//returns list of clipped points, some of them new, nv=new nverts, cc=new codes
//the returned list will be either of src,dest at entry
g3s_point **clip_polygon(g3s_point **src,g3s_point **dest,int *nv,g3s_codes *cc)
{
	int plane;

	//now loop through each plane, a-clipping as we go

	for (plane=1;plane<16;plane<<=1) {

		if (cc->or & plane) {		//off this plane?
			g3s_point **t;

			//clip against this plane, from src to dest
			*nv = clip_plane(plane,src,dest,*nv,cc);

			//new dest = old src
			t = src;
			src = dest;
			dest = t;

			if (cc->and)		//polygon all off screen
				break;
		}
	}

	return src;
}
