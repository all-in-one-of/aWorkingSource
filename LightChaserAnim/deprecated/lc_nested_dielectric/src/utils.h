#pragma once
#include <vector>
#include <string>
#include <math.h>
#include <cassert>



namespace kt
{

struct SGCache{
   
   void initCache(const AtShaderGlobals *sg){
      u = sg->u;
      v = sg->v;
      dudx = sg->dudx;
      dudy = sg->dudy;
      dvdx = sg->dvdx;
      dvdy = sg->dvdy;
      dPdu = sg->dPdu;
      dPdv = sg->dPdv;
      Nf =sg->Nf;
   }

   void restoreSG(AtShaderGlobals *sg){
      sg->u = u;
      sg->v = v;
      sg->dudx = dudx;
      sg->dudy = dudy;
      sg->dvdx = dvdx;
      sg->dvdy = dvdy;
      sg->dPdu = dPdu;
      sg->dPdv = dPdv;
      sg->Nf = Nf;
   }

   float u;
   float v;
   float dudx;
   float dudy;
   float dvdx;
   float dvdy;
   AtVector dPdu;
   AtVector dPdv;
   AtVector Nf;   
};


inline float maxh(const AtRGB& c)
{
   return std::max(std::max(c.r, c.g), c.b);
}

inline float minh(const AtRGB& c)
{
   return std::min(std::min(c.r, c.g ), c.b);
}

}// ending namespace