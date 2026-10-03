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
 * Horizon drawing routines
 *
 * C port of horizon.asm.
 */

#include "3dlocal.h"

extern void gr_upoly_tmap(int nverts,int *vert);	//in the texture mapper

//32-bit wrapping arithmetic, like the asm
#define ADD(a,b)	((fix) ((ulong) (a) + (ulong) (b)))
#define SUB(a,b)	((fix) ((ulong) (a) - (ulong) (b)))
#define NEG(a)		((fix) (0u - (ulong) (a)))

//the abs_eax macro: abs(0x80000000) stays negative
#define ABS(a)		((a) < 0 ? NEG(a) : (a))

typedef struct horz_point {
	fix x,y;			//coords of point
	int edge;		//which edge point is on
} horz_point;

static int sky_color;
static int ground_color;
static int top_color;
static int bot_color;

static short color_swap;		//flag for if we swapped
static short sky_ground_flag;	//0=both, 1=all sky, -1=all gnd

static vms_vector horizon_vec;	//unscaled up vector

static fix up_right,down_right,down_left,up_left;

static horz_point left_point,right_point;

//the coordinates of the four corners of the screen (values are placeholders)
static fix corners[4*2] = {
	0,0,			//up left
	319,0,		//up right
	319,199,		//down right
	0,199,		//down left
};

static fix horizon_poly[5*2];	//max of 5 points

//draw a polygon (one half of horizon) from the horizon line
static void draw_horz_poly(horz_point *start,horz_point *end)
{
	int n,i,c;

	//copy horizon line as first points in poly

	horizon_poly[0] = end->x;
	horizon_poly[1] = end->y;

	horizon_poly[2] = start->x;
	horizon_poly[3] = start->y;

	//add corners to polygon

	n = end->edge - start->edge;	//number of edges
	if (n < 0)
		n += 4;

	c = start->edge;					//first corner

	for (i=0;i<n;i++) {
		horizon_poly[4+i*2]   = corners[c*2];		//copy a corner
		horizon_poly[4+i*2+1] = corners[c*2+1];
		if (++c == 4)					//end of list?
			c = 0;
	}

	//now draw the polygon
	gr_upoly_tmap(n+2,(int *) horizon_poly);
}

