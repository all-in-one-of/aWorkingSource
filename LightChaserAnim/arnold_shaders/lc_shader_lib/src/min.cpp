#include "shaderlib.h"

#include <ai.h>

#include <algorithm>


AI_SHADER_NODE_EXPORT_METHODS(MinMethods);

enum MinParams
{
   p_input,
   p_min
};

node_parameters
{
   AiParameterRGB("input", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("min",   0.0f, 0.0f, 0.0f);
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
   AtColor min = AiShaderEvalParamRGB(p_min);
   
   sg->out.RGB.r = std::min(input.r, min.r);
   sg->out.RGB.g = std::min(input.g, min.g);
   sg->out.RGB.b = std::min(input.b, min.b);
}


void registerMin(AtNodeLib *node)
{
   node->methods     = MinMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "min";
   node->node_type   = AI_NODE_SHADER;
}

