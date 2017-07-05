#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(AbsMethods);

enum AbsParams
{
   p_input
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
   sg->out.RGB = AiColorABS(AiShaderEvalParamRGB(p_input));
}


void registerAbs(AtNodeLib *node)
{
   node->methods     = AbsMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "abs";
   node->node_type   = AI_NODE_SHADER;
}

