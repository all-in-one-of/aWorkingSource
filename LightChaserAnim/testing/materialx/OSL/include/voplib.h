#pragma once
#include <stdosl.h>
#include <oslutil.h>
/*
 * PROPRIETARY INFORMATION.  This software is proprietary to
 * Side Effects Software Inc., and is not to be reproduced,
 * transmitted, or disclosed in any way without written permission.
 *
 * Produced by:
 *	Side Effects Software Inc
 *	477 Richmond Street West
 *	Toronto, Ontario
 *	Canada   M5V 3E7
 *	416-504-9876
 *
 * NAME:	rsl_voplib.h ( VOP Library, RSL)
 *
 * COMMENTS:	This include file is used by VOPs to implement higher level
 *		shading functions.
 */

#ifndef __voplib__
#define __voplib__

#if !defined(PI)
    #define PI	3.14159265358979323846
#endif
#if !defined(M_PI)
    #define M_PI		PI
#endif
#if !defined(M_SQRT1_2)
    #define M_SQRT1_2		0.7071067
#endif

#define VOP_MIN_FILTER_SIZE	1e-6
#define VOP_PERIOD_FADE		(1.0/8.0)

#define FILTERSTEP(e0, x0, x1, filter)	aastep(e0,(x0+x1)/2)
#define AREA(pp)			area(pp)
#define VOP_FADE_PERIODIC(v1, v0, average, fwidth)	\
	    mix((v1-v0)/fwidth, average, clamp(fwidth*VOP_PERIOD_FADE, 0, 1));




#define VOP_LERP(type)	\
	type lerp(type x, type y, float alpha) { return mix(x, y, alpha); }

VOP_LERP(float)
VOP_LERP(point)
VOP_LERP(vector)
VOP_LERP(normal)
VOP_LERP(color)

#undef VOP_LERP


// The comp() funcitons exist in RSL but not in OSL.
float	comp(matrix m, int row, int column)
	{ return m[row][column]; }
void	setcomp(matrix m, int row, int column, float val)
	{ m[row][column] = val; }

#define VOP_COMP_FUNCTIONS(type) \
float  comp(type v, int i)	{ return v[i]; } \
float  xcomp(type v)		{ return comp(v,0); } \
float  ycomp(type v)		{ return comp(v,1); } \
float  zcomp(type v)		{ return comp(v,2); } \
void   setcomp(type v, int i, float val)	{ v[i] = val; } \
void   setxcomp(type v, float val)		{ setcomp(v,0,val); } \
void   setycomp(type v, float val)		{ setcomp(v,1,val); } \
void   setzcomp(type v, float val)		{ setcomp(v,2,val); } 

VOP_COMP_FUNCTIONS(vector)
VOP_COMP_FUNCTIONS(point)
VOP_COMP_FUNCTIONS(normal)
VOP_COMP_FUNCTIONS(color)

#undef VOP_COMP_FUNCTIONS

// Consistent with VEX implementation.
float	vop_frac_helper(float x)
	{ return x - floor(x); }

// Here are some common 'vop_' prefixed functions.
#define VOP_VECTOR_FUNCTIONS_HELPER(fname,func)	\
float  vop_##fname(float v)  { return func(v); } \
vector vop_##fname(vector v) { return vector(func(xcomp(v)), func(ycomp(v)), \
					func(zcomp(v))); } \
point  vop_##fname(point  v) { return point (func(xcomp(v)), func(ycomp(v)), \
					func(zcomp(v))); } \
normal vop_##fname(normal v) { return normal(func(xcomp(v)), func(ycomp(v)), \
					func(zcomp(v))); } \
color  vop_##fname(color v)  { return color(func(xcomp(v)), func(ycomp(v)), \
					func(zcomp(v))); }
#define VOP_VECTOR_FUNCTIONS(fname) VOP_VECTOR_FUNCTIONS_HELPER(fname, fname)

VOP_VECTOR_FUNCTIONS(abs)
VOP_VECTOR_FUNCTIONS(sqrt)
VOP_VECTOR_FUNCTIONS(ceil)
VOP_VECTOR_FUNCTIONS(floor)
VOP_VECTOR_FUNCTIONS(radians)
VOP_VECTOR_FUNCTIONS(degrees)
VOP_VECTOR_FUNCTIONS(exp)
VOP_VECTOR_FUNCTIONS(acos)
VOP_VECTOR_FUNCTIONS(asin)
VOP_VECTOR_FUNCTIONS(atan)
VOP_VECTOR_FUNCTIONS(cosh)
VOP_VECTOR_FUNCTIONS(tanh)
VOP_VECTOR_FUNCTIONS(cos)
VOP_VECTOR_FUNCTIONS(sin)
VOP_VECTOR_FUNCTIONS(tan)
VOP_VECTOR_FUNCTIONS_HELPER(rint, round)
VOP_VECTOR_FUNCTIONS_HELPER(frac, vop_frac_helper)

