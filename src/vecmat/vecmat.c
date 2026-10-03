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
 * Source for vector/matrix library
 *
 * C port of vecmat.asm (revision 1.54).  The arithmetic matches the
 * assembly exactly: 64-bit sums of products shifted down and truncated to
 * 32 bits, quad_sqrt() for magnitudes, the same approximation for the
 * "quick" functions, and the same handling of zero-length vectors.
 *
 */

#include <stdint.h>
#include <stddef.h>

#include "fix.h"
#include "vecmat.h"

//These should never be changed!
vms_vector vmd_zero_vector = {0,0,0};
vms_matrix vmd_identity_matrix = {	{f1_0,0,0},
												{0,f1_0,0},
												{0,0,f1_0} };

//Access to matrix elements by the names the assembly code used.
//<m1,m2,m3> is the first row, etc.
#define M1(m) ((m)->rvec.x)
#define M4(m) ((m)->rvec.y)
#define M7(m) ((m)->rvec.z)
#define M2(m) ((m)->uvec.x)
#define M5(m) ((m)->uvec.y)
#define M8(m) ((m)->uvec.z)
#define M3(m) ((m)->fvec.x)
#define M6(m) ((m)->fvec.y)
#define M9(m) ((m)->fvec.z)

//absolute value, with 0x80000000 left unchanged (like neg)
static fix abs_fix(fix v)
{
	return (v < 0) ? (fix) (0 - (uint32_t) v) : v;
}

//64-bit product of two fixes, as unsigned so sums wrap like add/adc
#define PROD(a,b) ((uint64_t) ((int64_t) (a) * (b)))

//returns the 64-bit sum of the squares of three fixes
static uint64_t sum_squares(fix x,fix y,fix z)
{
	return PROD(x,x) + PROD(y,y) + PROD(z,z);
}

//square root of a 64-bit sum of squares (negative/overflowed sums give 0)
static fix sqrt_sum(uint64_t sum)
{
	return (fix) quad_sqrt((long) (uint32_t) sum,(long) (uint32_t) (sum >> 32));
}

//sum of three products, shifted down to a fix (like imul,add/adc,shrd)
static fix dot3(fix x0,fix y0,fix z0,fix x1,fix y1,fix z1)
{
	return (fix) ((int64_t) (PROD(x0,x1) + PROD(y0,y1) + PROD(z0,z1)) >> 16);
}

#ifndef INLINE

//add two vectors, filling in dest
vms_vector *vm_vec_add(vms_vector *dest,vms_vector *src0,vms_vector *src1)
{
	dest->x = src0->x + src1->x;
	dest->y = src0->y + src1->y;
	dest->z = src0->z + src1->z;

	return dest;
}

//subtracts two vectors, filling in dest
vms_vector *vm_vec_sub(vms_vector *dest,vms_vector *src0,vms_vector *src1)
{
	dest->x = src0->x - src1->x;
	dest->y = src0->y - src1->y;
	dest->z = src0->z - src1->z;

	return dest;
}

//adds one vector to antother
vms_vector *vm_vec_add2(vms_vector *dest,vms_vector *src)
{
	dest->x += src->x;
	dest->y += src->y;
	dest->z += src->z;

	return dest;
}

//subtract one vector from another
vms_vector *vm_vec_sub2(vms_vector *dest,vms_vector *src)
{
	dest->x -= src->x;
	dest->y -= src->y;
	dest->z -= src->z;

	return dest;
}

#endif

//averages two vectors
vms_vector *vm_vec_avg(vms_vector *dest,vms_vector *src0,vms_vector *src1)
{
	dest->x = (src0->x + src1->x) >> 1;
	dest->y = (src0->y + src1->y) >> 1;
	dest->z = (src0->z + src1->z) >> 1;

	return dest;
}

