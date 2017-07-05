#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(ClampMethods);

enum ClampParams
{
   p_input,
   p_min,
   p_max
};

node_parameters
{
   AiParameterRGB("input", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("min",   0.0f);
   AiParameterFlt("max",   1.0f);
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
   float min = AiShaderEvalParamFlt(p_min);
   float max = AiShaderEvalParamFlt(p_max);
   sg->out.RGB = AiColorClamp(input, min, max);
}


void registerClamp(AtNodeLib *node)
{
   node->methods     = ClampMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "clamp";
   node->node_type   = AI_NODE_SHADER;
}

