#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(UserDataFloatMethods);

enum UserDataFloatParams
{
   p_attribute,
   p_default
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterFlt("default", 0.0f);
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

   if (AiUDataGetFlt(attribute, &value.FLT))
      sg->out.FLT = value.FLT;
   else if (AiUDataGetInt(attribute, &value.INT))
      sg->out.FLT = value.INT;
   else if (AiUDataGetUInt(attribute, &value.UINT))
      sg->out.FLT = value.UINT;
   else if (AiUDataGetByte(attribute, &value.BYTE))
      sg->out.FLT = value.BYTE;
   else if (AiUDataGetBool(attribute, &value.BOOL))
      sg->out.FLT = value.BOOL;
   else
      sg->out.FLT = AiShaderEvalParamFlt(p_default);
}


void registerUserDataFloat(AtNodeLib *node)
{
   node->methods     = UserDataFloatMethods;
   node->output_type = AI_TYPE_FLOAT;
   node->name        = SHADERLIB_PREFIX "user_data_float";
   node->node_type   = AI_NODE_SHADER;
}