//averages four vectors.  (This was assembled out of the DESCENT asm library)
vms_vector *vm_vec_avg4(vms_vector *dest,vms_vector *src0,vms_vector *src1,vms_vector *src2,vms_vector *src3)
{
	dest->x = (src0->x + src1->x + src2->x + src3->x) >> 2;
	dest->y = (src0->y + src1->y + src2->y + src3->y) >> 2;
	dest->z = (src0->z + src1->z + src2->z + src3->z) >> 2;

	return dest;
}

//scales a vector in place
vms_vector *vm_vec_scale(vms_vector *dest,fix s)
{
	dest->x = fixmul(dest->x,s);
	dest->y = fixmul(dest->y,s);
	dest->z = fixmul(dest->z,s);

	return dest;
}

//scales and copies a vector
vms_vector *vm_vec_copy_scale(vms_vector *dest,vms_vector *src,fix s)
{
	dest->x = fixmul(src->x,s);
	dest->y = fixmul(src->y,s);
	dest->z = fixmul(src->z,s);

	return dest;
}

//scales a vector, adds it to another, and stores in a 3rd
//dest = src1 + k * src2
vms_vector *vm_vec_scale_add(vms_vector *dest,vms_vector *src1,vms_vector *src2,fix k)
{
	dest->x = src1->x + fixmul(src2->x,k);
	dest->y = src1->y + fixmul(src2->y,k);
	dest->z = src1->z + fixmul(src2->z,k);

	return dest;
}

//scales a vector and adds it to another
//dest += k * src
vms_vector *vm_vec_scale_add2(vms_vector *dest,vms_vector *src,fix k)
{
	dest->x += fixmul(src->x,k);
	dest->y += fixmul(src->y,k);
	dest->z += fixmul(src->z,k);

	return dest;
}

//scales a vector in place, taking n/d for scale
vms_vector *vm_vec_scale2(vms_vector *dest,fix n,fix d)
{
	if (d == 0)			// @mk, 01/04/94, prevent divide overflow
		return dest;

	dest->x = fixmuldiv(dest->x,n,d);
	dest->y = fixmuldiv(dest->y,n,d);
	dest->z = fixmuldiv(dest->z,n,d);

	return dest;
}

//compute the distance between two points. (does sub and mag)
fix vm_vec_dist(vms_vector *v0,vms_vector *v1)
{
	return sqrt_sum(sum_squares(v0->x - v1->x,v0->y - v1->y,v0->z - v1->z));
}

//computes an approximation of the magnitude from three absolute values
//uses dist = largest + next_largest*3/8 + smallest*3/16
static fix mag_quick(fix a,fix b,fix c)
{
	fix t;

	//sort so a >= b >= c
	if (! (a > b)) {t=a; a=b; b=t;}
	if (! (b > c)) {
		t=b; b=c; c=t;
		if (! (a > b)) {t=a; a=b; b=t;}
	}

	b = (b >> 2) + (c >> 3);	//    b*1/4 + c*1/8
	a += b;							//a + b*1/4 + c*1/8
	b >>= 1;							//    b*1/8 + c*1/16
	a += b;							//a + b*3/8 + c*3/16

	return a;
}

//computes an approximation of the magnitude of a vector
//uses dist = largest + next_largest*3/8 + smallest*3/16
fix vm_vec_mag_quick(vms_vector *v)
{
	return mag_quick(abs_fix(v->x),abs_fix(v->y),abs_fix(v->z));
}

//computes an approximation of the distance between two points.
//uses dist = largest + next_largest*3/8 + smallest*3/16
fix vm_vec_dist_quick(vms_vector *v0,vms_vector *v1)
{
	return mag_quick(abs_fix(v0->z - v1->z),abs_fix(v0->x - v1->x),abs_fix(v0->y - v1->y));
}

//compute magnitude of vector
fix vm_vec_mag(vms_vector *v)
{
	return sqrt_sum(sum_squares(v->x,v->y,v->z));
}

