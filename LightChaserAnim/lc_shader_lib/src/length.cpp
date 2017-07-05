#include "shaderlib.h"

#include <ai.h>

#include <cmath>


AI_SHADER_NODE_EXPORT_METHODS(LengthMethods);

enum LengthParams
{
   p_input,
   p_mode,
};

#define LENGTH_MANHATTAN 0
#define LENGTH_EUCLIDIAN 1
#define LENGTH_QUADRANCE 2

static const char* enum_length_mode[] =
{
   "manhattan",
   "euclidian",
   "quadrance",
   NULL
};

node_parameters
{
   AiParameterVec("input", 0.0f, 0.0f, 0.0f);
   AiParameterEnum("mode", LENGTH_EUCLIDIAN, enum_length_mode);
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
   AtVector vec = AiShaderEvalParamVec(p_input);

   switch (AiShaderEvalParamInt(p_mode))
   {
   case LENGTH_MANHATTAN:
      sg->out.FLT = std::abs(vec.x) + std::abs(vec.y) + std::abs(vec.z);
      break;

   case LENGTH_EUCLIDIAN:
      sg->out.FLT = AiV3Length(vec);
      break;

   case LENGTH_QUADRANCE:
      sg->out.FLT = AiV3Dot(vec, vec);
      break;

   default:
      break;
   }
}


void registerLength(AtNodeLib *node)
{
   node->methods     = LengthMethods;
   node->output_type = AI_TYPE_FLOAT;
   node->name        = SHADERLIB_PREFIX "length";
   node->node_type   = AI_NODE_SHADER;
}

