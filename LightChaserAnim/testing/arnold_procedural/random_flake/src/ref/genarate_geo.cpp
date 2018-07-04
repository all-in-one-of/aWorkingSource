#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

using namespace std;

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

AtNode * make_plane()
{
   AtNode *polymesh = AiNode( "polymesh" );
   // AiNodeSetStr( polymesh, "name", "KT_PolyMesh" );
   AiNodeSetBool( polymesh, "opaque", true );
   AiNodeSetBool( polymesh, "smoothing", false );
   AtMatrix mat = AiM4Identity();
   AiNodeSetMatrix ( polymesh, "matrix", mat );

   static const int vertices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7, 6, 7, 0, 0, 7, 1, 1, 7, 3, 3, 7, 5, 6, 0, 4, 4, 0, 2};
   vector<uint8_t> vertices_list (vertices, vertices + 
      sizeof(vertices) / sizeof(vertices[0]) );

   static const float point[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> point_list (point, point + 
      sizeof(point) / sizeof(point[0]) );

   static const float normal[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> normal_list (normal, normal + 
      sizeof(normal) / sizeof(normal[0]) );

   unsigned int vertexCount = point_list.size()/3;
   unsigned int polyCount = vertices_list.size()/3;
   // create array to hold poly data
   AtArray *posArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );
   AtArray *nSidesArray = AiArrayAllocate( (uint32_t) polyCount, 1, AI_TYPE_UINT );
   AtArray *vertexArray = AiArrayAllocate( (uint32_t) polyCount * 3, 1, AI_TYPE_UINT );

   AtArray *normalArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );

   for ( unsigned int i = 0; i < vertexCount; i++ ) {
      // set position data
      int index = i * 3;
      AtVector tmpPoint = AtVector ( point[index], point[index + 1], point[index + 2] );
      AiArraySetVec( posArray, i, tmpPoint );

      AtVector tmpNormal = AtVector ( normal[index], normal[index + 1], normal[index + 2] );
      AiArraySetVec( normalArray, i, tmpNormal );
   }
   // get triangle data
   for ( unsigned int i = 0; i < polyCount; i++ ) {
     // set position data
     int index = i * 3;
     AiArraySetUInt( nSidesArray, i, 3 );
     AiArraySetUInt( vertexArray, index, vertices[index] );
     AiArraySetUInt( vertexArray, index + 1, vertices[index + 1] );
     AiArraySetUInt( vertexArray, index + 2, vertices[index + 2] );
   }

   AiNodeSetArray( polymesh, "vlist", posArray );
   AiNodeSetArray( polymesh, "nsides", nSidesArray );
   AiNodeSetArray( polymesh, "vidxs", vertexArray );

   AiNodeSetArray( polymesh, "nlist", normalArray );
   AiNodeSetArray( polymesh, "nidxs", AiArrayCopy(vertexArray) );

   return polymesh;
}

AI_PROCEDURAL_NODE_EXPORT_METHODS(SpeedTreeMethods);

node_parameters
{
   AiParameterStr("filename", "");
   AiParameterStr("shader_trunks", "");
   AiParameterStr("shader_branches", "");
   AiParameterStr("shader_leaves", "");
   AiParameterStr("shader_roots", "");

}

procedural_init
{
   return true;
}
  
procedural_cleanup
{
   return true;
}
  
procedural_num_nodes
{
   return true;
}
  
procedural_get_node
{
  
   AtNode *meshNode = make_plane();
   return meshNode;
}
  
node_loader
{
   if (i>0)
      return false;
  
   node->methods      = SpeedTreeMethods;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "speedtree_procedural";
   node->node_type    = AI_NODE_SHAPE_PROCEDURAL;
   strcpy(node->version, AI_VERSION);
  
   return true;
}