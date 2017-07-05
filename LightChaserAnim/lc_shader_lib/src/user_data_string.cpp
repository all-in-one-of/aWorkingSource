#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(UserDataStringMethods);

enum UserDataStringParams
{
    p_attribute,
    p_default
};

node_parameters
{
    AiParameterStr("attribute", "");
    AiParameterStr("default", "");
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
   const AtChar* attribute = AiShaderEvalParamStr(p_attribute);
   const AtChar* value = NULL;

   if (AiUDataGetStr(attribute, &value))
      sg->out.STR = value;
   else
      sg->out.STR = AiShaderEvalParamStr(p_default);
}


void registerUserDataString(AtNodeLib *node)
{
   node->methods     = UserDataStringMethods;
   node->output_type = AI_TYPE_STRING;
   node->name        = SHADERLIB_PREFIX "user_data_string";
   node->node_type   = AI_NODE_SHADER;
}

