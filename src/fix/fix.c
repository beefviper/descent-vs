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
 * Fixed-point routines
 *
 * C port of fix.asm (revision 1.16).  Results are bit-identical to the
 * assembly version: same tables, same interpolation, same Newton iteration.
 *
 */

#include <stdint.h>

#include "fix.h"

//sine table, 256 steps per circle (plus the extra entries the cosine
//table needs).  The cosine table starts 64 entries (90 degrees) in.
static const short sincos_table[64+257] = {
	0, 402, 804, 1205, 1606, 2006, 2404, 2801,
	3196, 3590, 3981, 4370, 4756, 5139, 5520, 5897,
	6270, 6639, 7005, 7366, 7723, 8076, 8423, 8765,
	9102, 9434, 9760, 10080, 10394, 10702, 11003, 11297,
	11585, 11866, 12140, 12406, 12665, 12916, 13160, 13395,
	13623, 13842, 14053, 14256, 14449, 14635, 14811, 14978,
	15137, 15286, 15426, 15557, 15679, 15791, 15893, 15986,
	16069, 16143, 16207, 16261, 16305, 16340, 16364, 16379,
	16384, 16379, 16364, 16340, 16305, 16261, 16207, 16143,
	16069, 15986, 15893, 15791, 15679, 15557, 15426, 15286,
	15137, 14978, 14811, 14635, 14449, 14256, 14053, 13842,
	13623, 13395, 13160, 12916, 12665, 12406, 12140, 11866,
	11585, 11297, 11003, 10702, 10394, 10080, 9760, 9434,
	9102, 8765, 8423, 8076, 7723, 7366, 7005, 6639,
	6270, 5897, 5520, 5139, 4756, 4370, 3981, 3590,
	3196, 2801, 2404, 2006, 1606, 1205, 804, 402,
	0, -402, -804, -1205, -1606, -2006, -2404, -2801,
	-3196, -3590, -3981, -4370, -4756, -5139, -5520, -5897,
	-6270, -6639, -7005, -7366, -7723, -8076, -8423, -8765,
	-9102, -9434, -9760, -10080, -10394, -10702, -11003, -11297,
	-11585, -11866, -12140, -12406, -12665, -12916, -13160, -13395,
	-13623, -13842, -14053, -14256, -14449, -14635, -14811, -14978,
	-15137, -15286, -15426, -15557, -15679, -15791, -15893, -15986,
	-16069, -16143, -16207, -16261, -16305, -16340, -16364, -16379,
	-16384, -16379, -16364, -16340, -16305, -16261, -16207, -16143,
	-16069, -15986, -15893, -15791, -15679, -15557, -15426, -15286,
	-15137, -14978, -14811, -14635, -14449, -14256, -14053, -13842,
	-13623, -13395, -13160, -12916, -12665, -12406, -12140, -11866,
	-11585, -11297, -11003, -10702, -10394, -10080, -9760, -9434,
	-9102, -8765, -8423, -8076, -7723, -7366, -7005, -6639,
	-6270, -5897, -5520, -5139, -4756, -4370, -3981, -3590,
	-3196, -2801, -2404, -2006, -1606, -1205, -804, -402,
	0, 402, 804, 1205, 1606, 2006, 2404, 2801,
	3196, 3590, 3981, 4370, 4756, 5139, 5520, 5897,
	6270, 6639, 7005, 7366, 7723, 8076, 8423, 8765,
	9102, 9434, 9760, 10080, 10394, 10702, 11003, 11297,
	11585, 11866, 12140, 12406, 12665, 12916, 13160, 13395,
	13623, 13842, 14053, 14256, 14449, 14635, 14811, 14978,
	15137, 15286, 15426, 15557, 15679, 15791, 15893, 15986,
	16069, 16143, 16207, 16261, 16305, 16340, 16364, 16379,
	16384
};

