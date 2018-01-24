#include "color_utils.h"
#include "shaderlib.h"

#include <ai.h>

#include <cmath>


AI_SHADER_NODE_EXPORT_METHODS(ColorCorrectMethods);

enum ColorCorrectParams
{
   p_input,
   p_hue_shift,
   p_saturation,
   p_brighten,
   p_contrast,
   p_contrast_pivot,
   p_exposure,
   p_gain,
   p_gamma,
};

node_parameters
{
   AiParameterRGB("input",          0.0f, 0.0f, 0.0f);
   AiParameterFlt("hue_shift",      0.0f);
   AiParameterFlt("saturation",     1.0f);
   AiParameterFlt("brighten",       0.0f);
   AiParameterFlt("contrast",       1.0f);
   AiParameterFlt("contrast_pivot", 0.5f);
   AiParameterFlt("exposure",       0.0f);
   AiParameterFlt("gain",           1.0f);
   AiParameterFlt("gamma",          1.0f);
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
   float hue_shift = AiShaderEvalParamFlt(p_hue_shift);
   float saturation = AiShaderEvalParamFlt(p_saturation);
   float brighten = AiShaderEvalParamFlt(p_brighten);
   float contrast = AiShaderEvalParamFlt(p_contrast);
   float exposure = AiShaderEvalParamFlt(p_exposure);
   float gain = AiShaderEvalParamFlt(p_gain);
   float gamma = AiShaderEvalParamFlt(p_gamma);
   
   // apply hue and saturation changes (but only if the parameters are
   // non-identity; the conversion to/from HSL isn't amazingly cheap)
   if (hue_shift != 0.0f || saturation != 1.0f)
   {
      AtColor hsl = convertFromRGB(color, COLOR_SPACE_HSL);
      
      // apply hue, keep it in 0..1
      hsl.r += hue_shift;
      hsl.r = hsl.r - floorf(hsl.r);
      
      // apply saturation
      hsl.g *= saturation;
      color = convertToRGB(hsl, COLOR_SPACE_HSL);
   }
   
   // apply contrast
   if (contrast != 1.0f)
   {
      AtColor contrast_pivot = AiColor(AiShaderEvalParamFlt(p_contrast_pivot));
      color = (color - contrast_pivot) * contrast + contrast_pivot;
   }
   
   if (brighten != 0.0f)
      color += brighten;
   
   if (exposure != 0.0f)
      color *= powf(2.0f, exposure);

   if (gain != 1.0f)
      color *= gain;
   
   if (gamma != 1.0f)
      AiColorGamma(&color, gamma);

   sg->out.RGB = color;
}


void registerColorCorrect(AtNodeLib *node)
{
   node->methods     = ColorCorrectMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "color_correct";
   node->node_type   = AI_NODE_SHADER;
}

