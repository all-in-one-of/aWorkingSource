#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(CrossMethods);

enum CrossParams
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
   sg->out.VEC = AiV3Cross(AiShaderEvalParamVec(p_input1), AiShaderEvalParamVec(p_input2));
}


void registerCross(AtNodeLib *node)
{
   node->methods     = CrossMethods;
   node->output_type = AI_TYPE_VECTOR;
   node->name        = SHADERLIB_PREFIX "cross";
   node->node_type   = AI_NODE_SHADER;
}