static const short asin_table[258] = {
	0, 41, 81, 122, 163, 204, 244, 285,
	326, 367, 408, 448, 489, 530, 571, 612,
	652, 693, 734, 775, 816, 857, 897, 938,
	979, 1020, 1061, 1102, 1143, 1184, 1225, 1266,
	1307, 1348, 1389, 1431, 1472, 1513, 1554, 1595,
	1636, 1678, 1719, 1760, 1802, 1843, 1884, 1926,
	1967, 2009, 2050, 2092, 2134, 2175, 2217, 2259,
	2300, 2342, 2384, 2426, 2468, 2510, 2551, 2593,
	2636, 2678, 2720, 2762, 2804, 2847, 2889, 2931,
	2974, 3016, 3059, 3101, 3144, 3187, 3229, 3272,
	3315, 3358, 3401, 3444, 3487, 3530, 3573, 3617,
	3660, 3704, 3747, 3791, 3834, 3878, 3922, 3965,
	4009, 4053, 4097, 4142, 4186, 4230, 4275, 4319,
	4364, 4408, 4453, 4498, 4543, 4588, 4633, 4678,
	4723, 4768, 4814, 4859, 4905, 4951, 4997, 5043,
	5089, 5135, 5181, 5228, 5274, 5321, 5367, 5414,
	5461, 5508, 5556, 5603, 5651, 5698, 5746, 5794,
	5842, 5890, 5938, 5987, 6035, 6084, 6133, 6182,
	6231, 6281, 6330, 6380, 6430, 6480, 6530, 6580,
	6631, 6681, 6732, 6783, 6835, 6886, 6938, 6990,
	7042, 7094, 7147, 7199, 7252, 7306, 7359, 7413,
	7466, 7521, 7575, 7630, 7684, 7740, 7795, 7851,
	7907, 7963, 8019, 8076, 8133, 8191, 8249, 8307,
	8365, 8424, 8483, 8543, 8602, 8663, 8723, 8784,
	8846, 8907, 8970, 9032, 9095, 9159, 9223, 9288,
	9353, 9418, 9484, 9551, 9618, 9686, 9754, 9823,
	9892, 9963, 10034, 10105, 10177, 10251, 10324, 10399,
	10475, 10551, 10628, 10706, 10785, 10866, 10947, 11029,
	11113, 11198, 11284, 11371, 11460, 11550, 11642, 11736,
	11831, 11929, 12028, 12130, 12234, 12340, 12449, 12561,
	12677, 12796, 12919, 13046, 13178, 13315, 13459, 13610,
	13770, 13939, 14121, 14319, 14538, 14786, 15079, 15462,
	16384, 16384	//extra for when exactly 1
};

static const short acos_table[258] = {
	16384, 16343, 16303, 16262, 16221, 16180, 16140, 16099,
	16058, 16017, 15976, 15936, 15895, 15854, 15813, 15772,
	15732, 15691, 15650, 15609, 15568, 15527, 15487, 15446,
	15405, 15364, 15323, 15282, 15241, 15200, 15159, 15118,
	15077, 15036, 14995, 14953, 14912, 14871, 14830, 14789,
	14748, 14706, 14665, 14624, 14582, 14541, 14500, 14458,
	14417, 14375, 14334, 14292, 14250, 14209, 14167, 14125,
	14084, 14042, 14000, 13958, 13916, 13874, 13833, 13791,
	13748, 13706, 13664, 13622, 13580, 13537, 13495, 13453,
	13410, 13368, 13325, 13283, 13240, 13197, 13155, 13112,
	13069, 13026, 12983, 12940, 12897, 12854, 12811, 12767,
	12724, 12680, 12637, 12593, 12550, 12506, 12462, 12419,
	12375, 12331, 12287, 12242, 12198, 12154, 12109, 12065,
	12020, 11976, 11931, 11886, 11841, 11796, 11751, 11706,
	11661, 11616, 11570, 11525, 11479, 11433, 11387, 11341,
	11295, 11249, 11203, 11156, 11110, 11063, 11017, 10970,
	10923, 10876, 10828, 10781, 10733, 10686, 10638, 10590,
	10542, 10494, 10446, 10397, 10349, 10300, 10251, 10202,
	10153, 10103, 10054, 10004, 9954, 9904, 9854, 9804,
	9753, 9703, 9652, 9601, 9549, 9498, 9446, 9394,
	9342, 9290, 9237, 9185, 9132, 9078, 9025, 8971,
	8918, 8863, 8809, 8754, 8700, 8644, 8589, 8533,
	8477, 8421, 8365, 8308, 8251, 8193, 8135, 8077,
	8019, 7960, 7901, 7841, 7782, 7721, 7661, 7600,
	7538, 7477, 7414, 7352, 7289, 7225, 7161, 7096,
	7031, 6966, 6900, 6833, 6766, 6698, 6630, 6561,
	6492, 6421, 6350, 6279, 6207, 6133, 6060, 5985,
	5909, 5833, 5756, 5678, 5599, 5518, 5437, 5355,
	5271, 5186, 5100, 5013, 4924, 4834, 4742, 4648,
	4553, 4455, 4356, 4254, 4150, 4044, 3935, 3823,
	3707, 3588, 3465, 3338, 3206, 3069, 2925, 2774,
	2614, 2445, 2263, 2065, 1846, 1598, 1305, 922,
	0, 0	//extra for when exactly 1
};

