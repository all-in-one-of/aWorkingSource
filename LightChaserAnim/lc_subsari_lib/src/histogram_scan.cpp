#include <algorithm>

#include <ai.h>

#include "utils.h"

using namespace std;

AI_SHADER_NODE_EXPORT_METHODS(HistogameScanMethods);

enum SimpleParams {
	p_input,
	p_position,
	p_contrast,
	p_invert,
	p_clamp 
};

node_parameters
{
	AiParameterRGBA("input", 0.7f, 0.7f, 0.7f,1.0f);
	AiParameterRGBA("position", 0.7f, 0.7f, 0.7f,1.0f);
	AiParameterRGBA("contrast", 0.7f, 0.7f, 0.7f,1.0f);
	AiParameterBool("invert", false);
	AiParameterBool("clamp", false);
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
	AtRGBA input = AiShaderEvalParamRGBA(p_input);
	AtRGBA position = AiShaderEvalParamRGBA(p_position);
	AtRGBA contrast = AiShaderEvalParamRGBA(p_contrast);
	bool invert_in = AiShaderEvalParamBool(p_invert);
	bool clamp_in = AiShaderEvalParamBool(p_clamp);
    float pos = kt::mix(position.r, 1.0f-position.r, float(invert_in));
    float pos_a = (max(pos, 0.5f) - 0.5f) * 2.0f;
    float pos_b = min( (pos * 2.0f), 1.0f);
    float pos_x = min( (contrast.r * 0.5f), 1.0f);
	pos_x = max(pos_x, 0.0f);

    float InBlack = kt::mix(pos_a, pos_b, pos_x);
    float InWhite = kt::mix(pos_b, pos_a, pos_x);

    float outWhite = 0.0f;
    float outBlack = 1.0f;

    float grad = (outWhite - outBlack) / (InWhite - InBlack);
    float inMid = 0.5f;
    inMid = kt::mix( InBlack, InWhite, inMid);

    float ratio = (inMid - InBlack) / (InWhite - InBlack);
    float alpha = 1.0f / pow(10.0f, 1.0f - 2.0f * ratio);

	AtVector levelled = {0.0f};
    for (int i = 0; i != 3; ++i)
    {
		float val = outBlack + (input[i] - InBlack) * grad;
		levelled[i] = pow( max(val, 0.0f), alpha);
    }
/*    if(clamp)
    {
		// Clamp to 0-1 range
		levelled.xyz = clamp( levelled.xyz, vec3(0.0), vec3(1.0));
    }*/
    AtVector MAX_V = {1.0f};
    AtVector MIN_V = {0.0f};
    AtVector result = kt::clamp(MAX_V - levelled, MIN_V, MAX_V);
	sg->out.RGB = kt::AtRGBtoAtVector(result);
	sg->out_opacity = input.a;
}

/*node_loader
{
   if (i > 0)
      return false;

   node->methods     = HistogameScanMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "HistogameScan";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}*/