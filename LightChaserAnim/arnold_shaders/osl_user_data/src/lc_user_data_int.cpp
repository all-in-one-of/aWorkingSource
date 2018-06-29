#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(UserDataIntMethods);

enum UserDataParams
{
   p_attribute = 0,
   p_default,
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterInt("default_value", 0);
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
   float default_value = AiShaderEvalParamInt(p_default);
   int result;
   int value;

   if (AiUDataGetInt(attribute, value))
      result = value;
   else
      result = default_value;
   
   sg->out.INT() = result;
}
