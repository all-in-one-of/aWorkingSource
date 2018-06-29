#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(UserDataFloatMethods);

enum UserDataParams
{
   p_attribute = 0,
   p_default,
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterFlt("default_value", 0.0f);
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
   AtString attribute = AiShaderEvalParamStr(p_attribute);
   float default_value = AiShaderEvalParamFlt(p_default);
   float result;
   float value;

   if (AiUDataGetFlt(attribute, value))
      result = value;
   else
      result = default_value;

   sg->out.FLT() = result;
}
