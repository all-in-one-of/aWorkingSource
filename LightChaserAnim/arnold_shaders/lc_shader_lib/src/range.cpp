#include "shaderlib.h"

#include <ai.h>

#include <cmath>


AI_SHADER_NODE_EXPORT_METHODS(RangeMethods);

enum RangeParams
{
   p_input,
   p_input_min,
   p_input_max,
   p_output_min,
   p_output_max,
   p_smoothstep
};

node_parameters
{
   AiParameterRGB ("input",      0.0f, 0.0f, 0.0f);
   AiParameterFlt ("input_min",  0.0f);
   AiParameterFlt ("input_max",  1.0f);
   AiParameterFlt ("output_min", 0.0f);
   AiParameterFlt ("output_max", 1.0f);
   AiParameterBool("smoothstep", false);
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
   float input_min = AiShaderEvalParamFlt(p_input_min);
   float input_max = AiShaderEvalParamFlt(p_input_max);
   float output_min = AiShaderEvalParamFlt(p_output_min);
   float output_max = AiShaderEvalParamFlt(p_output_max);

   bool smoothstep = AiShaderEvalParamBool(p_smoothstep);

   AtColor canonical = (input - input_min) / (input_max - input_min);

   if (smoothstep)
   {
      canonical.r = SMOOTHSTEP(0.0f, 1.0f, canonical.r);
      canonical.g = SMOOTHSTEP(0.0f, 1.0f, canonical.g);
      canonical.b = SMOOTHSTEP(0.0f, 1.0f, canonical.b);
   }
   sg->out.RGB = canonical * (output_max - output_min) + AiColor(output_min);
}


void registerRange(AtNodeLib *node)
{
   node->methods     = RangeMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "range";
   node->node_type   = AI_NODE_SHADER;
}