//draws a horizon. takes sky_color, ground_color
void g3_draw_horizon(int s_color,int g_color)
{
	fix x2,y2,t;
	ubyte flags_and,flags_or;
	horz_point ends[4] = {0},*e;
	int64_t q;

	sky_color = s_color;
	ground_color = g_color;

	color_swap = 0;		//assume no swap
	sky_ground_flag = 0;	//assume both

	//check up
	if (View_matrix.uvec.y > 0 || (View_matrix.uvec.y == 0 && View_matrix.uvec.x < 0)) {
		top_color = s_color;
		bot_color = g_color;
	}
	else {					//make sky down
		top_color = g_color;
		bot_color = s_color;
		color_swap = -1;	//we swapped
	}

	//compute horizon_vector

	horizon_vec.x = fixmul(fixmul(Unscaled_matrix.rvec.y,Matrix_scale.y),Matrix_scale.z);
	horizon_vec.y = fixmul(fixmul(Unscaled_matrix.uvec.y,Matrix_scale.x),Matrix_scale.z);
	horizon_vec.z = fixmul(fixmul(Unscaled_matrix.fvec.y,Matrix_scale.x),Matrix_scale.y);

	//now compute values & flag for 4 corners.

	flags_and = 0xff;
	flags_or = 0;

	up_right = ADD(ADD(horizon_vec.z,horizon_vec.x),horizon_vec.y);	//x+y+z
	flags_and &= (up_right < 0) ? 0xff : 0;
	flags_or  |= (up_right < 0) ? 0xff : 0;

	x2 = (fix) ((ulong) horizon_vec.x << 1);		//get x*2, y*2
	y2 = (fix) ((ulong) horizon_vec.y << 1);

	down_right = SUB(up_right,y2);		//x-y+z
	flags_and &= (down_right < 0) ? 0xff : 0;
	flags_or  |= (down_right < 0) ? 0xff : 0;

	down_left = SUB(down_right,x2);		//-x-y+z
	flags_and &= (down_left < 0) ? 0xff : 0;
	flags_or  |= (down_left < 0) ? 0xff : 0;

	up_left = ADD(down_left,y2);			//-x+y+z

	//check flags for all sky or all ground.

	flags_and &= (up_left < 0) ? 0xff : 0;
	if (flags_and) {		//we see all ground.  clear screen with appropriate color
		sky_ground_flag = -1;
		gr_clear_canvas(ground_color);
		return;
	}

	flags_or |= (up_left < 0) ? 0xff : 0;
	if (!flags_or) {		//we see all sky.  clear screen with appropriate color
		sky_ground_flag = 1;
		gr_clear_canvas(sky_color);
		return;
	}

	//fill in values for corners

	corners[2] = corners[4] = (fix) ((ulong) Canvas_width << 16);	//make a fix
	corners[5] = corners[7] = (fix) ((ulong) Canvas_height << 16);

	//check for intesection with each of four edges & compute horizon line
	//(there are always exactly two)

	e = ends;

	//check intersection with left edge

	if ((up_left ^ down_left) < 0) {		//signs different?
		q = ((int64_t) corners[5] * up_left) / y2;		//up_left * height / y*2
		t = (fix) q;
		e->y = ABS(t);
		e->x = 0;
		e->edge = 0;
		e++;
	}

	//check intersection with top edge

	if ((up_left ^ up_right) < 0) {		//signs different?
		q = ((int64_t) corners[4] * up_left) / x2;		//up_left * width / x*2
		t = (fix) q;
		e->x = ABS(t);
		e->y = 0;
		e->edge = 1;
		e++;
	}

	//check intersection with right edge

	if ((up_right ^ down_right) < 0) {		//signs different?
		q = ((int64_t) corners[5] * up_right) / y2;		//up_right * height / y*2
		t = (fix) q;
		e->y = ABS(t);
		e->x = corners[4];		//x=width-1
		e->edge = 2;
		e++;
	}

	//check intersection with bottom edge

	if ((down_right ^ down_left) < 0) {		//signs different?
		q = ((int64_t) corners[4] * down_left) / x2;		//down_left * width / x*2
		t = (fix) q;
		e->x = ABS(t);
		e->y = corners[5];		//y=height-1
		e->edge = 3;
		e++;
	}

	left_point = ends[0];
	right_point = ends[1];

	//make sure first edge is left

	if (left_point.x > right_point.x) {		//swap left and right
		horz_point t = left_point;
		left_point = right_point;
		right_point = t;
	}

	//draw two polygons from horizon line

	if (top_color != -1) {
		gr_setcolor(top_color);
		draw_horz_poly(&left_point,&right_point);
	}

	if (bot_color != -1) {
		gr_setcolor(bot_color);
		draw_horz_poly(&right_point,&left_point);
	}
}

//compute vector describing horizon intersection with a point.
//takes 2d point (x,y fix pair), fills in vec.
static void compute_horz_end_vec(fix *pnt,vms_vector *v)
{
	fix xfrac,yfrac,t,a,b,ratio,mag;
	int64_t q;

	//compute rotated x/z & y/z ratios

	xfrac = fixdiv(SUB(pnt[0],Canv_w2),Canv_w2);
	yfrac = NEG(fixdiv(SUB(pnt[1],Canv_h2),Canv_h2));	//y inversion

	//compute fraction unrotated x/z

	t = ADD(xfrac,yfrac);
	b = SUB(SUB(fixmul(t,View_matrix.fvec.z),View_matrix.rvec.z),View_matrix.uvec.z);	//numerator
	a = SUB(ADD(View_matrix.rvec.x,View_matrix.uvec.x),fixmul(t,View_matrix.fvec.x));

	//now a/b = z/x. do divide in way to give result < 0

	if (ABS(b) >= ABS(a)) {

		//x is bigger, so do as z/x

		ratio = fixdiv(a,b);

		//now ratio = z/x ratio.  Compute vector by normalizing and correcting sign

		q = (int64_t) ratio * ratio;	//compute z*z
		mag = (fix) quad_sqrt((long) (ulong) q,(long) ((ulong) (q >> 32) + 1));	//+ x*x (x==1)

		v->z = fixdiv(ratio,mag);
		v->x = fixdiv(f1_0,mag);
	}
	else {

		//z is bigger, so do as x/z

		ratio = fixdiv(b,a);

		//now ratio = x/z ratio.  Compute vector by normalizing and correcting sign

		q = (int64_t) ratio * ratio;	//compute x*x
		mag = (fix) quad_sqrt((long) (ulong) q,(long) ((ulong) (q >> 32) + 1));	//+ z*z (z==1)

		v->x = fixdiv(ratio,mag);
		v->z = fixdiv(f1_0,mag);
	}

	v->y = 0;

	//now make sure that this vector is in front of you, not behind

	q = (int64_t) v->x * View_matrix.fvec.x + (int64_t) v->z * View_matrix.fvec.z;

	if (q < 0) {		//z is neg, flip vector
		v->x = NEG(v->x);
		v->z = NEG(v->z);
	}
}

