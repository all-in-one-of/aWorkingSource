#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

using namespace std;

// Procedural parameters
struct SpeedTreeData
{
   AtString filename;
   int thread;
   float offset;
   AtArray *motion_samples;
   AtString shader_roots;
   AtString shader_trunks;
   AtString shader_branches;
   AtString shader_leaves;
};
  
AtNode * create_example_cube()
{
   AtNode *polymesh = AiNode( "polymesh" );
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
   AtArray *pointArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );
   AtArray *nSidesArray = AiArrayAllocate( (uint32_t) polyCount, 1, AI_TYPE_UINT );
   AtArray *vertexArray = AiArrayAllocate( (uint32_t) polyCount * 3, 1, AI_TYPE_UINT );

   AtArray *normalArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );

   for ( unsigned int i = 0; i < vertexCount; i++ ) {
      // set position data
      int index = i * 3;
      AtVector tmpPoint = AtVector ( point[index], point[index + 1], point[index + 2] );
      AiArraySetVec( pointArray, i, tmpPoint );

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

   AiNodeSetArray( polymesh, "vlist", pointArray );
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
   AiParameterInt("thread", 1);
   AiParameterArray("motion_samples", AiArray(-0.25f, 0.25f, AI_TYPE_FLOAT));
   AiParameterStr("shader_roots", "");
   AiParameterStr("shader_trunks", "");
   AiParameterStr("shader_branches", "");
   AiParameterStr("shader_leaves", "");
}

procedural_init
{
   SpeedTreeData *proc_data = new SpeedTreeData();
   *user_ptr = proc_data;
   proc_data -> filename = AiNodeGetStr(node, "filename");
   proc_data -> offset = AiNodeGetFlt(node, "offset");
   proc_data -> thread = AiNodeGetInt(node, "thread");
   proc_data -> motion_samples = AiNodeGetArray(node, "motion_samples");
   proc_data -> shader_roots = AiNodeGetStr(node, "shader_roots");
   proc_data -> shader_trunks = AiNodeGetStr(node, "shader_trunks");
   proc_data -> shader_branches = AiNodeGetStr(node, "shader_branches");
   proc_data -> shader_leaves = AiNodeGetStr(node, "shader_leaves");

   return true;
}
  
procedural_cleanup
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   delete proc_data;
   return true;
}
  
procedural_num_nodes
{
   return 1;
}
  
procedural_get_node
{
  
   AtNode *meshNode = create_example_cube();
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