#undef VOP_VECTOR_FUNCTIONS
#undef VOP_VECTOR_FUNCTIONS_HELPER


#define VOP_VECTOR_BIN_FUNCTIONS_HELPER(fname,func)	\
float  vop_##fname(float v, float f)  { return func(v,f); } \
vector vop_##fname(vector v, float f) \
	{ return vector(func(xcomp(v),f), func(ycomp(v),f), func(zcomp(v),f));}\
point  vop_##fname(point  v, float f) \
	{ return point (func(xcomp(v),f), func(ycomp(v),f), func(zcomp(v),f));}\
normal vop_##fname(normal v, float f) \
	{ return normal(func(xcomp(v),f), func(ycomp(v),f), func(zcomp(v),f));}\
color  vop_##fname(color v, float f) \
	{ return color(func(xcomp(v),f), func(ycomp(v),f), func(zcomp(v),f));}
#define VOP_VECTOR_BIN_FUNCTIONS(fname)	\
	    VOP_VECTOR_BIN_FUNCTIONS_HELPER(fname, fname)

VOP_VECTOR_BIN_FUNCTIONS(mod)
VOP_VECTOR_BIN_FUNCTIONS(pow)

#undef VOP_VECTOR_BIN_FUNCTIONS
#undef VOP_VECTOR_BIN_FUNCTIONS_HELPER

#define vop_dot(a,b)	dot((a), (b)) 
#define vop_cross(a,b)	cross((a), (b))

float  frac(float x)  { return vop_frac_helper(x); } 
vector frac(vector x) { return x - vop_floor(x); }
point  frac(point x)  { return x - vop_floor(x); }
normal frac(normal x) { return x - vop_floor(x); }
color  frac(color x)  { return x - vop_floor(x); }

float  atan(float y, float x)	{ return atan2(y, x); }
vector atan(vector y, vector x)	{ return atan2(y, x); }
point  atan(point y, point x)	{ return atan2(y, x); }
normal atan(normal y, normal x)	{ return atan2(y, x); }
color  atan(color y, color x)	{ return atan2(y, x); }

#define VOP_COMPTEST_FUNCTIONS(type) \
float  vop_avg(type v)  { return (xcomp(v)+ycomp(v)+zcomp(v))/3.0; } \
float  vop_mincomp(type v)  { return min(min(xcomp(v), ycomp(v)), zcomp(v)); } \
float  vop_maxcomp(type v)  { return max(max(xcomp(v), ycomp(v)), zcomp(v)); } 

VOP_COMPTEST_FUNCTIONS(vector)
VOP_COMPTEST_FUNCTIONS(point)
VOP_COMPTEST_FUNCTIONS(normal)
VOP_COMPTEST_FUNCTIONS(color)
float  vop_getrayweight() { return 1; }

#undef VOP_COMPTEST_FUNCTIONS


matrix
vop_floattomatrix(float a00, float a01, float a02, float a03, 
		  float a10, float a11, float a12, float a13,
		  float a20, float a21, float a22, float a23, 
		  float a30, float a31, float a32, float a33)
{
    return matrix(a00,a01,a02,a03, a10,a11,a12,a13,
		  a20,a21,a22,a23, a30,a31,a32,a33);
}

#define VOP_CONVERSION_FUNCTIONS(type) \
type	vop_floattovec(float x, float y, float z) \
	{ return type(x, y, z); } \
void	vop_vectofloat(type v, output float x, output float y, output float z) \
	{ x = xcomp(v); y = ycomp(v); z = zcomp(v); }

VOP_CONVERSION_FUNCTIONS(vector)
VOP_CONVERSION_FUNCTIONS(point)
VOP_CONVERSION_FUNCTIONS(normal)
VOP_CONVERSION_FUNCTIONS(color)

#undef VOP_CONVERSION_FUNCTIONS


float
fit(float x, float omin, float omax, float nmin, float nmax)
{
    float	t;
    t = clamp((x - omin)/(omax - omin), 0, 1);
    return mix(nmin, nmax, t);
}

float	vop_fit(float v, float s0, float s1, float d0, float d1)
	{ 
	    return fit(v, s0, s1, d0, d1);
	}

#define VOP_FIT_FUNCTIONS(type) \
type	vop_fit(type v, type s0, type s1, type d0, type d1) \
	{ return type( \
		fit(xcomp(v), xcomp(s0), xcomp(s1), xcomp(d0), xcomp(d1)), \
		fit(ycomp(v), ycomp(s0), ycomp(s1), ycomp(d0), ycomp(d1)), \
		fit(zcomp(v), zcomp(s0), zcomp(s1), zcomp(d0), zcomp(d1)));\
	}

VOP_FIT_FUNCTIONS(vector)
VOP_FIT_FUNCTIONS(point)
VOP_FIT_FUNCTIONS(normal)
VOP_FIT_FUNCTIONS(color)

#undef VOP_FIT_FUNCTIONS