//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of dir vec
//NOTE: the order of the parameters matches the vector subtraction
fix vm_vec_normalized_dir(vms_vector *dest,vms_vector *end,vms_vector *start)
{
	fix mag;

	dest->x = end->x - start->x;
	dest->y = end->y - start->y;
	dest->z = end->z - start->z;

	mag = sqrt_sum(sum_squares(dest->x,dest->y,dest->z));

	if (mag != 0) {
		dest->x = fixdiv(dest->x,mag);
		dest->y = fixdiv(dest->y,mag);
		dest->z = fixdiv(dest->z,mag);
	}

	return mag;
}

//normalize a vector.  returns mag of source vec.
//if the source has zero length, dest is left unchanged
fix vm_vec_copy_normalize(vms_vector *dest,vms_vector *src)
{
	fix mag;

	mag = sqrt_sum(sum_squares(src->x,src->y,src->z));

	if (mag != 0) {
		dest->x = fixdiv(src->x,mag);
		dest->y = fixdiv(src->y,mag);
		dest->z = fixdiv(src->z,mag);
	}

	return mag;
}

//normalize a vector in place.  returns mag of source vec
fix vm_vec_normalize(vms_vector *v)
{
	return vm_vec_copy_normalize(v,v);
}

//normalize a vector.  returns mag of source vec.  uses approx. dist
fix vm_vec_copy_normalize_quick(vms_vector *dest,vms_vector *src)
{
	fix mag;

	mag = vm_vec_mag_quick(src);

	if (mag != 0) {
		dest->x = fixdiv(src->x,mag);
		dest->y = fixdiv(src->y,mag);
		dest->z = fixdiv(src->z,mag);
	}

	return mag;
}

//normalize a vector in place.  returns mag of source vec.  uses approx. dist
fix vm_vec_normalize_quick(vms_vector *v)
{
	return vm_vec_copy_normalize_quick(v,v);
}

//same as vm_vec_normalized_dir, but with quick sqrt
fix vm_vec_normalized_dir_quick(vms_vector *dest,vms_vector *end,vms_vector *start)
{
	vm_vec_sub(dest,end,start);

	return vm_vec_normalize_quick(dest);
}

//compute dot product of two vectors.  On overflow, returns a saturated value
fix vm_vec_dotprod(vms_vector *v0,vms_vector *v1)
{
	int64_t sum;
	fix result;

	sum = (int64_t) (PROD(v0->x,v1->x) + PROD(v0->y,v1->y) + PROD(v0->z,v1->z));

	result = (fix) (sum >> 16);

	//always do overflow check, and return saturated value.  The result is
	//ok if the high 16 bits of the sum match the sign of the result
	if ((int16_t) (sum >> 48) != ((result < 0) ? -1 : 0))
		result = (sum < 0) ? -0x7fffffff : 0x7fffffff;

	return result;
}

//computes cross product of two vectors.  Note: this magnitude of the
//resultant vector is the product of the magnitudes of the two source
//vectors.  This means it is quite easy for this routine to overflow and
//underflow.  Be careful that your inputs are ok.
vms_vector *vm_vec_crossprod(vms_vector *dest,vms_vector *src0,vms_vector *src1)
{
	//Assert(dest!=src0 && dest!=src1);

	dest->x = (fix) ((int64_t) (PROD(src1->z,src0->y) - PROD(src1->y,src0->z)) >> 16);
	dest->y = (fix) ((int64_t) (PROD(src1->x,src0->z) - PROD(src1->z,src0->x)) >> 16);
	dest->z = (fix) ((int64_t) (PROD(src1->y,src0->x) - PROD(src1->x,src0->y)) >> 16);

	return dest;
}

