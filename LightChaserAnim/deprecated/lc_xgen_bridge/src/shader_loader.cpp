#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* LcXgenRGBMethods;
extern AtNodeMethods* LcXgenRGBAMethods;

enum SHADERS
{
   LcXgenRGB = 0,
   LcXgenRGBA,
};

node_loader
{
   switch (i) 
   {     
      case LcXgenRGB:
         node->methods     = (AtNodeMethods*) LcXgenRGBMethods;
         node->output_type = AI_TYPE_RGB;
         node->name        = "lc_xgen_rgb";
         node->node_type   = AI_NODE_SHADER;
      break;
      case LcXgenRGBA:
         node->methods     = (AtNodeMethods*) LcXgenRGBAMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "lc_xgen_rgba";
         node->node_type   = AI_NODE_SHADER;
      break;
      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}