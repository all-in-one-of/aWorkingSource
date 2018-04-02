#include <ai.h>

#include <iostream>
#include <cstring>

extern AtNodeMethods* LCAUserDataFloatMethods;
extern AtNodeMethods* LCAUserDataIntMethods;

enum SHADERS
{
   LCAUserDataFloat,
   LCAUserDataInt,
};

node_loader
{
   switch (i) 
   {     
      case LCAUserDataFloat:
         node->methods     = (AtNodeMethods*) LCAUserDataFloatMethods;
         node->output_type = AI_TYPE_FLOAT;
         node->name        = "lc_user_data_float";
         node->node_type   = AI_NODE_SHADER;
      break;
      case LCAUserDataInt:
         node->methods     = (AtNodeMethods*) LCAUserDataIntMethods;
         node->output_type = AI_TYPE_INT;
         node->name        = "lc_user_data_int";
         node->node_type   = AI_NODE_SHADER;
      break;
      default:
         return false;      
   }

   strcpy(node->version, AI_VERSION);
   return true;
}