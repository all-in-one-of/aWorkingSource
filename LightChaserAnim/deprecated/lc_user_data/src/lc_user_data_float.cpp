#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(LCAUserDataFloatMethods);

enum UserDataFloatParams
{
   p_attribute,
   p_default_value
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
   float default_value = AiShaderEvalParamFlt(p_default_value);
   float result;

   float value = default_value;
   result = default_value;

   if (AiUDataGetFlt(attribute, value))
      result = value;
   else
      AiMsgWarning("LCA inhouse shader logging:%s is invaid", attribute.c_str());

   sg->out.FLT() = result;
}
