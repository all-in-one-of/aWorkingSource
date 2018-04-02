#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(UserDataGatherMethods);

enum UserDataGatherParams
{
   p_attribute,
   p_default,
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterVec("default", 0.0f, 0.0f, 0.0f);
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
   AtString attribute_value = AiShaderEvalParamStr(p_attribute);

   AtVector default_value = AiShaderEvalParamVec(p_default);
   AtVector result;
   AtRGB result_rgb;
   AtRGBA result_rgba;
   AtVector result_vector;
   int result_int;float result_float;
   if ( AiUDataGetRGBA(attribute_value,result_rgba) )
      result = AtVector(result_rgba.r, result_rgba.g, result_rgba.b);
   else if ( AiUDataGetRGB(attribute_value,result_rgb) )
      result = AtVector(result_rgb.r, result_rgb.g, result_rgb.b);
   else if ( AiUDataGetVec (attribute_value,result_vector) )
      result = result_vector;
   else if ( AiUDataGetFlt (attribute_value,result_float) )
      result = AtVector(result_float,result_float,result_float);
   else if ( AiUDataGetInt (attribute_value,result_int) )
      result = AtVector(result_int,result_int,result_int);
   else 
      result = default_value;
   sg->out.VEC() = result;
}


node_loader
{
      if (i > 0)
        return false;
      node->methods = UserDataGatherMethods;
      node->output_type = AI_TYPE_VECTOR;
      node->name = "lc_user_data_gather";
      node->node_type = AI_NODE_SHADER;
      strcpy(node->version, AI_VERSION);
      return true;
}
