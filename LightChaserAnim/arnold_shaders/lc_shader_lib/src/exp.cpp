#include "shaderlib.h"

#include <ai.h>

#include <cmath>


AI_SHADER_NODE_EXPORT_METHODS(ExpMethods);

enum ExpParams
{
    p_input,
};

node_parameters
{
    AiParameterRGB("input", 0.0f, 0.0f, 0.0f);
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
    AiColorCreate(sg->out.RGB, expf(input.r), expf(input.g), expf(input.b));
}


void registerExp(AtNodeLib *node)
{
    node->methods     = ExpMethods;
    node->output_type = AI_TYPE_RGB;
    node->name        = SHADERLIB_PREFIX "exp";
    node->node_type   = AI_NODE_SHADER;
}

