#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(MultiplyMethods);

enum MultiplyParams
{
   p_input1,
   p_input2
};

node_parameters
{
   AiParameterRGB("input1", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("input2", 1.0f, 1.0f, 1.0f);
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
   sg->out.RGB = AiShaderEvalParamRGB(p_input1) * AiShaderEvalParamRGB(p_input2);
}


void registerMultiply(AtNodeLib *node)
{
   node->methods     = MultiplyMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "multiply";
   node->node_type   = AI_NODE_SHADER;
}