float vop_smooth(float v0, float v1, float amount, float rolloff)
{
    float	v;

    v = smoothstep(v0, v1, amount);
    if (rolloff > 1)
	v = pow(v, rolloff);
    else if (rolloff > 0 && rolloff < 1)
	v = 1-pow(1-v, 1/rolloff);
    return v;
}

float	vop_getcomp(matrix m, int row, int column)
	{ return comp(m, row, column); }

#define VOP_GETCOMP(type) \
float vop_getcomp(type v, int index) \
	{ return comp(v, index); }

VOP_GETCOMP(vector)
VOP_GETCOMP(point)
VOP_GETCOMP(normal)
VOP_GETCOMP(color)

#undef VOP_GETCOMP


matrix	vop_setmatcomp(matrix in, float fval, int row, int col)
	{
	    matrix out = in;
	    setcomp(out, row, col, fval);
	    return out;
	}

#define VOP_SETCOMP(type) \
   type vop_setcomp(type in, float val, int index) \
	{ \
	    type out = in; \
	    setcomp(out, index, val); \
	    return out; \
	}
VOP_SETCOMP(vector)
VOP_SETCOMP(point)
VOP_SETCOMP(normal)
VOP_SETCOMP(color)

#undef VOP_SETCOMP


void
vop_assign(
	output float v11, output float v12, output float v13, output float v14,
	output float v21, output float v22, output float v23, output float v24,
	output float v31, output float v32, output float v33, output float v34,
	output float v41, output float v42, output float v43, output float v44,
	matrix mat)
{
    v11 = comp(mat, 0, 0);
    v12 = comp(mat, 0, 2);
    v13 = comp(mat, 0, 3);
    v14 = comp(mat, 0, 4);
    v21 = comp(mat, 1, 0);
    v22 = comp(mat, 1, 1);
    v23 = comp(mat, 1, 2);
    v24 = comp(mat, 1, 3);
    v31 = comp(mat, 2, 0);
    v32 = comp(mat, 2, 1);
    v33 = comp(mat, 2, 2);
    v34 = comp(mat, 2, 3);
    v41 = comp(mat, 3, 0);
    v42 = comp(mat, 3, 1);
    v43 = comp(mat, 3, 2);
    v44 = comp(mat, 3, 3);
}

matrix
vop_translate(matrix m, vector t)
{
    matrix out = 1;

    // Pre-concatenate a translation matrix onto m.
    for(int i = 0; i < 4; ++i)
    {
	out[i][0] = m[i][0] + m[i][3] * t[0];
	out[i][1] = m[i][1] + m[i][3] * t[1];
	out[i][2] = m[i][2] + m[i][3] * t[2];
    }

    return out; 
} 

matrix
vop_rotate(matrix m, float angle, vector axis)
{
    vector  n;
    float   x, y, z;
    float   ca, sa, cr;
    matrix  r;

    n = normalize(axis);
    x = n[0];
    y = n[1];
    z = n[2];

    ca = cos(angle);
    sa = sin(angle);
    cr = 1.0 - ca;

    r[0][0] = cr*x*x + ca;   r[0][1] = cr*x*y + sa*z; r[0][2] = cr*x*z - sa*y;
    r[1][0] = cr*y*x - sa*z; r[1][1] = cr*y*y + ca;   r[1][2] = cr*y*z + sa*x;
    r[2][0] = cr*z*x + sa*y; r[2][1] = cr*z*y - sa*x; r[2][2] = cr*z*z + ca;
    r[3][0] = 0;	     r[3][1] = 0;	      r[3][2] = 0; 

    r[0][3] = 0;
    r[1][3] = 0;
    r[2][3] = 0;
    r[3][3] = 1;

    return r * m; 
}

matrix
vop_scale(matrix m, point s)
{
    matrix out = 1;

    // Pre-concatenate a scale matrix onto m.
    for(int i = 0; i < 4; ++i)
    {
	out[i][0] = m[i][0] * s[0];
	out[i][1] = m[i][1] * s[1];
	out[i][2] = m[i][2] * s[2];
    }

    return out; 
}

void
vop_bindST(output float news, float ss, int Sconnected,
	   output float newt, float tt, int Tconnected)
{ 
    news = (Sconnected != 0) ? ss : u; 
    newt = (Tconnected != 0) ? tt : v;
}

void
vop_bindS(output float news, float ss, float Sconnected)
{
    news = (Sconnected != 0) ? ss : u;
}

float
vop_bias(float base, float bias) 
{
    float val;
    if (base <= 0)
	val = 0;
    else if (base >= 1)
	val = 1;
    else
	val = bias / (((1.0 / base) - 2) * (1 - bias) + 1);
    return val;
}

float
vop_gain(float base, float gain) 
{
    float	val;
    if (base < 0.5)
	val = vop_bias(2*base, gain)*.5;
    else
	val = 1-vop_bias(2*(1-base), gain)*.5;
    return val;
}


