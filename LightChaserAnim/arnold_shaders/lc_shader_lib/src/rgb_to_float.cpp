#include "shaderlib.h"

#include <ai.h>

AI_SHADER_NODE_EXPORT_METHODS(RGBToFloatMethods);

enum RGBToFloatParams
{
   p_input,
   p_mode
};

#define RGB_TO_FLOAT_R          0
#define RGB_TO_FLOAT_G          1
#define RGB_TO_FLOAT_B          2
#define RGB_TO_FLOAT_MIN        3
#define RGB_TO_FLOAT_MAX        4
#define RGB_TO_FLOAT_AVERAGE    5
#define RGB_TO_FLOAT_SUM        6
#define RGB_TO_FLOAT_PERCEPTUAL 7

static const char* enum_rgb_to_float_mode[] =
{
    "r",
    "g",
    "b",
    "min",
    "max",
    "average",
    "sum",
    "perceptual",
    NULL
};

node_parameters
{
   AiParameterRGB ("input", 0.0f, 0.0f, 0.0f);
   AiParameterEnum("mode",  RGB_TO_FLOAT_AVERAGE, enum_rgb_to_float_mode);
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
   AtColor color = AiShaderEvalParamRGB(p_input);
   int mode = AiShaderEvalParamInt(p_mode);
   switch (mode)
   {
   case RGB_TO_FLOAT_R:
      sg->out.FLT = color.r;
      break;
      
   case RGB_TO_FLOAT_G:
      sg->out.FLT = color.g;
      break;
      
   case RGB_TO_FLOAT_B:
      sg->out.FLT = color.b;
      break;
      
   case RGB_TO_FLOAT_MIN:
      sg->out.FLT = MIN3(color.r, color.g, color.b);
      break;
      
   case RGB_TO_FLOAT_MAX:
      sg->out.FLT = AiColorMaxRGB(color);
      break;
      
   case RGB_TO_FLOAT_AVERAGE:
      sg->out.FLT = (color.r + color.g + color.b) / 3.0f;
      break;
      
   case RGB_TO_FLOAT_SUM:
      sg->out.FLT = color.r + color.g + color.b;
      break;
      
   case RGB_TO_FLOAT_PERCEPTUAL:
      // rec 709 / sRGB luminance mix
      sg->out.FLT = 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
      break;
      
   default:
      break;
   }
}


void registerRGBToFloat(AtNodeLib *node)
{
   node->methods     = RGBToFloatMethods;
   node->output_type = AI_TYPE_FLOAT;
   node->name        = SHADERLIB_PREFIX "rgb_to_float";
   node->node_type   = AI_NODE_SHADER;
}

