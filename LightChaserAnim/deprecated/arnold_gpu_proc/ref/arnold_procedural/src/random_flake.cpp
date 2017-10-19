#include "ai.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
 
using namespace std;
 
// global variables for procedural
int   G_npoints;
float G_cloudrad;
float G_sphererad;
int   G_recursions;
int   G_counter;
int   G_computedtotal;
 
// returns a random float between 0 and 1
static float myFrand()
{
   return (float)rand() / (float)RAND_MAX;
}
 
// returns a random vector in a unit sphere with a
// power function to bias it towards the center
static AtVector randvec(float power)
{
   AtVector out;
   out.x = myFrand() - 0.5;
   out.y = myFrand() - 0.5;
   out.z = myFrand() - 0.5;
   out = AiV3Normalize(out) * pow(myFrand(), power);
   return out;
}
 
// recursive function that creates random cloud with fractal clumping
static void makecloud(AtArray *pointarr, AtArray *radarr, AtVector cenIn, float rad, int recursions)
{
   for (int i = 0; i < G_npoints; i++)
   {
      AtVector cen = randvec(0.5) * rad;
      AiArraySetPnt(pointarr, G_counter, cen+cenIn);
      AiArraySetFlt(radarr, G_counter, G_sphererad);
      G_counter++;
      if (recursions > 1)
         makecloud(pointarr, radarr, cen + cenIn, rad * 0.5, recursions - 1);
   }
}
 
static int MyInit(AtNode *mynode, void **user_ptr)
{
   *user_ptr = mynode; // make a copy of the parent procedural  
   srand(AiNodeGetInt(mynode, "G_seed"));
   G_npoints    = AiNodeGetInt(mynode, "G_npoints");
   G_sphererad  = AiNodeGetFlt(mynode, "G_sphererad");
   G_cloudrad   = AiNodeGetFlt(mynode, "G_cloudrad") - G_sphererad;
   G_recursions = AiNodeGetInt(mynode, "G_recursions");
   int npoints = 0;
   for (int i = 0; i < G_recursions; i++)
      npoints += pow(G_npoints, i);
   AiMsgInfo("[randflake] NPOINTS: %d", npoints);
   G_computedtotal = npoints;
   G_counter = 0;
   return 1;
}
 
static int MyCleanup(void *user_ptr)
{
   return 1;
}
 
static int MyNumNodes(void *user_ptr)
{
   return 1;
}
 
static AtNode *MyGetNode(void *user_ptr, int i)
{
   AtArray *pointarray  = AiArrayAllocate(G_computedtotal, 1, AI_TYPE_POINT);
   AtArray *radiusarray = AiArrayAllocate(G_computedtotal, 1, AI_TYPE_FLOAT);
   AtVector center;
   center.x = center.y = center.z = 0;
   makecloud(pointarray, radiusarray, center, G_cloudrad, G_recursions - 1);
 
   AtNode *node = AiNode("points");
   AiNodeSetArray(node, "points", pointarray);
   AiNodeSetArray(node, "radius", radiusarray);
   AiNodeSetStr(node, "mode", "sphere");
    
   return node;
}
 
// vtable passed in by proc_loader macro define
proc_loader
{
   vtable->Init     = MyInit;
   vtable->Cleanup  = MyCleanup;
   vtable->NumNodes = MyNumNodes;
   vtable->GetNode  = MyGetNode;
   strcpy(vtable->version, AI_VERSION);
   return 1;
}