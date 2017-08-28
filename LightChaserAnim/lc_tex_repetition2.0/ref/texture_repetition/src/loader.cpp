#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* TextureRepetitionMethods;
extern AtNodeMethods* TextureBlendMethods;
extern AtNodeMethods* TextureBlendXMethods;
extern AtNodeMethods* TextureBlendYMethods;
enum SHADERS
{
   TextureRepetition,
   TextureBlend,
   TextureBlendX,
   TextureBlendY,
};

node_loader
{
   switch (i) 
   {     
      case TextureRepetition:
         node->methods     = (AtNodeMethods*) TextureRepetitionMethods;
         node->output_type = AI_TYPE_RGB;
         node->name        = "TextureRepetition";
         node->node_type   = AI_NODE_SHADER;
      break;
      case TextureBlend:
         node->methods     = (AtNodeMethods*) TextureBlendMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "TextureBlend";
         node->node_type   = AI_NODE_SHADER;
      break;
      case TextureBlendX:
         node->methods     = (AtNodeMethods*) TextureBlendXMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "TextureBlendX";
         node->node_type   = AI_NODE_SHADER;
      break;
      case TextureBlendY:
         node->methods     = (AtNodeMethods*) TextureBlendYMethods;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "TextureBlendY";
         node->node_type   = AI_NODE_SHADER;
      break;

      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}