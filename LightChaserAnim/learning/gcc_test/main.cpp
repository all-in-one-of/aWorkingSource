#include <stdio.h>
#include <math.h>

int main()
{
   double num = -0.55;
   int result;

   result = floor(num);

   printf("Floor integer of %.2f = %d", num, result);
   return 0;
}



/*#include "aHeader.h"

void hash4(float u,float v)
{
   if( u < 0.5 && u > 0 && v < 0.5 && v > 0)
   {
      std::cout << "@ One" << std::endl;
   }
   else if( u < 0.5 && u > 0 && v < 1 && v > 0.5)
   {
      std::cout << "# Two" << std::endl;
   } 
   else if( u < 1 && u > 0.5 && v < 0.5 && v > 0)
   {
      std::cout << "$ Three" << std::endl;
   }
   else
   {
      std::cout << "& Four" << std::endl;
   }
}


int main()
{
	float u = 0.6;
	float v = 0.3;
	hash4(u,v);
}*/