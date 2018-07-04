#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "constants.h"
#include "geometry.h"

using namespace std;

AI_PROCEDURAL_NODE_EXPORT_METHODS(SpeedTreeMethods);

node_parameters
{
   AiParameterStr("filename", "");
   AiParameterFlt("offset", 0.0f);
   AiParameterInt("threads", 1);
   AiParameterArray("motionSamples", AiArray(-0.25f, 0.25f, AI_TYPE_FLOAT));
}

procedural_init
{
   SpeedTreeData *proc_data = new SpeedTreeData();
   *user_ptr = proc_data;
   proc_data -> filename = AiNodeGetStr(node, "filename");
   proc_data -> offset = AiNodeGetFlt(node, "offset");
   proc_data -> threads = AiNodeGetInt(node, "threads");
   proc_data -> motionSamples = AiNodeGetArray(node, "motionSamples");

   proc_data -> id = 0;
   proc_data -> num = SPEEDTREE_POLY_COUNT;
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
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   return proc_data->num;
}

procedural_get_node
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   
   AtNode* polymesh = create_quadrangle_cube(node, proc_data);
   return polymesh;
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