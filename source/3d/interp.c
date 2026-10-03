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
 * Polygon object interpreter
 *
 * C port of interp.asm.
 *
 * The model data is a byte-code stream.  Each opcode starts with a 16-bit
 * opcode number; the layouts (byte offsets from the start of the opcode) are:
 *
 *  OP_EOF        0  short op
 *  OP_DEFPOINTS  0  short op, 2 short n, 4 vms_vector points[n]
 *  OP_FLATPOLY   0  short op, 2 short nv, 4 vms_vector point, 16 vms_vector normal,
 *                  28 short color, 30 short pointnums[nv] (padded to an odd count)
 *  OP_TMAPPOLY   0  short op, 2 short nv, 4 vms_vector point, 16 vms_vector normal,
 *                  28 short bitmap, 30 short pointnums[nv] (padded to an odd count),
 *                  followed by g3s_uvl uvls[nv]
 *  OP_SORTNORM   0  short op, 2 short pad, 4 vms_vector normal, 16 vms_vector point,
 *                  28 short front offset, 30 short back offset  (size 32)
 *  OP_RODBM      0  short op, 2 short bitmap, 4 vms_vector top point, 16 fix bot width,
 *                  20 vms_vector bot point, 32 fix top width  (size 36)
 *  OP_SUBCALL    0  short op, 2 short anim angle num, 4 vms_vector offset,
 *                  16 short offset of subobject, 18 pad  (size 20)
 *  OP_DEFP_START 0  short op, 2 short n, 4 short start point, 6 pad,
 *                  8 vms_vector points[n]
 *  OP_GLOW       0  short op, 2 short glow num  (size 4)
 *
 * All 16-bit fields are unsigned.  Offsets in SORTNORM and SUBCALL are
 * relative to the start of that opcode.
 */

#include "3dlocal.h"

extern int gr_find_closest_color_15bpp(int rgb);	//in the 2d library

#define OP_EOF				0	//eof
#define OP_DEFPOINTS		1	//defpoints
#define OP_FLATPOLY		2	//flat-shaded polygon
#define OP_TMAPPOLY		3	//texture-mapped polygon
#define OP_SORTNORM		4	//sort by normal
#define OP_RODBM			5	//rod bitmap
#define OP_SUBCALL		6	//call a subobject
#define OP_DEFP_START	7	//defpoints with start
#define OP_GLOW			8	//glow value for next poly

#define MAX_POINTS_PER_POLY 25

//access the fields of the model data
#define W(p)	(*((ushort *) (p)))			//unsigned 16-bit field
#define FX(p)	(*((fix *) (p)))				//32-bit fix field
#define VP(p)	((vms_vector *) (p))			//vector
#define UVLP(p)	((g3s_uvl *) (p))			//uvl list

//size of the point number list of a polygon, including the pad
#define POINTLIST_SIZE(nv)	((((size_t)(nv) & ~(size_t)1) + 1) * 2)

static grs_bitmap **bitmap_ptr;
static vms_angvec *anim_angles;		//pointer to angle data

static vms_vector *morph_points;		//alternate points for morph

//light value for the next tmap
static int glow_num = -1;				//-1 means off
static fix *glow_values;				//ptr to array of values

short highest_texture_num = 0;

static vms_angvec zero_angles = {0,0,0};	//for if no angles specified

static g3s_point rod_top_p,rod_bot_p;

int g3d_interp_outline = 0;			//if on, polygon models outlined in white

static g3s_point *morph_pointlist[3];

static g3s_uvl morph_uvls[3];

//the light for the current model
static fix model_light;

//ptr to array of points
static g3s_point *Interp_point_list;

static g3s_point *point_list[MAX_POINTS_PER_POLY];

//set when drawing a morphing model, to use the alternate opcode handlers
static int morphing = 0;

//give the interpreter an array of points to use
void g3_set_interp_points(g3s_point *pointlist)
{
	Interp_point_list = pointlist;
}

//rotate a list of points into the interpreter's point list
static void rotate_point_list(g3s_point *dest,vms_vector *src,int n)
{
	while (n--)
		g3_rotate_point(dest++,src++);
}

//make list of point pointers from a list of point numbers
static void make_point_list(ushort *pointnums,int nv)
{
	int i;

	for (i=0;i<nv;i++)
		point_list[i] = &Interp_point_list[pointnums[i]];
}

#ifndef NDEBUG
//draws the outline of a polygon. takes count, ptr to point number list
static void draw_outline(int nv,ushort *pointnums)
{
	int i;

	gr_setcolor(255);		//bright white

	for (i=0;i<nv-1;i++)
		g3_draw_line(&Interp_point_list[pointnums[i]],&Interp_point_list[pointnums[i+1]]);

	g3_draw_line(&Interp_point_list[pointnums[nv-1]],&Interp_point_list[pointnums[0]]);
}
#endif

//calculate light from surface normal: 1/4 + dot * 3/4, scaled by model light
static fix calc_light(vms_vector *norm)
{
	fix l;

	l = -vm_vec_dotprod(&View_matrix.fvec,norm);

	l = ((fix) ((ulong) l * 3)) >> 2;	//l *= 3/4
	l += f1_0/4;							//l = 1/4 + l * 3/4

	return fixmul(l,model_light);
}

