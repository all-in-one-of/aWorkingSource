#pragma once

#include <ai.h>

#include <cstring>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <climits>
#include <cfloat>
#include <iostream>  
#include <algorithm> 
#include <math.h>

#define SAMPLE 10.0f

inline float lerp(const float a, const float b, const float t)
{
   return (1-t)*a + t*b;
}

inline AtRGB lerp(const AtRGB& a, const AtRGB& b, const float t)
{
   AtRGB r;
   r.r = lerp( a.r, b.r, t );
   r.g = lerp( a.g, b.g, t );
   r.b = lerp( a.b, b.b, t );
   return r;
}

inline AtVector lerp(const AtVector& a, const AtVector& b, const float t)
{
   AtVector r;
   r.x = lerp( a.x, b.x, t );
   r.y = lerp( a.y, b.y, t );
   r.z = lerp( a.z, b.z, t );
   return r;
}

inline AtRGBA lerp(const AtRGBA& a, const AtRGBA& b, const float t)
{
   AtRGBA r;
   r.r = lerp( a.r, b.r, t );
   r.g = lerp( a.g, b.g, t );
   r.b = lerp( a.b, b.b, t );
   r.a = lerp( a.a, b.a, t );
   return r;
}

inline AtPoint2 lerpOffset(AtPoint2 block)
{
   AtPoint2 result;
   result.x = (1/block.x)/SAMPLE;
   result.y = (1/block.y)/SAMPLE;
   return result;
}

/*AtPoint2 fract(AtPoint2 inArg)
{
   AtPoint2 result;
   result.x = inArg.x - floor(inArg.x);
   result.y = inArg.y - floor(inArg.y);
   return result;
}

float clamp(float x, float minVal, float maxVal)
{
   float result;
   result = std::min(std::max(x, minVal), maxVal);
   return result;    
}
*/