#define MIN_DEN 0x7fff

//compute vector decribing a corner of the screen.
//takes vector, corner num
static void compute_corner_vec(vms_vector *v,int corner_num)
{
	fix b,c,d,den;
	fix m13,m46,m79,m56,m23,m89;

	if (corner_num >= 4)
		corner_num -= 4;

	//compute all deltas

	b = View_matrix.rvec.x;
	c = View_matrix.rvec.y;
	d = View_matrix.rvec.z;

	if (corner_num == 0 || corner_num == 3) {
		b = NEG(b);
		c = NEG(c);
		d = NEG(d);
	}

	m13 = SUB(b,View_matrix.fvec.x);	//m1-m3
	m46 = SUB(c,View_matrix.fvec.y);	//m4-m6
	m79 = SUB(d,View_matrix.fvec.z);	//m7-m9

	b = View_matrix.uvec.y;
	c = View_matrix.uvec.x;
	d = View_matrix.uvec.z;

	if (corner_num >= 2) {
		b = NEG(b);
		c = NEG(c);
		d = NEG(d);
	}

	m56 = SUB(b,View_matrix.fvec.y);	//m5-m6
	m23 = SUB(c,View_matrix.fvec.x);	//m2-m3
	m89 = SUB(d,View_matrix.fvec.z);	//m8-m9

	//compute x/z ratio

	//compute denomonator

	den = SUB(fixmul(m56,m13),fixmul(m46,m23));

	//(the asm then checked abs(den) against MIN_DEN, but did nothing either way)

	//now do x/z numerator, and divide by the denominator

	v->x = fixdiv(SUB(fixmul(m89,m46),fixmul(m79,m56)),den);	//x/z

	//now do y/z

	v->y = fixdiv(SUB(fixmul(m79,m23),fixmul(m89,m13)),den);	//y/z

	v->z = f1_0;

	vm_vec_normalize(v);

	//make sure this vec is pointing in right direction

	if (vm_vec_dotprod(v,&View_matrix.fvec) <= 0) {
		v->x = NEG(v->x);
		v->y = NEG(v->y);
		v->z = NEG(v->z);
	}
}

//return information on the polygon that is the sky.
//takes ptr to x,y pairs, ptr to vecs for each point
//returns number of points
//IMPORTANT: g3_draw_horizon() must be called before this routine.
int g3_compute_sky_polygon(fix *points_2d,vms_vector *vecs)
{
	horz_point *start,*end;
	int n,i,corner_num;

	if (sky_ground_flag < 0)		//we drew all ground, so there was no horizon drawn
		return 0;						//no points in poly

	if (sky_ground_flag > 0) {		//we drew all sky, so find 4 corners

		for (i=0;i<8;i++)
			points_2d[i] = corners[i];

		for (i=0;i<4;i++)
			compute_corner_vec(&vecs[i],i);

		return 4;		//4 corners
	}

	start = &left_point;
	end = &right_point;

	if (color_swap) {		//sky isn't top
		start = &right_point;
		end = &left_point;
	}

	//copy horizon line as first points in poly

	points_2d[0] = end->x;		//copy end point
	points_2d[1] = end->y;

	points_2d[2] = start->x;	//copy start point
	points_2d[3] = start->y;

	compute_horz_end_vec(&points_2d[0],&vecs[0]);	//end point is first point
	compute_horz_end_vec(&points_2d[2],&vecs[1]);

	points_2d += 4;		//past two x,y pairs
	vecs += 2;				//past two vectors

	//add corners to polygon

	corner_num = start->edge;

	n = end->edge - start->edge;		//number of edges
	if (n < 0)
		n += 4;

	for (i=0;i<n;i++) {
		int c = corner_num & 3;

		points_2d[i*2]   = corners[c*2];		//copy a corner
		points_2d[i*2+1] = corners[c*2+1];

		compute_corner_vec(&vecs[i],corner_num);

		corner_num++;
	}

	//now return with count
	return n+2;		//..plus horz line end points
}
