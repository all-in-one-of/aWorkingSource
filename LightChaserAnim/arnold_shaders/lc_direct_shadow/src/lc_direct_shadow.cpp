#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(LcDirectShadowMethods);

enum ModeEnum
{
   LC_SHADOW_MASK = 0,
   LC_LAMBERT,
};

static const char* ModeNames[] =
{
   "shadow_mask",
   "lambert",
   NULL
};



enum Params
{
      p_mode,
      p_color,
};

node_parameters
{
      AiParameterEnum("mode", 0, ModeNames);
      AiParameterRGB("color", 1.0f,1.0f,1.0f);
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

      // old
      AtRGB color = AiShaderEvalParamRGB(p_color);
      int mode = AiShaderEvalParamInt(p_mode);
      AiLightsPrepare(sg);
      AtRGB La = AI_RGB_BLACK; // initialize light accumulator to = 0
      AtLightSample ls;
      while (AiLightsGetSample(sg, ls)) // loop over the lights
      {
         float LdotN = AiV3Dot(ls.Ld, sg->N);
         if (LdotN < 0) LdotN = 0;
         // Lambertian diffuse
         La += ls.Li * (1/ls.pdf) * LdotN * color;
      }
      AtRGB diffuse_result = La;
      AtRGB shadow_result = AI_RGB_RED;
      float diff_t = AiClamp(diffuse_result.r, 0.0f, 1.0f);
      if(diff_t > 0.0f)
      {
         shadow_result = color;
      }
      else
      {
         shadow_result = AI_RGB_ZERO;
      }

      AtRGB result;
      switch(mode)
      {
         case LC_SHADOW_MASK:
            result = shadow_result;
            break;
         case LC_LAMBERT:
            result = diffuse_result;
            break;
         default:
            result = AI_RGB_RED;
            break;
      }
      sg->out.RGB() = result;

}


node_loader
{
   if (i > 0)
      return false;

   node->methods     = LcDirectShadowMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "lc_direct_shadow";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}