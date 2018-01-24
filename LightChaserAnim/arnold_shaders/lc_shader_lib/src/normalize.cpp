#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(NormalizeMethods);

enum NormalizeParams
{
   p_input,
};

node_parameters
{
   AiParameterVec("input", 1.0f, 1.0f, 1.0f);
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
   sg->out.VEC = AiV3Normalize(AiShaderEvalParamVec(p_input));
}


void registerNormalize(AtNodeLib *node)
{
   node->methods     = NormalizeMethods;
   node->output_type = AI_TYPE_VECTOR;
   node->name        = SHADERLIB_PREFIX "normalize";
   node->node_type   = AI_NODE_SHADER;
}