float
vop_luminance(color c)
{
    return comp(c, 0)*0.299 + comp(c, 1)*0.587 + comp(c, 2)*0.114;
}

color
rgbtohsv(color rgb)
{
    return transformc("rgb", "hsv", rgb);
}

color
hsvtorgb(color hsv)
{
    return transformc("hsv", "rgb", hsv);
}

color
vop_colormix(color c1, color c2, float bias, float adjust)
{
    color	clr;
    if (adjust == 3)
    {
	float a0[4] = {c1[0], c1[0], c2[0], c2[0]};
	float a1[4] = {c1[1], c1[1], c2[1], c2[1]};
	float a2[4] = {c1[2], c1[2], c2[2], c2[2]};
	clr[0] = spline("catrom", bias, a0);
	clr[1] = spline("catrom", bias, a1);
	clr[2] = spline("catrom", bias, a2);
    }
    else if (adjust == 2)
	clr = mix(c1, c2, float(linearstep(0, 1, bias)));
    else if (adjust == 1)
	clr = mix(c1, c2, float(clamp(bias, 0, 1)));
    else
	clr = mix(c1, c2, bias);
    return clr;
}

void
vop_composite(string operation,
	      output color C, output float Ca,	// Resulting color/alpha
	      color A, float Aa,		// Src color (overlay)
	      color B, float Ba 		// Dest color (background)
	     )
{
    if (operation == "AoverB")
    {
	C  = mix(B, A, Aa);
	Ca = mix(Ba, Aa, Aa);
    }
    else if (operation == "AinsideB")	// A inside B
    {
	C  = A*Ba;
	Ca = Aa*Ba;
    }
    else if (operation == "AoutsideB")
    {
	C  = A*(1-Ba);
	Ca = Aa*(1-Ba);
    }
    else if (operation == "AatopB")
    {
	C  = A*Ba + B*(1-Aa);
	Ca = Ba;
    }
    else if (operation == "AxorB")
    {
	C  = A*(1-Ba) + B*(1-Aa);
	Ca = Aa + Ba - 2*(Aa*Ba);
    }
    else if (operation == "A")
    {
	C  = A;
	Ca = Aa;
    }
    else if (operation == "B")
    {
	C  = B;
	Ca = Ba;
    }
    else if (operation == "clear")
    {
	C = 0;
	Ca = 0;
    }
}

float
vop_ptlined(point P1, point P2, point Q)
{
    return distance(P1, P2, Q);
}

#define vop_luminance(c)	luminance(c)


float
vop_FilterWidth(float x)
{
    return filterwidth(x);
}

float
vop_Pulse(float edge0, float edge1, float x, float fwidth, string filter)
{
    float	x0, x1;

    x0 = x  - fwidth*.5;
    x1 = x0 + fwidth;
    return max(0, (min(x1, edge1)-max(x0, edge0))/fwidth);
}

float
vop_FilteredSin(float x, float fwidth)
{
    float	x0, x1;

    x0 = x - fwidth * .5;
    x1 = x + fwidth;
    return (-M_SQRT1_2)*VOP_FADE_PERIODIC(cos(x1), cos(x0), 0, fwidth);
}

float
vop_FilteredSinD(float x)
{
    return vop_FilteredSin(x, vop_FilterWidth(x));
}

float
vop_FilteredCos(float x, float fwidth)
{
    float	x0, x1;

    x0 = x - fwidth * .5;
    x1 = x + fwidth;
    return M_SQRT1_2*VOP_FADE_PERIODIC(sin(x1), sin(x0), 0, fwidth);
}

float
vop_FilteredCosD(float x)
{
    return vop_FilteredCos(x, vop_FilterWidth(x));
}

float
vop_PulseD(float edge0, float edge1, float x, string filter)
{
    return vop_Pulse(edge0, edge1, x, vop_FilterWidth(x), filter);
}

float
vop_PulseTrain(float edge0, float x, float fwidth)
{
    float	x0, x1;

    x0 = x  - fwidth*.5;
    x1 = x0 + fwidth;
    x0 = edge0*floor(x0) + min(edge0, frac(x0));
    x1 = edge0*floor(x1) + min(edge0, frac(x1));
    return VOP_FADE_PERIODIC(x1, x0, edge0, fwidth);
}

float
vop_PulseTrainD(float edge0, float x)
{
    return vop_PulseTrain(edge0, x, vop_FilterWidth(x));
}

float
vop_RampTrain(float x, float fwidth)
{
    float	x0, x1, f;
    x0 = x - fwidth*.5;
    x1 = x0 + fwidth;
    f = frac(x0); x0 = floor(x0) + f*f;
    f = frac(x1); x1 = floor(x1) + f*f;
    return .5*VOP_FADE_PERIODIC(x1, x0, .5, fwidth);
}

float
vop_RampTrainD(float x)
{
    return vop_RampTrain(x, vop_FilterWidth(x));
}

