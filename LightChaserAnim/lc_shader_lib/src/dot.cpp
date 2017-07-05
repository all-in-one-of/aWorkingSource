#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(DotMethods);

enum DotParams
{
   p_input1,
   p_input2,
};

node_parameters
{
   AiParameterVec("input1", 1.0f, 1.0f, 1.0f);
   AiParameterVec("input2", 1.0f, 1.0f, 1.0f);
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
   sg->out.FLT = AiV3Dot(AiShaderEvalParamVec(p_input1), AiShaderEvalParamVec(p_input2));
}


void registerDot(AtNodeLib *node)
{
   node->methods     = DotMethods;
   node->output_type = AI_TYPE_FLOAT;
   node->name        = SHADERLIB_PREFIX "dot";
   node->node_type   = AI_NODE_SHADER;
}

