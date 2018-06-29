
#include <ai.h>
#include <cstring>
AI_SHADER_NODE_EXPORT_METHODS(KTSimpleTestMethods);

enum SimpleParams { 
   p_vector = 0,
   p_attribute,
   p_value,
};

node_parameters
{

   AiParameterRGB("vector", 0.7f, 0.7f, 0.7f);
   AiParameterStr("attribute", "");
   AiParameterFlt("value", 1.0f);
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
   AtVector vector = AiShaderEvalParamVec(p_vector);
   AtString attribute = AiShaderEvalParamStr(p_attribute);
   float value = AiShaderEvalParamFlt(p_value);
   AiMsgWarning("KT Debug: attribute is %s", attribute.c_str());
   AiMsgWarning("KT Debug: value is %f", value);
   float user_value;
   
   if (AiUDataGetFlt(attribute,user_value))
      AiMsgWarning("KT Debug: kt value is %f", user_value);
   else
      AiMsgWarning("KT Debug: kt value is not found!");
   sg->out.VEC() = vector;
}

node_loader
{
   if (i > 0)
      return false;

   node->methods     = KTSimpleTestMethods;
   node->output_type = AI_TYPE_VECTOR;
   node->name        = "dispReadUserdata";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}
