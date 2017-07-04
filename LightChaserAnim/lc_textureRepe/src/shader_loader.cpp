#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* TextureRepetitionMethods;
extern AtNodeMethods* TextureBlendMethods;

enum SHADERS
{
   TextureRepetition,
   TextureBlend,
};

node_loader
{
   switch (i) 
   {     
      case TextureRepetition:
         node->methods     = (AtNodeMethods*) TextureRepetitionMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "TextureRepetition";
         node->node_type   = AI_NODE_SHADER;
      break;
      case TextureBlend:
         node->methods     = (AtNodeMethods*) TextureBlendMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "TextureBlend";
         node->node_type   = AI_NODE_SHADER;
      break;
      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}