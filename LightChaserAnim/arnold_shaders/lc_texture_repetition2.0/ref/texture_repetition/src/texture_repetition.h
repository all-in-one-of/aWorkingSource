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



struct AtFloatUV
{
   float u;
   float v;
};

const int RANDOM[10] = {0,1,1,0,1,1,0,1,1,0};
inline AtPoint2 hashBlock(AtFloatUV inUV,AtPoint2 inBlock)
{
   AtPoint2 result = {1, 1};
   float unitX = 1/inBlock.x;
   float unitY = 1/inBlock.y;
   AtFloatUV fUV;
   fUV.u = inUV.u - floor(inUV.u);
   fUV.v = inUV.v - floor(inUV.v);

   result.x = ceil(fUV.u / unitX);
   result.y = ceil(fUV.v / unitY);
   return result;
}

inline AtPoint2 hashCenterUV(AtFloatUV inUV,AtPoint2 inBlock)
{
   AtPoint2 block = hashBlock(inUV,inBlock);
   float unitX = 1/block.x;
   float unitY = 1/block.y;
   AtPoint2 result;
   result.x = unitX*(block.x - 0.5);
   result.y = unitY*(block.y - 0.5);
   return result;
}


float hash4(float u,float v)
{
   if( u < 0.5 && u >= 0 && v < 0.5 && v >= 0)
   {
      //std::cout << "@ One\n" << std::endl;
      return 1;
   }
   else if( u < 0.5 && u >= 0 && v < 1 && v >= 0.5)
   {
      //std::cout << "@ Two\n" << std::endl;
      return 2;
   } 
   else if( u < 1 && u >= 0.5 && v < 0.5 && v >= 0)
   {
      //std::cout << "@ Three\n" << std::endl;
      return 3;
   }
   else
   {
      //std::cout << "@ Four\n" << std::endl;
      return 4;
   }
}
