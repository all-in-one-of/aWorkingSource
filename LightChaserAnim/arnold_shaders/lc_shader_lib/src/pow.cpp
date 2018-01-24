#include "shaderlib.h"

#include <ai.h>

#include <cmath>


AI_SHADER_NODE_EXPORT_METHODS(PowMethods);

enum PowParams
{
   p_base,
   p_exponent
};

node_parameters
{
   AiParameterRGB("base",     AI_E, AI_E, AI_E);
   AiParameterRGB("exponent", 0.0f, 0.0f, 0.0f);
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
   AtColor base = AiShaderEvalParamRGB(p_base);
   AtColor exponent = AiShaderEvalParamRGB(p_exponent);
   AiColorCreate(sg->out.RGB, powf(base.r, exponent.r), powf(base.g, exponent.g), powf(base.b, exponent.b));
}


void registerPow(AtNodeLib *node)
{
   node->methods     = PowMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "pow";
   node->node_type   = AI_NODE_SHADER;
}