//make sure a vector is reasonably sized to go into a cross product
static void check_vec(vms_vector *v)
{
	fix bits;
	int cnt = 0;

	bits = abs_fix(v->x) | abs_fix(v->y) | abs_fix(v->z);

	if (bits == 0)		//null vector
		return;

	if (bits & 0xfffc0000) {		//too big

		while (bits & 0x00f00000) {
			cnt += 4;
			bits >>= 4;
		}

		while (bits & 0xfffc0000) {
			cnt += 2;
			bits >>= 2;
		}

		v->x >>= cnt;
		v->y >>= cnt;
		v->z >>= cnt;
	}
	else if (! (bits & 0xffff8000)) {		//maybe too small...

		while (! (bits & 0xfffff000)) {
			cnt += 4;
			bits <<= 4;
		}

		while (! (bits & 0xffff8000)) {
			cnt += 2;
			bits <<= 2;
		}

		v->x = (fix) ((uint32_t) v->x << cnt);
		v->y = (fix) ((uint32_t) v->y << cnt);
		v->z = (fix) ((uint32_t) v->z << cnt);
	}
}

//computes surface normal from three points.  Result vector is NOT
//normalized, but this routine does make an effort that cross product does
//not overflow or underflow
vms_vector *vm_vec_perp(vms_vector *dest,vms_vector *p0,vms_vector *p1,vms_vector *p2)
{
	vms_vector t0,t1;

	vm_vec_sub(&t1,p2,p1);
	vm_vec_sub(&t0,p1,p0);

	check_vec(&t0);
	check_vec(&t1);

	return vm_vec_crossprod(dest,&t0,&t1);
}

//computes surface normal from three points.  Result vector is normalized.
vms_vector *vm_vec_normal(vms_vector *dest,vms_vector *p0,vms_vector *p1,vms_vector *p2)
{
	vm_vec_perp(dest,p0,p1,p2);

	vm_vec_normalize(dest);

	return dest;
}

//compute a rotation matrix from sines & cosines of the three angles
static vms_matrix *sincos_2_matrix(vms_matrix *m,fix sinp,fix cosp,fix sinb,fix cosb,fix sinh,fix cosh)
{
	fix sbsh,cbch,cbsh,sbch;

	sbsh = fixmul(sinb,sinh);
	cbch = fixmul(cosb,cosh);
	M1(m) = cbch + fixmul(sbsh,sinp);		//m1=cbch+sbspsh
	M8(m) = fixmul(cbch,sinp) + sbsh;		//m8=sbsh+cbchsp

	cbsh = fixmul(cosb,sinh);
	sbch = fixmul(sinb,cosh);
	M2(m) = fixmul(cbsh,sinp) - sbch;		//m2=cbshsp-sbch
	M7(m) = fixmul(sbch,sinp) - cbsh;		//m7=sbchsp-cbsh

	M3(m) = fixmul(sinh,cosp);					//m3=shcp
	M4(m) = fixmul(sinb,cosp);					//m4=sbcp
	M5(m) = fixmul(cosb,cosp);					//m5=cbcp
	M6(m) = -sinp;									//m6=-sp
	M9(m) = fixmul(cosh,cosp);					//m9=chcp

	return m;
}

//compute a rotation matrix from three angles
vms_matrix *vm_angles_2_matrix(vms_matrix *m,vms_angvec *a)
{
	fix sinp,cosp,sinb,cosb,sinh,cosh;

	fix_sincos(a->p,&sinp,&cosp);
	fix_sincos(a->b,&sinb,&cosb);
	fix_sincos(a->h,&sinh,&cosh);

	return sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);
}

//compute a rotation matrix from the forward vector and a rotation around
//that vector
vms_matrix *vm_vec_ang_2_matrix(vms_matrix *m,vms_vector *v,fixang a)
{
	fix sinp,cosp,sinb,cosb,sinh,cosh;

	fix_sincos(a,&sinb,&cosb);

	//extract heading & pitch from vector

	sinp = -v->y;										//m6=-sp
	cosp = fix_sqrt(f1_0 - fixmul(sinp,sinp));
	sinh = fixdiv(v->x,cosp);
	cosh = fixdiv(v->z,cosp);

	return sincos_2_matrix(m,sinp,cosp,sinb,cosb,sinh,cosh);
}

