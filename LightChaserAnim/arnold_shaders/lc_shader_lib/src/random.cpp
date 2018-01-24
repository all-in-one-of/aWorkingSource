#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(RandomMethods);

enum RandomParams
{
   p_seed
};

/// from OpenEXR/ImathRandom.h
float rand32i(unsigned long int seed)
{
   unsigned long int state;
   state = (seed * 0xa5a573a5L) ^ 0x5a5a5a5aL;
   state = 1664525L * state + 1013904223L;

   union
   {
      float f;
      unsigned int i;
   } u;

   u.i = 0x3f800000 | (state & 0x7fffff);
   return u.f - 1;
}

float rand32f(float seed)
{
   union
   {
      float f;
      unsigned int i;
   } u;

   u.f = seed;
   return rand32i(u.i);
}


node_parameters
{
   AiParameterRGB("seed", 0.0f, 0.0f, 0.0f);
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
   AtRGB seed = AiShaderEvalParamRGB(p_seed);

   for (int i = 0; i < 3; ++i)
      sg->out.RGB[i] = rand32f(seed[i]);
}


void registerRandom(AtNodeLib *node)
{
   node->methods = RandomMethods;
   node->output_type = AI_TYPE_RGB;
   node->name = SHADERLIB_PREFIX "random";
   node->node_type = AI_NODE_SHADER;
}

