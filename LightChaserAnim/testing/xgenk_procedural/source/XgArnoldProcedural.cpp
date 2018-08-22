#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#include "XgArnoldExpand.h"
#include <ai.h>
#include <vector>
#include <string>

using namespace XGenArnold;

AI_PROCEDURAL_NODE_EXPORT_METHODS(XgkArnoldProceduralMtd);

node_parameters
{
	AiParameterStr("data", "");
}

procedural_init
{
   //AiMsgInfo("[xgArnoldProcedural] Init()");

   // Isn't this a bug on arnold side ?
   // If this procedural node is disabled, we shouldn't even get here
   if (AiNodeIsDisabled(node))
   {
      *user_ptr = 0;
      return 1;
   }
   ProceduralWrapper* ud = new ProceduralWrapper( new Procedural(), false /* Won't do cleanup */ );
   if( !ud )
     return 0;
   

   *user_ptr = (void*)ud;

   return ud->Init( node );
}

// Cleanup
procedural_cleanup
{
   //AiMsgInfo("[xgArnoldProcedural] Cleanup()");

   ProceduralWrapper* ud = (ProceduralWrapper*)user_ptr;
   if( !ud )
      return 0;
   int ret = ud->Cleanup();
   delete ud;
   return ret;
}

// Get number of nodes
procedural_num_nodes
{
   //AiMsgInfo("[xgArnoldProcedural] NumNodes()");

   ProceduralWrapper* ud = (ProceduralWrapper*)user_ptr;
   if( !ud )
      return 0;
   return ud->NumNodes();
}

// Get the i_th node
procedural_get_node
{
   //AiMsgInfo("[xgArnoldProcedural] GetNode()");

   ProceduralWrapper* ud = (ProceduralWrapper*)user_ptr;
   if( !ud )
      return 0;
   return ud->GetNode(i);
}

// DSO hook
#ifdef __cplusplus
extern "C"
{
#endif


node_loader
{
   if (i>0)
      return false;

   node->methods      = XgkArnoldProceduralMtd;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "xgenk_procedural";
   node->node_type    = AI_NODE_SHAPE_PROCEDURAL;
   strcpy(node->version, AI_VERSION);

   return true;
}

#ifdef __cplusplus
}
#endif