#define sin_table	(sincos_table)
#define cos_table	(sincos_table+64)

//values for first guess in square root routines.  Note that the first entry
//is useful in quad_sqrt when edx=0 and high bit of eax is set.
static const ubyte guess_table[256] = {
	1,														//0
	1,1,1,												//1..3
	2,2,2,2,2,											//4..8
	3,3,3,3,3,3,3,									//9..15
	4,4,4,4,4,4,4,4,4,								//16..24
	5,5,5,5,5,5,5,5,5,5,5,							//25..35
	6,6,6,6,6,6,6,6,6,6,6,6,6,					//36..48
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,				//49..63
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,			//64..80
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,	//81..99
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,	//100..120
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,	//121..143
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,	//144..168
	13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,	//169..195
	14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,	//196..224
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15	//225..255
};

//compute sine and cosine of an angle, without interpolation.
//only the low 16 bits of the angle are used.  Either pointer can be NULL
void fix_fastsincos(fix a,fix *s,fix *c)
{
	int i = (a >> 8) & 0xff;	//get high byte

	if (s) *s = (fix) sin_table[i] * 4;	//make a fix
	if (c) *c = (fix) cos_table[i] * 4;
}

//compute sine and cosine of an angle, interpolating between table entries.
//only the low 16 bits of the angle are used.  Either pointer can be NULL
void fix_sincos(fix a,fix *s,fix *c)
{
	int i = (a >> 8) & 0xff;	//get high byte
	int f = a & 0xff;				//low byte is fraction
	fix t;

	if (s) {
		t = sin_table[i];
		t += ((sin_table[i+1] - t) * f) >> 8;	//add in frac part
		*s = t * 4;										//make a fix
	}

	if (c) {
		t = cos_table[i];
		t += ((cos_table[i+1] - t) * f) >> 8;
		*c = t * 4;
	}
}

//look up v (which must be in 0..f1_0) in an arcsine/arccosine table,
//interpolating between entries
static fix arc_lookup(const short *table,fix v)
{
	int i = v >> 8;		//get high byte (+1 bit)
	int f = v & 0xff;		//low byte is fraction
	fix t;

	t = table[i];
	t += ((table[i+1] - t) * f) >> 8;		//add in frac part

	return t;
}

//get absolute value of v, saturating at 1.0.  Returns sign mask (0 or -1)
//in *sign.  Done unsigned so that 0x80000000 also saturates.
static fix arc_abs(fix v,fix *sign)
{
	uint32_t u;

	*sign = (v < 0) ? -1 : 0;
	u = (uint32_t) v;
	if (v < 0)
		u = 0 - u;
	if (u > 0x10000)
		u = 0x10000;

	return (fix) u;
}

//takes cos of angle, returns angle
fixang fix_acos(fix v)
{
	fix sign,t;

	t = arc_lookup(acos_table,arc_abs(v,&sign));

	t = (t ^ sign) - sign;		//make correct sign
	t += sign & 0x8000;			//zero or 1/2

	return (fixang) t;
}

//takes sin of angle, returns angle
fixang fix_asin(fix v)
{
	fix sign,t;

	t = arc_lookup(asin_table,arc_abs(v,&sign));

	t = (t ^ sign) - sign;		//make correct sign

	return (fixang) t;
}