float
vop_IntegrateTent(float x)
{
    float	f;
    f = frac(x);
    if (f > .5) f = f*(2 - f) - .5;
    else	f = f*f;
    return .5*floor(x) + f;
}

float
vop_TentTrain(float x, float fwidth)
{
    float	x0, x1;
    x0 = x - fwidth*.5;
    x1 = x0 + fwidth;
    x0 = vop_IntegrateTent(x0);
    x1 = vop_IntegrateTent(x1);
    return VOP_FADE_PERIODIC(x1, x0, .5, fwidth);
}

float
vop_TentTrainD(float x)
{
    return vop_RampTrain(x, vop_FilterWidth(x));
}

float
vop_DotStamp(float px, float py, float fwidth, string filter)
{
    float	d;
    d = px*px + py*py;
    return FILTERSTEP(1, d-fwidth, d+fwidth, filter);
}

float
vop_BumpStamp(float px, float py, float fwidth, string filter)
{
    float	d, v;
    float	x0, x1;
    d  = px*px + py*py;
    x0 = d  - fwidth*.5;
    x1 = x0 + fwidth;
    v  = 1-FILTERSTEP(1, d-fwidth, d+fwidth, filter);
    v *= 1-smoothstep(0.0, 1.0, d);
    return v;
}

float
vop_RoundCosStamp(float px, float py, float fwidth, 
	float hexness, float sides, float power, string filter)
{
    float	ss, tt;
    float	x0, x1;

    ss = atan2(py, px);
    tt = px*px + py*py + hexness*vop_pow(abs(1-cos(ss*(sides))), power);
    x0 = tt - fwidth*.5;
    x1 = x0 + fwidth;
    return FILTERSTEP(1, x0, x1, filter);
}

float
vop_RoundSinStamp(float px, float py, float fwidth, 
	float hexness, float sides, float power, string filter)
{
    float	ss, tt;
    float	x0, x1;

    ss = atan2(py, px) - M_PI;
    tt = px*px + py*py + hexness*vop_pow(abs(1-sin(ss*(sides))), power);
    x0 = tt - fwidth*.5;
    x1 = x0 + fwidth;
    return FILTERSTEP(1, x0, x1, filter);
}


float
vop_RingStamp(float px, float py, float iradius, float oradius, float fwidth,
	string filter)
{
    float	d;
    d = px*px + py*py;
    return vop_Pulse(iradius, oradius, d, fwidth, filter);
}

float
vop_BoxStamp(float px, float py, float fu, float fv, string filter)
{
    float	dx;
    dx  = vop_Pulse(-1, 1, px, fu, filter);
    dx *= vop_Pulse(-1, 1, py, fv, filter);
    return dx;
}

//
// Tile generation
//  Input:	u, v	- The position
//		fx, fy	- Frequency
//		ox, oy	- Offset
//		jitter	- Jitter for each row
//  Return:	id	- Random ID for the tile
//  Output:	u, v,	- The coordinates within the tile
//		du, dv	- Derivative information
//  Unchanged:	fx, fy, ox, oy, jitter
//		
int
vop_TileGen(output float u, output float v, 
	float fx, float fy, float ox, float oy, float stagger, float jitter)
{
    float	row;

    v = v*fy - oy;
    row = floor(v);
    u = u*fx - ox - stagger*row + jitter*cellnoise(row);
    return int(floor(u) + row*1984);
}

int
vop_HexTileGen(output float u, output float v, 
	float fx, float fy, float ox, float oy)
{
    float	left, right;
    float	row, col;

    v   = v*fy - oy;
    row = floor(v);
    v  = frac(v);

    u = u*fx + ox;
    if (mod(floor(row), 2) == 1)
	u += .5;

    col = floor(u);
    u = frac(u);

    if (v > .5)
    {
	right = v - .5;
	left  = 1.5 - v;
	if (u > left)
	{
	    if (mod(floor(row), 2) == 0) col += 1;
	    row += 1;
	    v -= 1;
	    u = fit(u, left, 1, 0, .5);
	}
	else if (u < right)
	{
	    if (mod(floor(row), 2) == 1) col -= 1;
	    row += 1;
	    v -= 1;
	    u = fit(u, 0, right, 0.5, 1);
	}
	else u = fit(u, right, left, 0, 1);
    }
    v = (v*2+1)/3;
    return int(row * 938 + col);
}

