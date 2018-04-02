#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(LCAUserDataIntMethods);

enum UserDataIntParams
{
   p_attribute,
   p_default_value
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
   int default_value = AiShaderEvalParamInt(p_default_value);
   int result;

   int value = default_value;
   result = default_value;

   if (AiUDataGetInt(attribute, value))
      result = value;
   else
      AiMsgWarning("LCA inhouse shader logging:%s is invaid", attribute.c_str());

   sg->out.INT() = result;
}

