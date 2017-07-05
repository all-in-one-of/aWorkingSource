#include "shaderlib.h"

#include <ai.h>

#include <algorithm>


AI_SHADER_NODE_EXPORT_METHODS(MaxMethods);

enum MaxParams
{
   p_input,
   p_max
};

node_parameters
{
   AiParameterRGB("input", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("max",   0.0f, 0.0f, 0.0f);
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
   AtColor max = AiShaderEvalParamRGB(p_max);
   
   sg->out.RGB.r = std::max(input.r, max.r);
   sg->out.RGB.g = std::max(input.g, max.g);
   sg->out.RGB.b = std::max(input.b, max.b);
}


void registerMax(AtNodeLib *node)
{
   node->methods     = MaxMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "max";
   node->node_type   = AI_NODE_SHADER;
}

