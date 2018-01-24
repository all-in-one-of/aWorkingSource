#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(UserDataRgbMethods);

enum UserDataRgbParams
{
   p_attribute,
   p_default
};

node_parameters
{
   AiParameterStr("attribute", "");
   AiParameterRGB("default", 0.0f, 0.0f, 0.0f);
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

   if (AiUDataGetRGB(attribute, &value.RGB))
      sg->out.RGB = value.RGB;
   else if (AiUDataGetRGBA(attribute, &value.RGBA))
   {
      sg->out.RGB.r = value.RGBA.r;
      sg->out.RGB.g = value.RGBA.g;
      sg->out.RGB.b = value.RGBA.b;
   }
   else if (AiUDataGetVec(attribute, &value.VEC))
   {
      sg->out.RGB.r = value.VEC.x;
      sg->out.RGB.g = value.VEC.y;
      sg->out.RGB.b = value.VEC.z;
   }
   else if (AiUDataGetPnt(attribute, &value.PNT))
   {
      sg->out.RGB.r = value.PNT.x;
      sg->out.RGB.g = value.PNT.y;
      sg->out.RGB.b = value.PNT.z;
   }
   else if (AiUDataGetPnt2(attribute, &value.PNT2))
   {
      sg->out.RGB.r = value.PNT2.x;
      sg->out.RGB.g = value.PNT2.y;
      sg->out.RGB.b = 0.0f;
   }
   else if (AiUDataGetFlt(attribute, &value.FLT))
      sg->out.RGB = value.FLT;
   else if (AiUDataGetInt(attribute, &value.INT))
      sg->out.RGB = value.INT;
   else if (AiUDataGetUInt(attribute, &value.UINT))
      sg->out.RGB = value.UINT;
   else if (AiUDataGetByte(attribute, &value.BYTE))
      sg->out.RGB = value.BYTE;
   else if (AiUDataGetBool(attribute, &value.BOOL))
      sg->out.RGB = value.BOOL;
   else
      sg->out.RGB = AiShaderEvalParamRGB(p_default);
}


void registerUserDataRgb(AtNodeLib *node)
{
   node->methods     = UserDataRgbMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "user_data_rgb";
   node->node_type   = AI_NODE_SHADER;
}