//build a matrix from only the forward vector, assuming zero bank.
//returns 0 if the vector has zero length (in which case the matrix is
//left unchanged)
static int forward_vec_2_matrix(vms_matrix *m,vms_vector *fvec,int normalize)
{
	vms_vector xvec,yvec,zvec;

	zvec = *fvec;
	if (normalize)
		if (vm_vec_normalize(&zvec) == 0)
			return 0;		//0-len vec in vec_2_mat

	m->fvec = zvec;

	if ((zvec.x | zvec.z) == 0) {		//check both x & z == 0

		//forward vector is straight up (or down)

		M1(m) = f1_0;
		M8(m) = (zvec.y < 0) ? f1_0 : -f1_0;
		M4(m) = M7(m) = M2(m) = M5(m) = 0;
	}
	else {
		xvec.x = zvec.z;
		xvec.y = 0;
		xvec.z = -zvec.x;
		vm_vec_normalize(&xvec);

		m->rvec = xvec;

		vm_vec_crossprod(&yvec,&zvec,&xvec);

		m->uvec = yvec;
	}

	return 1;
}

//create a rotation matrix from one or two vectors.
//requires forward vec, and assumes zero bank if up & right vecs==NULL
//up/right vector need not be exactly perpendicular to forward vec
//Note: this routine loses precision as the forward vector approaches
//straigt up or down (I think)
vms_matrix *vm_vector_2_matrix(vms_matrix *m,vms_vector *fvec,vms_vector *uvec,vms_vector *rvec)
{
	vms_vector xvec,yvec,zvec;

	//Assert(fvec != NULL);

	if (uvec != NULL) {				//use forward and up vectors

		zvec = *fvec;
		if (vm_vec_normalize(&zvec) == 0)
			goto bad_vector2;

		yvec = *uvec;
		if (vm_vec_normalize(&yvec) == 0)
			goto bad_vector2;

		vm_vec_crossprod(&xvec,&yvec,&zvec);	//get x vector

		//normalize new perpendicular vector
		if (vm_vec_normalize(&xvec) == 0)
			goto bad_vector2;

		//now recompute up vector, in case it wasn't entirely perpendiclar
		vm_vec_crossprod(&yvec,&zvec,&xvec);
	}
	else if (rvec != NULL) {		//use forward and right vectors

		zvec = *fvec;
		if (vm_vec_normalize(&zvec) == 0)
			goto bad_vector2;

		xvec = *rvec;
		if (vm_vec_normalize(&xvec) == 0)
			goto bad_vector2;

		vm_vec_crossprod(&yvec,&zvec,&xvec);	//get y = z cross x

		//normalize new perpendicular vector
		if (vm_vec_normalize(&yvec) == 0)
			goto bad_vector2;

		//now recompute right vector, in case it wasn't entirely perpendiclar
		vm_vec_crossprod(&xvec,&yvec,&zvec);	//x = y cross z
	}
	else {
		//only the forward vector is present
		forward_vec_2_matrix(m,fvec,1);
		return m;
	}

	m->rvec = xvec;
	m->uvec = yvec;
	m->fvec = zvec;

	return m;

	//one of the non-forward vectors caused a problem, so ignore them and
	//use just the forward vector (as normalized above)
bad_vector2:
	forward_vec_2_matrix(m,&zvec,1);
	return m;
}