//
// Stamping simple shape patterns.  Some patterns take auxilliary data.  These
// are:
//	ring	- aux data specifies the ring width (0 to 1)
//	hex	- aux specifies the "hexness" (0 to 1)
//
float
vop_StampPattern(float  layers,		// Number of layers
		 float dotsize,		// Base dotsize
		 float dj,		// Dotsize jitter
		 float softness,	// Softness
		 float px, float jx,	// X position and jitter
		 float py, float jy,	// Y position and jitter
		 float aux, float jaux,	// Auxilliary data
		 string spottype,	// Type of spot
		 string filter,		// Filter type
		 output float floatseed	// Return a random seed
	 )
{
    float	fwidth;
    float	lrandom;
    float	cx, cy, loff, dsize;
    float	fx, fy;
    float	djx, djy;
    float	du, dv;
    float	result, amount;
    float	irad, idot;
    point	pp;
    int	i;

    du = vop_FilterWidth(px) * softness;
    dv = vop_FilterWidth(py) * softness;
    fwidth = max(du, dv);
    result = 0;
    loff = 0;
    for (i = 0; i < layers; i += 1)
    {
	lrandom = cellnoise(i);
	pp = point(floor(px+loff), floor(py+loff),
		    1000.0*lrandom+500) + vector(.5, .5, .5);
	floatseed = cellnoise(pp);

	pp = point(cellnoise(pp)) - .5;
	dsize = dotsize * (1-dj*floatseed);
	idot = 1/dsize;

	djy  = idot*(1 - dsize);
	djx  = (2*clamp(jx, 0, 1)) * djy;
	djy *= (2*clamp(jy, 0, 1));

	cx = xcomp(pp) * djx;
	cy = ycomp(pp) * djy;
	fx = cx - 2*(frac(px+loff) - .5)*idot;
	fy = cy - 2*(frac(py+loff) - .5)*idot;

	if (spottype == "ring")
	{
	    irad = 1-clamp(aux + (zcomp(pp) - .5)*jaux, 0, 1);
	    amount = vop_RingStamp(fx, fy, irad, 1, fwidth, filter);
	}
	else if (spottype == "box")
	{
	    amount = vop_BoxStamp(fx, fy, du, dv, filter);
	}
	else if (spottype == "hex")
	{
	    amount = 1-vop_RoundCosStamp(fx, fy, fwidth, aux, 6, jaux, filter);
	}
	else if (spottype == "star")
	{
	    amount = 1-vop_RoundSinStamp(fx, fy, fwidth, aux, 5, jaux, filter);
	}
	else if (spottype == "bump")
	{
	    amount = vop_BumpStamp(fx, fy, fwidth, filter);
	}
	else
	{
	    amount = 1-vop_DotStamp(fx, fy, fwidth, filter);
	}
	result = (1-amount)*result + amount;
	loff += 0.5;
    }
    floatseed = (floatseed - 0.5)*32000;
    return result;
}

float
vop_RipplePattern(float x, float y, float decay, float toff)
{
    float	d;

    d = sqrt(x*x + y*y);
    return sin(d - toff) * exp(d*(-decay));
}

#define VOP_FBMNOISE_FUNC(type)	\
	    float	scale, plimit, blend; \
	    float	octaves; \
	    plimit = 2*blur; \
	    nval = 0; octaves = 0; scale = 1; \
	    while (scale > plimit && octaves < maxoctaves) { \
		nval += scale *(type(noise(pp)) - 0.5); \
		scale *= rough; \
		pp *= 2; \
		octaves += 1; \
	    } \
	    if (scale > blur) { \
		blend = scale * clamp(scale/blur - 1, 0, 1); \
		nval += blend*(type(noise(pp)) - 0.5); \
	    }

float
vop_fbmNoise(float pos, float rough, float maxoctaves)
{
    float	blur = vop_FilterWidth(pos);
    float	nval, pp = pos;
    VOP_FBMNOISE_FUNC( float )
    return nval;
}

float
vop_fbmNoise(point pos, float rough, float maxoctaves)
{
    float	blur = AREA(pos);
    float	nval;
    point	pp = pos;
    VOP_FBMNOISE_FUNC( float )
    return nval;
}

point
vop_fbmNoise(float pos, float rough, float maxoctaves)
{
    float	blur = vop_FilterWidth(pos);
    point	nval, pp = pos;
    VOP_FBMNOISE_FUNC( point )
    return nval;
}

point
vop_fbmNoise(point pos, float rough, float maxoctaves)
{
    float	blur = AREA(pos);
    point	nval, pp = pos;
    VOP_FBMNOISE_FUNC( point )
    return nval;
}

#define VOP_PERLINNOISE_FUNC(type)	\
	    point	pp	= pos; \
	    float	scale	= amp; \
	    float	i; \
	    nval = 0; \
	    for (i = 0; i < turb; i += 1) { \
		nval += scale * type(noise(pp)); \
		pp *= 2.0; scale *= rough; \
	    } \
	    nval = (type(vop_pow(nval, atten)));


float
vop_perlinNoise(point pos, float turb, float amp, float rough, float atten)
{
    float nval;
    VOP_PERLINNOISE_FUNC(float)
    return nval;
}

point
vop_perlinNoise(point pos, float turb, float amp, float rough, float atten)
{
    point nval;
    VOP_PERLINNOISE_FUNC(point)
    return nval;
}

