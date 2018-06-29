#include <ai.h>
#include <cstring>
#include <cstdlib>
  
// Procedural parameters
struct RandomFlake
{
   int   count;
   float flake_radius;
   float sphere_radius;
   int   recursions;
   int   counter;
   int   num_points;
};
  
// returns a random vector in a unit sphere with a
// power function to bias it towards the center
static AtVector random_vector(float power)
{
   AtVector out(drand48() - 0.5, drand48() - 0.5, drand48() - 0.5);
   return AiV3Normalize(out) * pow(drand48(), power);
}
  
// recursive function that creates random flake with fractal clumping
static void make_cloud(RandomFlake *flake, AtArray *point_array, AtArray *radius_array, AtVector center, float radius, int recursions)
{
   for (int i = 0; i < flake->count; i++)
   {
      AtVector new_center = random_vector(0.5) * radius;
      AiArraySetVec(point_array, flake->counter, new_center + center);
      AiArraySetFlt(radius_array, flake->counter, flake->sphere_radius);
      flake->counter++;
      if (recursions > 1)
         make_cloud(flake, point_array, radius_array, new_center + center, radius * 0.5, recursions - 1);
   }
}
  
AI_PROCEDURAL_NODE_EXPORT_METHODS(RandomFlakeMtd);
  
node_parameters
{
   AiParameterInt("count"        , 10);
   AiParameterInt("recursions"   , 5);
   AiParameterFlt("sphere_radius", 0.01f);
   AiParameterFlt("flake_radius" , 10.0f);
   AiParameterInt("seed"         , 0);
}
  
procedural_init
{
   RandomFlake *flake = new RandomFlake();
   *user_ptr = flake;
  
   srand48(AiNodeGetInt(node, "seed"));
  
   flake->count         = AiNodeGetInt(node, "count");
   flake->sphere_radius = AiNodeGetFlt(node, "sphere_radius");
   flake->flake_radius  = AiNodeGetFlt(node, "flake_radius") - flake->sphere_radius;
   flake->recursions    = AiNodeGetInt(node, "recursions");
   flake->counter       = 0;
  
   flake->num_points = 0;
   for (int i = 0; i < flake->recursions; i++)
      flake->num_points += pow(flake->count, i);
   AiMsgInfo("[random_flake] number of points: %d", flake->num_points);
  
   return true;
}
  
procedural_cleanup
{
   RandomFlake *flake = (RandomFlake*)user_ptr;
   delete flake;
   return true;
}
  
procedural_num_nodes
{
   return 1;
}
  
procedural_get_node
{
   RandomFlake *flake = (RandomFlake*)user_ptr;
   AtArray *point_array    = AiArrayAllocate(flake->num_points, 1, AI_TYPE_VECTOR);
   AtArray *radius_array   = AiArrayAllocate(flake->num_points, 1, AI_TYPE_FLOAT);
  
   make_cloud(flake, point_array, radius_array, AI_V3_ZERO, flake->flake_radius, flake->recursions - 1);
  
   // create node with procedural node as parent
   AtNode *points_node = AiNode("points", "flake", node);
   AiNodeSetArray(points_node, "points", point_array);
   AiNodeSetArray(points_node, "radius", radius_array);
   AiNodeSetStr  (points_node, "mode"  , "sphere");
  
   return points_node;
}
  
node_loader
{
   if (i>0)
      return false;
  
   node->methods      = RandomFlakeMtd;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "random_flake";
   node->node_type    = AI_NODE_SHAPE_PROCEDURAL;
   strcpy(node->version, AI_VERSION);
  
   return true;
}