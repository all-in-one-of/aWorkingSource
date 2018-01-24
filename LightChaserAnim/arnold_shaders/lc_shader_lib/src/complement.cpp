#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(ComplementMethods);

enum ComplementParams
{
   p_input,
};

node_parameters
{
   AiParameterRGB("input", 1.0f, 1.0f, 1.0f);
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
   sg->out.RGB = AI_RGB_WHITE - AiShaderEvalParamRGB(p_input);
}


void registerComplement(AtNodeLib *node)
{
   node->methods     = ComplementMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "complement";
   node->node_type   = AI_NODE_SHADER;
}
