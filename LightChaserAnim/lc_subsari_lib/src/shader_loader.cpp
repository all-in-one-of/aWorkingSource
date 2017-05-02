#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* HistogameScanMethods;

enum SHADERS
{
   HistogameScan,
};

node_loader
{
   switch (i) 
   {     
      case HistogameScan:
         node->methods     = (AtNodeMethods*) HistogameScanMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "HistogameScan";
         node->node_type   = AI_NODE_SHADER;
      break;
      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}