void
vop_displaceAlongNormal(point pp, normal nn,
			float amount, float scale, float sshear, float tshear,
			float for_poly, int obj_space, int bump_only,
			output point dP, output normal dN)
{
    normal	ng;
    if (obj_space != 0)
    {
	dN = transform("current", "object", nn);
	ng = transform("current", "object", Ng);
	dP = transform("current", "object", pp);
    }
    else
    {
	dN = nn;
	ng = Ng;
	dP = pp;
    }
    dP += (scale * amount) * dN;
    dP += (sshear * amount) * normalize(dPdu);
    dP += (tshear * amount) * normalize(dPdu);
    if (for_poly != 0)
    {
	normal	dn = normalize(dN) - normalize(ng);	// Delta of normal
	dN = calculatenormal(dP);
	dN = normalize(dN) + dn;
    }
    else
    {
	dN = calculatenormal(dP);
    }
    if (obj_space != 0)
    {
	// TODO: If the transform doesn't have a negative determinant, then the
	// following code should probably not have a negative sign in front.
	dN = -transform("object", "current", dN);
	dP =  transform("object", "current", dP);
    }
    if (bump_only != 0)
    {
	dP = pp;
    }
}

point
vop_frompolar(float u, float v, float radius)
{
    // u, v are expected to be in the range(0, 1)
    float	sv = radius*sin(v);
    return point(sv*cos(u), sv*sin(u), radius*cos(v));
}

point
vop_topolarXYZ(float x, float y, float z)
{
    // Return a vector(u, v, radius)
    float	r = sqrt(x*x+y*y+z*z);
    float	u = mod(atan2(y, x), (2*M_PI));	// Range (0, 2*M_PI)
    float	v = acos(z/r);
    return point(u, v, r);
}

point
vop_topolar(point v)
{
    return vop_topolarXYZ(xcomp(v), ycomp(v), zcomp(v));
}

void
vop_vnoise(float pos, float jitter, 
	output int seed, output float dist1, output float dist2,
	output float near1, output float near2)
{
    float thiscell = floor(pos) + 0.5;

    dist1 = 1000;  // initialize to a large number
    dist2 = 1000;
    near1 = 0;
    near2 = 0;
    int	    i;

    for (i = -1; i <= 1; i += 1) 
    {
	float  testcell = thiscell + i;
	float  cv       = (float) cellnoise(testcell) - 0.5;
	float  ptpos    = testcell + jitter * cv;

	float dist = ptpos - pos;
	if (dist < dist1)
	{
	    seed = (int) testcell;
	    dist2 = dist1;
	    near2 = near1;

	    dist1 = dist;
	    near1 = ptpos;
	}
	else if (dist < dist2)
	{
	    dist2 = dist;
	    near2 = ptpos;
	}
    }
    seed = (int) ((float) cellnoise(seed)-.5)*1000;
}

void
vop_vnoise(float pos, float jitter, 
	output int seed, output float dist1, output float dist2,
	output float near1, output float near2,
	float period)
{
    float thiscell = floor(pos) + 0.5;
	
    dist1 = 1000;  // initialize to a large number
    dist2 = 1000;
    near1 = 0;
    near2 = 0;
    int	    i;

    for (i = -1; i <= 1; i += 1) 
    {
	float  testcell = vop_mod(thiscell + i, period);
	float  cv       = (float) cellnoise(testcell) - 0.5;
	float  ptpos    = testcell + jitter * cv;

	float dist = ptpos - pos;
	if (dist < dist1)
	{
	    seed = (int) testcell;
	    dist2 = dist1;
	    near2 = near1;

	    dist1 = dist;
	    near1 = ptpos;
	}
	else if (dist < dist2)
	{
	    dist2 = dist;
	    near2 = ptpos;
	}
    }
    seed = (int) ((float) cellnoise(seed)-.5)*1000;
}

void
vop_vnoise_compute_dist(point pos, point testcell, point jitter,
	output float dist1, output float dist2,
	output point near1, output point near2)
{
    vector cv       = (vector) cellnoise(testcell) - vector(0.5);
    point  ptpos    = testcell + vector(
			xcomp(jitter)*xcomp(cv),
			ycomp(jitter)*ycomp(cv),
			zcomp(jitter)*zcomp(cv));
    vector offset = ptpos - pos;
    float dist = vop_dot(offset, offset); // dist squared
    if (dist < dist1)
    {
	dist2 = dist1;
	near2 = near1;

	dist1 = dist;
	near1 = ptpos;
    }
    else if (dist < dist2)
    {
	dist2 = dist;
	near2 = ptpos;
    }
}