//this version of vector_2_matrix requires that the vectors be more-or-less
//normalized and close to perpendicular.  (This was assembled out of the
//DESCENT asm library)
vms_matrix *vm_vector_2_matrix_norm(vms_matrix *m,vms_vector *fvec,vms_vector *uvec,vms_vector *rvec)
{
	vms_vector xvec,yvec,zvec;

	//Assert(fvec != NULL);

	if (uvec != NULL) {				//use forward and up vectors

		zvec = *fvec;
		yvec = *uvec;

		vm_vec_crossprod(&xvec,&yvec,&zvec);	//get x vector

		//normalize new perpendicular vector
		if (vm_vec_normalize(&xvec) == 0)
			goto bad_vector2;

		//now recompute up vector, in case it wasn't entirely perpendiclar
		vm_vec_crossprod(&yvec,&zvec,&xvec);
	}
	else if (rvec != NULL) {		//use forward and right vectors

		zvec = *fvec;
		xvec = *rvec;

		vm_vec_crossprod(&yvec,&zvec,&xvec);	//get y = z cross x

		//normalize new perpendicular vector
		if (vm_vec_normalize(&yvec) == 0)
			goto bad_vector2;

		//now recompute right vector, in case it wasn't entirely perpendiclar
		vm_vec_crossprod(&xvec,&yvec,&zvec);	//x = y cross z
	}
	else {
		//only the forward vector is present
		forward_vec_2_matrix(m,fvec,0);
		return m;
	}

	m->rvec = xvec;
	m->uvec = yvec;
	m->fvec = zvec;

	return m;

	//one of the non-forward vectors caused a problem, so ignore them and
	//use just the forward vector
bad_vector2:
	forward_vec_2_matrix(m,&zvec,0);
	return m;
}

//rotate a vector by a rotation matrix
vms_vector *vm_vec_rotate(vms_vector *dest,vms_vector *src,vms_matrix *m)
{
	//Assert(dest != src);

	dest->x = dot3(src->x,src->y,src->z,M1(m),M4(m),M7(m));
	dest->y = dot3(src->x,src->y,src->z,M2(m),M5(m),M8(m));
	dest->z = dot3(src->x,src->y,src->z,M3(m),M6(m),M9(m));

	return dest;
}

//transpose a matrix in place
vms_matrix *vm_transpose_matrix(vms_matrix *m)
{
	fix t;

	t = M2(m); M2(m) = M4(m); M4(m) = t;
	t = M3(m); M3(m) = M7(m); M7(m) = t;
	t = M6(m); M6(m) = M8(m); M8(m) = t;

	return m;
}

//copy and transpose a matrix
vms_matrix *vm_copy_transpose_matrix(vms_matrix *dest,vms_matrix *src)
{
	M1(dest) = M1(src);
	M4(dest) = M2(src);
	M7(dest) = M3(src);
	M2(dest) = M4(src);
	M5(dest) = M5(src);
	M8(dest) = M6(src);
	M3(dest) = M7(src);
	M6(dest) = M8(src);
	M9(dest) = M9(src);

	return dest;
}

//mulitply 2 matrices, fill in dest.  returns ptr to dest
vms_matrix *vm_matrix_x_matrix(vms_matrix *dest,vms_matrix *src0,vms_matrix *src1)
{
	//Assert(dest!=src0 && dest!=src1);

	M1(dest) = dot3(M1(src0),M2(src0),M3(src0),M1(src1),M4(src1),M7(src1));
	M2(dest) = dot3(M1(src0),M2(src0),M3(src0),M2(src1),M5(src1),M8(src1));
	M3(dest) = dot3(M1(src0),M2(src0),M3(src0),M3(src1),M6(src1),M9(src1));

	M4(dest) = dot3(M4(src0),M5(src0),M6(src0),M1(src1),M4(src1),M7(src1));
	M5(dest) = dot3(M4(src0),M5(src0),M6(src0),M2(src1),M5(src1),M8(src1));
	M6(dest) = dot3(M4(src0),M5(src0),M6(src0),M3(src1),M6(src1),M9(src1));

	M7(dest) = dot3(M7(src0),M8(src0),M9(src0),M1(src1),M4(src1),M7(src1));
	M8(dest) = dot3(M7(src0),M8(src0),M9(src0),M2(src1),M5(src1),M8(src1));
	M9(dest) = dot3(M7(src0),M8(src0),M9(src0),M3(src1),M6(src1),M9(src1));

	return dest;
}

