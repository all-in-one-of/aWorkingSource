#include <iostream>
#include <vector>
#include <string>
#include <math.h>
struct AtPoint2
{
   float x;
   float y;
};

struct AtFloatUV
{
   float u;
   float v;
};

inline AtPoint2 hashBlock(AtFloatUV inUV,AtPoint2 inBlock)
{
   AtPoint2 result = {1, 1};
   float unitX = 1/inBlock.x;
   float unitY = 1/inBlock.y;

   result.x = ceil(inUV.u / unitX);
   result.y = ceil(inUV.v / unitY);
   return result;
}

int main()
{
   AtFloatUV inUV = {0.25, 1};
   AtPoint2 inBlock = {4, 4};
   AtPoint2 result = hashBlock(inUV, inBlock);
   float offset = sin(result.x+offset);
   float mirror = asin(0.2)*M_PI;
/*   std::cout << "result: "<< result.x << " " << result.y << std::endl;
   std::cout << "offset: "<< offset << std::endl;
   std::cout << "mirror: "<< mirror << std::endl;*/
   int mirrorE = (int)result.x % 2;
   int mirrorO = (int)result.y % 2;
   std::cout << mirrorE<< mirrorO << std::endl;
   return 0;
}