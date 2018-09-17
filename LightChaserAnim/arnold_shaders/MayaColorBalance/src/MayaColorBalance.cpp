#include <ai.h>
 
AI_SHADER_NODE_EXPORT_METHODS(MayaColorBalanceMethods);
 
enum Params { 
   p_input_color = 0,
   p_default_color,
   p_color_gain,
   p_color_offset,
   p_alpha_gain,
   p_alpha_offset,
   p_alpha_is_luminance,
   p_invert,
   p_exposure,
};
 
node_parameters
{
   AiParameterRGBA("input_color", 1.0f, 1.0f, 1.0f, 1.0f);
   AiParameterRGBA("default_color", 0.5f, 0.5f, 0.5f, 1.0f);
   AiParameterRGB("color_gain", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("color_offset", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("alpha_gain", 1.0f);
   AiParameterFlt("alpha_offset", 0.0f);
   AiParameterBool("alpha_is_luminance", false);
   AiParameterBool("invert", false);
   AiParameterFlt("exposure", 0.0f);
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
   AtRGBA input = AiShaderEvalParamRGBA(p_input_color);
   AtRGB color = AtRGB(input.r,input.g,input.b);
   float alpha = input.a;

   // AtRGBA default_color = AiShaderEvalParamRGBA(p_default_color);

   const AtRGB colorGain      = AiShaderEvalParamRGB(p_color_gain);  // p_colorGain);
   const AtRGB colorOffset    = AiShaderEvalParamRGB(p_color_offset);  // p_colorOffset);
   const float alphaGain      = AiShaderEvalParamFlt(p_alpha_gain);  // p_alphaGain);
   const float alphaOffset    = AiShaderEvalParamFlt(p_alpha_offset);  // p_alphaOffset);
   const bool alphaIsLuminance     = AiShaderEvalParamBool(p_alpha_is_luminance);  // alphaIsLuminance);
   const bool invert = AiShaderEvalParamBool(p_invert); // p_invert);
   const float exposure = powf(2.0f, AiShaderEvalParamFlt(p_exposure)); // p_exposure

   if (invert)
   {
      color = 1.0f - color;
   }

   if (alphaIsLuminance)
   {
      alpha = (0.3f * color.r + 0.59f * color.g + 0.11f * color.b);
   }

   color = color*colorGain*exposure + colorOffset;
   alpha = alpha*alphaGain*alphaGain + alphaOffset;

   sg->out.RGBA() = AtRGBA(color,alpha);
}

node_loader
{
   if (i > 0)
      return false;
   node->methods     = MayaColorBalanceMethods;
   node->output_type = AI_TYPE_RGBA;
   node->name        = "MayaColorBalance";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}