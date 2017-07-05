#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(UserDataRgbaMethods);

enum UserDataRgbaParams
{
   p_attribute,
   p_default
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterRGBA("default", 0.0f, 0.0f, 0.0f, 1.0f);
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

   if (AiUDataGetRGBA(attribute, &value.RGBA))
      sg->out.RGBA = value.RGBA;
   else if (AiUDataGetRGB(attribute, &value.RGB))
   {
      sg->out.RGBA.r = value.RGB.r;
      sg->out.RGBA.g = value.RGB.g;
      sg->out.RGBA.b = value.RGB.b;
      sg->out.RGBA.a = 1.0f;
   }
   else if (AiUDataGetVec(attribute, &value.VEC))
   {
      sg->out.RGBA.r = value.VEC.x;
      sg->out.RGBA.g = value.VEC.y;
      sg->out.RGBA.b = value.VEC.z;
      sg->out.RGBA.a = 1.0f;
   }
   else if (AiUDataGetPnt(attribute, &value.PNT))
   {
      sg->out.RGBA.r = value.PNT.x;
      sg->out.RGBA.g = value.PNT.y;
      sg->out.RGBA.b = value.PNT.z;
      sg->out.RGBA.a = 1.0f;
   }
   else if (AiUDataGetPnt2(attribute, &value.PNT2))
   {
      sg->out.RGBA.r = value.PNT2.x;
      sg->out.RGBA.g = value.PNT2.y;
      sg->out.RGBA.b = 0.0f;
      sg->out.RGBA.a = 1.0f;
   }
   else if (AiUDataGetFlt(attribute, &value.FLT))
      sg->out.RGBA = value.FLT;
   else if (AiUDataGetInt(attribute, &value.INT))
      sg->out.RGBA = value.INT;
   else if (AiUDataGetUInt(attribute, &value.UINT))
      sg->out.RGBA = value.UINT;
   else if (AiUDataGetByte(attribute, &value.BYTE))
      sg->out.RGBA = value.BYTE;
   else if (AiUDataGetBool(attribute, &value.BOOL))
      sg->out.RGBA = value.BOOL;
   else
      sg->out.RGBA = AiShaderEvalParamRGBA(p_default);
}


void registerUserDataRgba(AtNodeLib *node)
{
   node->methods     = UserDataRgbaMethods;
   node->output_type = AI_TYPE_RGBA;
   node->name        = SHADERLIB_PREFIX "user_data_rgba";
   node->node_type   = AI_NODE_SHADER;
}

