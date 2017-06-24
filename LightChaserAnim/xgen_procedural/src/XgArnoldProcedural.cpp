#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#include "common/XgArnoldExpand.h"
#include <ai.h>
#include <vector>
#include <string>

using namespace XGenArnold;

// Redirect Init/Cleanup/NumNodes/GetNode to our XGenArnoldProcedural class wrapped in the user data.
static int Init( AtNode* node, void** user_ptr )
{
   //AiMsgInfo("[xgArnoldProcedural] Init()");

   // Isn't this a bug on arnold side ?
   // If this procedural node is disabled, we shouldn't even get here
   if (AiNodeIsDisabled(node))
   {
      *user_ptr = 0;
      return 1;
   }
   ProceduralWrapper* ud = (ProceduralWrapper*)AiNodeGetPtr( node, "userptr" );

   // Create a brand new one.
   if( ud==NULL )
   {
      ud = new ProceduralWrapper( new Procedural(), false /* Won't do cleanup */ );
      if( !ud )
        return 0;
   }

   *user_ptr = (void*)ud;

   return ud->Init( node );
}

// Cleanup
static int Cleanup( void* user_ptr )
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
static int NumNodes( void* user_ptr )
{
   //AiMsgInfo("[xgArnoldProcedural] NumNodes()");

   ProceduralWrapper* ud = (ProceduralWrapper*)user_ptr;
   if( !ud )
      return 0;
   return ud->NumNodes();
}

// Get the i_th node
static AtNode* GetNode( void* user_ptr, int i )
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

AI_EXPORT_LIB int ProcLoader(AtProcVtable *vtable)
{
   vtable->Init = Init;
   vtable->Cleanup = Cleanup;
   vtable->NumNodes = NumNodes;
   vtable->GetNode = GetNode;

   s_bCleanDescriptionCache = true;
   sprintf(vtable->version, AI_VERSION);
   return 1;
}

#ifdef __cplusplus
}
#endif

