#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(UserDataStringMethods);

enum UserDataParams
{
   p_attribute = 0,
   p_default,
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterStr("default_value", "");
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
   AtString default_value = AiShaderEvalParamStr(p_default);
   AtString result;
   AtString value;

   if (AiUDataGetStr(attribute, value))
      result = value;
   else
      result = default_value;
   
   sg->out.STR() = result;
}