//execute the opcodes starting at p, until OP_EOF
static void interp_execute(ubyte *p)
{
	for (;;) {
		switch (W(p)) {

			case OP_EOF:		//end of a model or sub-rountine
				return;

			case OP_DEFPOINTS: {		//define a list of points
				int n = W(p+2);

				if (morphing)
					rotate_point_list(Interp_point_list,morph_points,n);
				else
					rotate_point_list(Interp_point_list,VP(p+4),n);

				p += 4 + n*sizeof(vms_vector);
				break;
			}

			case OP_DEFP_START: {	//define a list of points, with starting point num specified
				int n = W(p+2);
				int s = W(p+4);

				//when morphing, the alternate points always start at the first one
				if (morphing)
					rotate_point_list(&Interp_point_list[s],morph_points,n);
				else
					rotate_point_list(&Interp_point_list[s],VP(p+8),n);

				p += 8 + n*sizeof(vms_vector);
				break;
			}

			case OP_FLATPOLY: {		//draw a flat-shaded polygon
				int nv = W(p+2);

				if (morphing) {
					ushort *pointnums = (ushort *) (p+30);
					int ntris;

					gr_setcolor(W(p+28));

					morph_pointlist[0] = &Interp_point_list[*pointnums++];
					morph_pointlist[1] = &Interp_point_list[*pointnums++];
					morph_pointlist[2] = &Interp_point_list[*pointnums++];

					//draw as a fan of triangles, computing the normal of each
					for (ntris=nv-2;ntris>0;ntris--) {
						g3_check_and_draw_poly(3,morph_pointlist,NULL,NULL);

						if (ntris > 1) {
							morph_pointlist[1] = morph_pointlist[2];
							morph_pointlist[2] = &Interp_point_list[*pointnums++];
						}
					}
				}
				else if (g3_check_normal_facing(VP(p+4),VP(p+16))) {

					//polygon is facing, so draw it

					gr_setcolor(W(p+28));		//set color

					make_point_list((ushort *) (p+30),nv);

					g3_draw_poly(nv,point_list);

					#ifndef NDEBUG
					if (g3d_interp_outline)
						draw_outline(nv,(ushort *) (p+30));
					#endif
				}

				//polygon is not facing (or we've plotted it). go to next opcode
				p += 30 + POINTLIST_SIZE(nv);
				break;
			}

			case OP_GLOW:		//set the glow value for the next tmap
				if (glow_values)
					glow_num = W(p+2);
				p += 4;
				break;

			case OP_TMAPPOLY: {		//draw a texture map
				int nv = W(p+2);
				g3s_uvl *uvl_list = UVLP(p+30+POINTLIST_SIZE(nv));

				if (morphing) {
					grs_bitmap *bm = bitmap_ptr[W(p+28)];
					ushort *pointnums = (ushort *) (p+30);
					fix light;
					int i,ntris;

					//calculate light from surface normal
					light = calc_light(VP(p+16));
					if (light < 0)
						light = 0;

					morph_uvls[0].l = morph_uvls[1].l = morph_uvls[2].l = light;

					morph_pointlist[0] = &Interp_point_list[*pointnums++];
					morph_pointlist[1] = &Interp_point_list[*pointnums++];
					morph_pointlist[2] = &Interp_point_list[*pointnums++];

					if (nv == 3) {			//3 points is good!

						//poke in light values
						for (i=0;i<3;i++)
							uvl_list[i].l = light;

						g3_check_and_draw_tmap(3,morph_pointlist,uvl_list,bm,NULL,NULL);
					}
					else if (nv > 3) {		//(less than 3 is bad, so abort)

						for (i=0;i<3;i++) {
							morph_uvls[i].u = uvl_list[i].u;
							morph_uvls[i].v = uvl_list[i].v;
						}
						uvl_list += 3;

						//draw as a fan of triangles, computing the normal of each
						for (ntris=nv-2;ntris>0;ntris--) {
							g3_check_and_draw_tmap(3,morph_pointlist,morph_uvls,bm,NULL,NULL);

							if (ntris > 1) {
								morph_pointlist[1] = morph_pointlist[2];
								morph_pointlist[2] = &Interp_point_list[*pointnums++];

								morph_uvls[1].u = morph_uvls[2].u;
								morph_uvls[1].v = morph_uvls[2].v;

								morph_uvls[2].u = uvl_list->u;
								morph_uvls[2].v = uvl_list->v;
								uvl_list++;
							}
						}
					}
				}
				else if (g3_check_normal_facing(VP(p+4),VP(p+16))) {
					grs_bitmap *bm;
					fix light;
					int i;

					//polygon is facing, so draw it

					bm = bitmap_ptr[W(p+28)];		//get bitmap

					//calculate light from surface normal

					if (glow_num >= 0) {		//glow override?
						//special glow lighting, which doesn't care about surface normal
						light = glow_values[glow_num];
						glow_num = -1;
					}
					else
						light = calc_light(VP(p+16));

					//now poke light into l values
					for (i=0;i<nv;i++)
						uvl_list[i].l = light;

					//now draw it

					make_point_list((ushort *) (p+30),nv);

					g3_draw_tmap(nv,point_list,uvl_list,bm);

					#ifndef NDEBUG
					if (g3d_interp_outline)
						draw_outline(nv,(ushort *) (p+30));
					#endif
				}

				//polygon is not facing (or we've plotted it). go to next opcode
				p += 30 + POINTLIST_SIZE(nv) + nv*sizeof(g3s_uvl);	//point past uvls
				break;
			}

			case OP_SORTNORM:		//sort based on surface normal

				if (g3_check_normal_facing(VP(p+16),VP(p+4))) {
					//is facing.  draw back then front
					interp_execute(p + W(p+30));
					interp_execute(p + W(p+28));
				}
				else {
					//is not facing.  draw front then back
					interp_execute(p + W(p+28));
					interp_execute(p + W(p+30));
				}

				p += 32;
				break;

			case OP_RODBM:		//draw a rod bitmap

				g3_rotate_point(&rod_bot_p,VP(p+20));
				g3_rotate_point(&rod_top_p,VP(p+4));

				//the asm passed whatever was in ecx as the light value; use full light
				g3_draw_rod_tmap(bitmap_ptr[W(p+2)],&rod_bot_p,FX(p+16),&rod_top_p,FX(p+32),f1_0);

				p += 36;
				break;

			case OP_SUBCALL: {		//draw a subobject
				vms_angvec *a;

				//get ptr to angles
				if (anim_angles)
					a = &anim_angles[W(p+2)];
				else
					a = &zero_angles;		//angles not specified.  Use zero angles

				g3_start_instance_angles(VP(p+4),a);

				interp_execute(p + W(p+16));	//draw the subobject

				g3_done_instance();

				p += 20;
				break;
			}

			default:		//invalid opcode (the asm would have jumped into the weeds)
				return;
		}
	}
}