void
vop_vnoise(point pos, point jitter,
	output int seed, output float dist1, output float dist2,
	output point near1, output point near2)
{
    point thiscell = point(floor(xcomp(pos)) + 0.5,
			   floor(ycomp(pos)) + 0.5,
			   floor(zcomp(pos)) + 0.5);
    dist1 = 1000;  // initialize to a large number
    dist2 = 1000;
    near1 = point(0, 0, 0);
    near2 = point(0, 0, 0);
    int   i, j, k;

    for (i = -1; i <= 1; i += 1) 
    {
	for (j = -1; j <= 1; j += 1) 
	{
	    for (k = -1; k <= 1; k += 1)
	    {
		vop_vnoise_compute_dist(pos, thiscell + vector(i, j, k),
					jitter, dist1, dist2, near1, near2);
	    }
	}
    }

    seed = (int) ((float) cellnoise(near1) - 0.5)*1000;
    dist1 = sqrt(dist1);
    dist2 = sqrt(dist2);
}

void
vop_vnoise(point pos, point jitter,
	output int seed, output float dist1, output float dist2,
	output point near1, output point near2,
	point period)
{
    point thiscell = point(floor(xcomp(pos)) + 0.5,
			   floor(ycomp(pos)) + 0.5,
			   floor(zcomp(pos)) + 0.5);
    dist1 = 1000;  // initialize to a large number
    dist2 = 1000;
    near1 = point(0, 0, 0);
    near2 = point(0, 0, 0);
    int   i, j, k;

    for (i = -1; i <= 1; i += 1) 
    {
	for (j = -1; j <= 1; j += 1) 
	{
	    for (k = -1; k <= 1; k += 1)
	    {
		point testcell = thiscell + vector(i, j, k);

		testcell = point(
			     vop_mod(xcomp(testcell), xcomp(period)),
			     vop_mod(ycomp(testcell), ycomp(period)),
			     vop_mod(zcomp(testcell), zcomp(period)) 
			   );

		vop_vnoise_compute_dist(pos, testcell, jitter, 
					dist1, dist2, near1, near2);
	    }
	}
    }

    seed = (int) ((float) cellnoise(near1) - 0.5)*1000;
    dist1 = sqrt(dist1);
    dist2 = sqrt(dist2);
}

void
vop_vnoise(vector vpos, point jitter, 
	output int seed, output float dist1, output float dist2,
	output point near1, output point near2, 
	point period)
{
    vop_vnoise(point(vpos), jitter, seed, dist1, dist2, near1, near2, period);
}

void
vop_vnoise(vector vpos, point jitter, 
	output int seed, output float dist1, output float dist2,
	output point near1, output point near2)
{
    vop_vnoise(point(vpos), jitter, seed, dist1, dist2, near1, near2);
}

#define VOP_VNOISE_VECTOR(type) \
void vop_vnoise(type vpos, type jitter,  \
	output int seed, output float dist1, output float dist2, \
	output type near1, output type near2) \
{ \
    point pnear1; \
    point pnear2; \
    vop_vnoise(point(vpos), point(jitter), seed, dist1, dist2, pnear1, pnear2);\
    near1 = type(pnear1); \
    near2 = type(pnear2); \
} \
void vop_vnoise(type vpos, type jitter, \
	output int seed, output float dist1, output float dist2, \
	output type near1, output type near2, \
	type period) \
{ \
    point pnear1; \
    point pnear2; \
    vop_vnoise(point(vpos), point(jitter), seed, dist1, dist2, pnear1, pnear2, \
	       point(period));\
    near1 = type(pnear1); \
    near2 = type(pnear2); \
}

VOP_VNOISE_VECTOR(vector)
VOP_VNOISE_VECTOR(color)
VOP_VNOISE_VECTOR(normal)

#undef VOP_VNOISE_VECTOR

//
// This function generates Worley noise with a border.  Parameters are:
//	Input:	ss = S coord
//		tt = T coord
//		jx = Jitter in S direction
//		jy = Jitter in T direction
//		bwidth = Border width
//		bsoft = Border softness
//	Output:
//		Return code = 0 in border area, 1 in cell interior
//		centerx = X Center of closest node
//		centery = Y Center of closest node
//		seed = A random seed for the closest node
float
vop_aaCell2D(float ss, float tt, float jx, float jy, float bwidth, float bsoft,
	     output float centerx, output float centery,
	     output float dist1, output float dist2,
	     output int seed)
{
    float	x0;
    float	blur;
    point	pos, jitter, pnear1, pnear2;

    pos    = point(ss, tt, 0);
    jitter = point(jx, jy, 0);
    vop_vnoise(pos, jitter, seed, dist1, dist2, pnear1, pnear2);

    setzcomp(pnear1, 0);
    setzcomp(pnear2, 0);

    centerx = comp(pnear1, 0);
    centery = comp(pnear1, 1);

    x0 = (dist2-dist1)*(dist1+dist2) / max(distance(pnear1, pnear2),
	                                   VOP_MIN_FILTER_SIZE);

    blur = max(vop_FilterWidth(ss), vop_FilterWidth(tt)) * (1 + bsoft);

    return FILTERSTEP(bwidth, x0-blur, x0+blur, "gauss");
}

// We need to include this after things like vop_dot are defined
#include "vop_singlescatter.h"

#endif
