#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* cryptomatteMethods;
extern AtNodeMethods* alSurfaceMethods;

enum SHADERS
{
   cryptomatte,
   alSurface,
};

node_loader
{
   switch (i) 
   {     
      case cryptomatte:
         node->methods     = (AtNodeMethods*) cryptomatteMethods;
         node->output_type = AI_TYPE_RGB;
         node->name        = "cryptomatte";
         node->node_type   = AI_NODE_SHADER;
      break;
      case alSurface:
         node->methods     = (AtNodeMethods*) alSurfaceMethods;
         node->output_type = AI_TYPE_RGB;
         node->name        = "alSurface";
         node->node_type   = AI_NODE_SHADER;
      break;
      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}