//computes the delta angle between two normalized vectors.
//if the forward vector is NULL, the absolute values of the delta angle
//is returned.  If it is specified, the rotation around that vector from
//v0 to v1 is returned.
fixang vm_vec_delta_ang_norm(vms_vector *v0,vms_vector *v1,vms_vector *fvec)
{
	vms_vector t;
	fixang a;

	a = fix_acos(vm_vec_dotprod(v0,v1));

	if (fvec) {
		//do cross product to find sign of angle
		vm_vec_crossprod(&t,v0,v1);
		if (vm_vec_dotprod(&t,fvec) < 0)
			a = -a;
	}

	return a;
}

//computes the delta angle between two vectors.  Note that the vectors
//are normalized IN PLACE.
fixang vm_vec_delta_ang(vms_vector *v0,vms_vector *v1,vms_vector *fvec)
{
	vm_vec_normalize(v0);
	vm_vec_normalize(v1);

	return vm_vec_delta_ang_norm(v0,v1,fvec);
}

//compute the distance from a point to a plane.  takes the normalized normal
//of the plane, a point on the plane, and the point to check.
//distance is signed, so negative dist is on the back of the plane
fix vm_dist_to_plane(vms_vector *checkp,vms_vector *norm,vms_vector *planep)
{
	vms_vector t;

	vm_vec_sub(&t,checkp,planep);

	return vm_vec_dotprod(&t,norm);
}

//extract the angles from a matrix
vms_angvec *vm_extract_angles_matrix(vms_angvec *a,vms_matrix *m)
{
	fix sinh,cosh,cosp,sinp,sinb,cosb;

	//extract heading & pitch from forward vector

	if ((m->fvec.x | m->fvec.z) == 0)
		a->h = 0;									//zero, use head=0
	else
		a->h = fix_atan2(m->fvec.z,m->fvec.x);

	fix_sincos(a->h,&sinh,&cosh);			//get back sh

	if (abs_fix(cosh) > abs_fix(sinh))	//which is larger?
		cosp = fixdiv(m->fvec.z,cosh);		//cosine is larger: cp = chcp / ch
	else
		cosp = fixdiv(M3(m),sinh);			//sine is larger: cp = shcp / sh

	sinp = -m->fvec.y;						//fvec.y = -sp

	if ((sinp | cosp) == 0)
		a->p = 0;									//bogus vec, set p=0
	else
		a->p = fix_atan2(cosp,sinp);

	if (cosp == 0)						//the cosine of pitch is zero.  we're
		a->b = 0;						//pitched straight up. say no bank
	else {
		sinb = fixdiv(M4(m),cosp);			//m4 = sbcp
		cosb = fixdiv(M5(m),cosp);			//m5 = cbcp

		if ((sinb | cosb) == 0)
			a->b = 0;								//bogus vec, set b=0
		else
			a->b = fix_atan2(cosb,sinb);
	}

	return a;
}

//extract the angles from a normalized vector, assuming zero bank
vms_angvec *vm_extract_angles_vector_normalized(vms_angvec *a,vms_vector *v)
{
	a->b = 0;										//always zero bank

	a->p = fix_asin(-v->y);						//p = asin(-y)

	if ((v->x | v->z) == 0)					//check for up vector
		a->h = 0;
	else
		a->h = fix_atan2(v->z,v->x);			//h = atan2(x,z)

	return a;
}

//extract the angles from a vector, assuming zero bank.
//if the vector has zero length, the angles are left unchanged
vms_angvec *vm_extract_angles_vector(vms_angvec *a,vms_vector *v)
{
	vms_vector t;

	if (vm_vec_copy_normalize(&t,v) != 0)
		vm_extract_angles_vector_normalized(a,&t);

	return a;
}