//given cos & sin of an angle, return that angle.
//parms need not be normalized, that is, the ratio cos/sin must
//equal the ratio of the actual cos & sin, but the parms need not be the
//actual cos & sin.
//NOTE: this is different from the standard C atan2, since it is left-handed.
//uses either asin or acos, to get better precision
fixang fix_atan2(fix cos,fix sin)
{
	uint64_t sum;
	fix mag,t,abs_cos,abs_sin;

	//Assert(cos!=0 || sin!=0);		//both parms to atan2 are zero!

	//sum of squares, as a 64-bit quantity (wraps like the add/adc pair)
	sum = (uint64_t) ((int64_t) cos * cos) + (uint64_t) ((int64_t) sin * sin);
	mag = (fix) quad_sqrt((long) (uint32_t) sum,(long) (uint32_t) (sum >> 32));

	//find smaller of two
	abs_cos = (cos < 0) ? (fix) (0 - (uint32_t) cos) : cos;
	abs_sin = (sin < 0) ? (fix) (0 - (uint32_t) sin) : sin;

	if (abs_cos >= abs_sin) {

		//sin is smaller, use arcsin

		if (mag == 0)
			return (fixang) sin;		//abort!

		t = fix_asin(fixdiv(sin,mag));

		if (cos < 0)				//check sign of cos
			t = 0x8000 - t;		//adjust

		return (fixang) t;
	}
	else {

		//cos is smaller, use arccos

		t = fix_acos(fixdiv(cos,mag));

		if (sin < 0)				//make sign correct
			t = -t;

		return (fixang) t;
	}
}

//standard Newtonian-iteration square root routine.  takes a long, returns
//a short.  Zero or negative input returns zero.
ushort long_sqrt(long a)
{
	uint32_t n,q,r,g,old_g;
	int cnt;

	if (a <= 0)					//zero or negative
		return 0;

	n = (uint32_t) a;

	//get a good first quess by checking which byte most significant bit is in
	if (n & 0xff000000)
		g = (uint32_t) guess_table[n >> 24] << 12;
	else if (n & 0x00ff0000)
		g = (uint32_t) guess_table[(n >> 16) & 0xff] << 8;
	else if (n & 0x0000ff00)
		g = (uint32_t) guess_table[(n >> 8) & 0xff] << 4;
	else
		g = guess_table[n & 0xff];

	//the loop nearly always executes 3 times, so the asm unrolled it 2 times
	//and did not do any checking until after the third time.  The quotient
	//always fits in 16 bits, so a 32-bit divide gives the same results as
	//the asm's 32/16 divide.

	for (cnt=0;cnt<2;cnt++) {
		q = n / g;
		g = (g + q) >> 1;			//next guess = (d + q)/2
	}

	for (;;) {
		q = n / g;
		r = n % g;
		if (q == g)					//correct?
			return (ushort) q;	//..yep
		old_g = g;					//save for compare
		g = (g + q) >> 1;			//next guess = (d + q)/2
		if (g == q || g == old_g)
			break;
	}

	//almost got it
	if (r)							//check remainder
		g++;

	return (ushort) g;
}

//standard Newtonian-iteration square root routine.  takes high:low as a
//signed 64-bit number, returns a long.  Negative input returns zero.
ulong quad_sqrt(long low,long high)
{
	uint32_t hi = (uint32_t) high;
	uint64_t n;
	uint32_t q,r,g,old_g;
	int cnt,shift;
	uint32_t b;

	if (high < 0)							//can't do negative number!
		return 0;

	if (hi == 0 && low >= 0)			//we can use longword version
		return long_sqrt(low);

	n = ((uint64_t) hi << 32) | (uint32_t) low;

	//get a good first quess by checking which byte most significant bit is in
	if (hi & 0xff000000) {
		b = hi >> 24;
		shift = 12+16;
	}
	else if (hi & 0x00ff0000) {
		b = (hi >> 16) & 0xff;
		shift = 8+16;
	}
	else if (hi & 0x0000ff00) {
		b = (hi >> 8) & 0xff;
		shift = 4+16;
	}
	else {
		b = hi & 0xff;				//can be zero when high bit of low is set
		shift = 0+16;
	}

	g = (uint32_t) guess_table[b] << shift;

	//quad loop usually executes 4 times.  The quotient always fits in 32
	//bits, and the (g+q)/2 sum is done with 33 bits like the asm's add/rcr.

	for (cnt=0;cnt<3;cnt++) {
		q = (uint32_t) (n / g);
		g = (uint32_t) (((uint64_t) g + q) >> 1);	//next guess = (d + q)/2
	}

	for (;;) {
		q = (uint32_t) (n / g);
		r = (uint32_t) (n % g);
		if (q == g)					//correct?
			return q;				//..yep
		old_g = g;					//save for compare
		g = (uint32_t) (((uint64_t) g + q) >> 1);	//next guess = (d + q)/2
		if (g == q || g == old_g)
			break;
	}

	//almost got it
	if (r)							//check remainder
		g++;

	return g;
}

//fixed-point square root
fix fix_sqrt(fix a)
{
	return (fix) long_sqrt(a) << 8;
}
