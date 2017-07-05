#include "color_utils.h"
#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(ColorConvertMethods);

enum ColorConvertParams
{
   p_input,
   p_from_space,
   p_to_space
};

static const char* enum_color_space[] = { "RGB", "XYZ", "xyY", "HSL", "HSV", NULL };

node_parameters
{
   AiParameterRGB ("input",      1.0f, 1.0f, 1.0f);
   AiParameterEnum("from_space", COLOR_SPACE_RGB, enum_color_space);
   AiParameterEnum("to_space",   COLOR_SPACE_RGB, enum_color_space);
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
   AtColor input = AiShaderEvalParamRGB(p_input);
   int from_space = AiShaderEvalParamInt(p_from_space);
   int to_space = AiShaderEvalParamInt(p_to_space);
   
   // passthrough for non-differing spaces
   if (from_space == to_space)
   {
      sg->out.RGB = input;
      return;
   }
   
   // special (fast) conversions between XYZ and xyY
   if (from_space == COLOR_SPACE_XYZ && to_space == COLOR_SPACE_XYY)
   {
      sg->out.RGB = XYZToxyY(input);
      return;
   }
   else if (from_space == COLOR_SPACE_XYY && to_space == COLOR_SPACE_XYZ)
   {
      sg->out.RGB = xyYToXYZ(input);
      return;
   }
   
   AtColor rgbColor = convertToRGB(input, from_space);
   sg->out.RGB = convertFromRGB(rgbColor, to_space);
}


void registerColorConvert(AtNodeLib *node)
{
   node->methods     = ColorConvertMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "color_convert";
   node->node_type   = AI_NODE_SHADER;
}

