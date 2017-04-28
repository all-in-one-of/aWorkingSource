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

struct ShaderData
{
    AtTextureHandle* texturehandle;
    AtTextureParams *textureparams;
    bool useCustomUVSet;
    std::string uvSetName;
};

struct SGCache{
   
   void initCache(const AtShaderGlobals *sg){
      u = sg->u;
      v = sg->v;
      dudx = sg->dudx;
      dudy = sg->dudy;
      dvdx = sg->dvdx;
      dvdy = sg->dvdy;
   }

   void restoreSG(AtShaderGlobals *sg){
      sg->u = u;
      sg->v = v;
      sg->dudx = dudx;
      sg->dudy = dudy;
      sg->dvdx = dvdx;
      sg->dvdy = dvdy;
   }

   float u;
   float v;
   float dudx;
   float dudy;
   float dvdx;
   float dvdy;
};


const int RANDOM[10] = {0,1,1,0,1,1,0,1,1,0};

inline AtPoint2 hashBlock(AtPoint2 inUV,AtPoint2 inBlock)
{
   AtPoint2 result = {1, 1};
   float unitX = 1/inBlock.x;
   float unitY = 1/inBlock.y;
   AtPoint2 fUV;
   fUV.x = inUV.x - floor(inUV.x);
   fUV.y = inUV.y - floor(inUV.y);

   result.x = ceil(fUV.x / unitX);
   result.y = ceil(fUV.y / unitY);
   return result;
}

inline AtPoint2 hashCenterUV(AtPoint2 inUV,AtPoint2 inBlock)
{
   AtPoint2 block = hashBlock(inUV,inBlock);
   float unitX = 1/block.x;
   float unitY = 1/block.y;
   AtPoint2 result;
   result.x = unitX*(block.x - 0.5);
   result.y = unitY*(block.y - 0.5);
   return result;
}

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
   result.x = (1/block.x)/SAMPLE + 1/block.x;
   result.y = (1/block.y)/SAMPLE + 1/block.y;
   return result;
}