//interpreter to draw polygon model
//takes ptr to object, ptr to array of bitmap pointers, ptr to anim angles,
//light value, ptr to array of glow values (or NULL)
bool g3_draw_polygon_model(void *model_ptr,grs_bitmap **model_bitmaps,vms_angvec *anim_angles_p,fix light,fix *glow_values_p)
{
	bitmap_ptr = model_bitmaps;		//save ptr to bitmap array
	anim_angles = anim_angles_p;
	model_light = light;
	glow_values = glow_values_p;

	glow_num = -1;

	interp_execute((ubyte *) model_ptr);

	return 1;
}

//interpreter to draw a morphing polygon model
//takes ptr to object, ptr to array of bitmap pointers, ptr to anim angles,
//light value, alternate points
bool g3_draw_morphing_model(void *model_ptr,grs_bitmap **model_bitmaps,vms_angvec *anim_angles_p,fix light,vms_vector *new_points)
{
	int save_morphing;

	bitmap_ptr = model_bitmaps;		//save ptr to bitmap array
	anim_angles = anim_angles_p;
	morph_points = new_points;
	model_light = light;

	//set alternate opcode handlers
	save_morphing = morphing;
	morphing = 1;

	interp_execute((ubyte *) model_ptr);

	morphing = save_morphing;

	return 1;
}

//initialize a polygon object
//translate colors, find the highest texture number
static void init_loop(ubyte *p)
{
	for (;;) {
		switch (W(p)) {

			case OP_EOF:
				return;

			case OP_DEFPOINTS:
			case OP_DEFP_START: {
				ulong n = W(p+2);
				ulong c;

				//count*3, computed like the asm (the add was only 16 bits)
				c = n << 1;
				c = (c & 0xffff0000ul) | ((c + n) & 0xffff);

				p += ((W(p) == OP_DEFPOINTS) ? 4 : 8) + c*4;
				break;
			}

			case OP_FLATPOLY:
				W(p+28) = (ushort) gr_find_closest_color_15bpp(W(p+28));	//translate color

				p += 30 + POINTLIST_SIZE(W(p+2));
				break;

			case OP_TMAPPOLY: {
				int nv = W(p+2);

				if ((short) W(p+28) > highest_texture_num)		//get bitmap number
					highest_texture_num = (short) W(p+28);

				p += 30 + POINTLIST_SIZE(nv) + nv*sizeof(g3s_uvl);	//skip point list and uvls
				break;
			}

			case OP_SORTNORM:
				init_loop(p + W(p+28));
				init_loop(p + W(p+30));
				p += 32;
				break;

			case OP_RODBM:
				p += 36;
				break;

			case OP_SUBCALL:
				init_loop(p + W(p+16));
				p += 20;
				break;

			case OP_GLOW:
				p += 4;
				break;

			default:		//invalid opcode (the asm would have looped forever)
				return;
		}
	}
}

void g3_init_polygon_model(void *model_ptr)
{
	highest_texture_num = -1;

	init_loop((ubyte *) model_ptr);
}
