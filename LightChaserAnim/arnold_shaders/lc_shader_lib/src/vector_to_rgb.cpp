#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(VectorToRGBMethods);

enum VectorToRGBParams
{
   p_input,
   p_mode
};

#define VECTOR_TO_RGB_RAW        0
#define VECTOR_TO_RGB_NORMALIZED 1
#define VECTOR_TO_RGB_CANONICAL  2
static const char* enum_vector_to_rgb_mode[] = { "raw", "normalized", "canonical", NULL };

node_parameters
{
   AiParameterVec ("input", 1.0f, 1.0f, 1.0f);
   AiParameterEnum("mode",  VECTOR_TO_RGB_RAW, enum_vector_to_rgb_mode);
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
   case VECTOR_TO_RGB_RAW:
       AiColorCreate(sg->out.RGB, vec.x, vec.y, vec.z);
       break;

   case VECTOR_TO_RGB_NORMALIZED:
       vec = AiV3Normalize(vec);
       AiColorCreate(sg->out.RGB, vec.x, vec.y, vec.z);
       break;

   case VECTOR_TO_RGB_CANONICAL:
       vec = AiV3Normalize(vec);
       AiColorCreate(sg->out.RGB, vec.x, vec.y, vec.z);
       sg->out.RGB = (sg->out.RGB + AI_RGB_WHITE) * 0.5f;
       break;
   }
}


void registerVectorToRGB(AtNodeLib *node)
{
   node->methods     = VectorToRGBMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "vector_to_rgb";
   node->node_type   = AI_NODE_SHADER;
}

