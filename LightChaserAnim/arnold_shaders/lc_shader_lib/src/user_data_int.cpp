#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(UserDataIntMethods);

enum UserDataIntParams
{
   p_attribute,
   p_default
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterInt("default", 0);
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
   AtParamValue value;

   if (AiUDataGetInt(attribute, &value.INT))
      sg->out.INT = value.INT;
   else if (AiUDataGetUInt(attribute, &value.UINT))
      sg->out.INT = value.UINT;
   else if (AiUDataGetByte(attribute, &value.BYTE))
      sg->out.INT = value.BYTE;
   else if (AiUDataGetBool(attribute, &value.BOOL))
      sg->out.INT = value.BOOL;
   else
      sg->out.INT = AiShaderEvalParamFlt(p_default);
}


void registerUserDataInt(AtNodeLib *node)
{
    node->methods     = UserDataIntMethods;
    node->output_type = AI_TYPE_INT;
    node->name        = SHADERLIB_PREFIX "user_data_int";
    node->node_type   = AI_NODE_SHADER;
}

