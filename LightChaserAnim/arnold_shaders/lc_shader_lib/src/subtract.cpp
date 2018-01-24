#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(SubtractMethods);

enum SubtractParams
{
   p_input1,
   p_input2
};

node_parameters
{
   AiParameterRGB("input1", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("input2", 0.0f, 0.0f, 0.0f);
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
   sg->out.RGB = AiShaderEvalParamRGB(p_input1) - AiShaderEvalParamRGB(p_input2);
}


void registerSubtract(AtNodeLib *node)
{
   node->methods     = SubtractMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "subtract";
   node->node_type   = AI_NODE_SHADER;
}
