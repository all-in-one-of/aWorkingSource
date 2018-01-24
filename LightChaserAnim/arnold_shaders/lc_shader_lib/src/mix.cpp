#include "shaderlib.h"

#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(MixMethods);

enum MixParams
{
   p_input1,
   p_input2,
   p_mix
};

node_parameters
{
   AiParameterRGB("input1", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("input2", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("mix",    0.5f);
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
   AtColor input1, input2;
   
   float mix = AiShaderEvalParamFlt(p_mix);
   if (mix <= 0.0f)
   {
      sg->out.RGB = AiShaderEvalParamRGB(p_input2);
   }
   else if (mix >= 1.0f)
   {
      sg->out.RGB = AiShaderEvalParamRGB(p_input1);
   }
   else
   {
      AtColor input1 = AiShaderEvalParamRGB(p_input1);
      AtColor input2 = AiShaderEvalParamRGB(p_input2);
      sg->out.RGB = AiColorLerp(mix, input1, input2);
   }
}


void registerMix(AtNodeLib *node)
{
   node->methods     = MixMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "mix";
   node->node_type   = AI_NODE_SHADER;
}

