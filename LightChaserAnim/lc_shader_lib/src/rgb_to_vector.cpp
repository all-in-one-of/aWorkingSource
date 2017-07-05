#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(RGBToVectorMethods);

enum RGBToVectorParams
{
   p_input,
   p_mode
};

#define RGB_TO_VECTOR_RAW        0
#define RGB_TO_VECTOR_CANONICAL  1
static const char* enum_rgb_to_vector_mode[] = { "raw", "canonical", NULL };

node_parameters
{
   AiParameterVec ("input", 1.0f, 1.0f, 1.0f);
   AiParameterEnum("mode",  RGB_TO_VECTOR_RAW, enum_rgb_to_vector_mode);
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
   AtColor color = AiShaderEvalParamRGB(p_input);
   int mode = AiShaderEvalParamInt(p_mode);

   if (mode == RGB_TO_VECTOR_CANONICAL)
   {
      color *= 2.0f;
      color -= AI_RGB_WHITE;
   }

   sg->out.VEC.x = color.r;
   sg->out.VEC.y = color.g;
   sg->out.VEC.z = color.b;
}


void registerRGBToVector(AtNodeLib *node)
{
   node->methods     = RGBToVectorMethods;
   node->output_type = AI_TYPE_VECTOR;
   node->name        = SHADERLIB_PREFIX "rgb_to_vector";
   node->node_type   = AI_NODE_SHADER;
}

