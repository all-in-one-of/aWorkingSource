#include <ai.h>
#include <cmath>

AI_SHADER_NODE_EXPORT_METHODS(LcHoudiniGlowMethods);


enum Params
{
    p_glowColor,
    p_exp,
    p_invert,
    p_emitLight,
    p_emisson,
    p_usePointColor,
    p_usePointAlpha,
    p_cd,
    p_alpha,
    p_ce,
};

node_parameters
{
      AiParameterRGB("glowColor", 1.0f,1.0f,1.0f);
      AiParameterFlt("exp",4.0f);
      AiParameterBool("invert",false);
      AiParameterBool("emitLight",false);
      AiParameterFlt("emisson",1.0f);
      AiParameterBool("usePointColor",true);
      AiParameterBool("usePointAlpha",true);
      AiParameterRGB("cd", 1.0f,1.0f,1.0f);
      AiParameterFlt("alpha",1.0f);
      AiParameterRGB("ce", 0.0f,0.0f,0.0f);
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
      AtRGB glowColor = AiShaderEvalParamRGB(p_glowColor);
      float exp = AiShaderEvalParamFlt(p_exp);
      bool invert = AiShaderEvalParamBool(p_invert);
      bool emitLight = AiShaderEvalParamBool(p_emitLight);
      float emisson = AiShaderEvalParamFlt(p_emisson);
      bool usePointColor = AiShaderEvalParamBool(p_usePointColor);
      bool usePointAlpha = AiShaderEvalParamBool(p_usePointAlpha);
      AtRGB cd = AiShaderEvalParamRGB(p_cd);
      float alpha = AiShaderEvalParamFlt(p_alpha);
      // AtRGB ce = AiShaderEvalParamRGB(p_ce); // not be used in this time



        AtRGB result_color = AI_RGB_WHITE;
        
        AtRGB diffuse_color = AI_RGB_WHITE;
        if (usePointColor)
        {
            diffuse_color = cd;
        }
        
        // caculate normal falloff,catually is N dot Eye
        AtVector incident_ray = sg->Ro - sg->P;
        AtVector shading_normal = sg->N;
        AiFaceForward(shading_normal,incident_ray);
        float falloff =  AiClamp(AiV3Dot(shading_normal,-incident_ray), 0.0f, 1.0f);
        if (invert) 
            falloff = 1-falloff;
        falloff = powf(falloff, exp);



         //float result1 = (0 != 0 ? (usePointAlpha != 0 ? 1 : alpha): (usePointAlpha != 0 ? alpha : 1));

        // choose alpha
        float result_alpha = 1.0;
        if (usePointAlpha)
            result_alpha = alpha;

        float falloff_multi_alpha = result_alpha * falloff ;

        AtRGB glow_color = diffuse_color * glowColor * falloff_multi_alpha;
        
        AtRGB emisson_color = glow_color * emisson;

         AtRGB opacity = AtRGB(falloff_multi_alpha);
         AtRGB result_opacity = AI_RGB_ZERO;

        if (emitLight)
        {
            result_color = emisson_color;
            result_opacity = opacity;         
        }
        else
        {
            result_color = glow_color;
            result_opacity = AI_RGB_ZERO;         
        }

      result_opacity = opacity;


      // AtRay ray;
      // AtShaderGlobals near_hitpoint;
      // AtVector origin = sg->Ro;
      // AtVector dir = sg->Ro -sg->P;
      // ray = AiMakeRay(AI_RAY_CAMERA, origin, &dir, AI_BIG, sg);
      
      // AtRGB weight;
      // AtRGB result_weight = AI_RGB_ZERO;
      // AtScrSample result;
      // static const uint32_t seed = static_cast<uint32_t>(AiNodeEntryGetNameAtString(AiNodeGetNodeEntry(node)).hash());
      // AtSampler* sampler = AiSampler(seed, 6, 2);
      // AtSamplerIterator* sampit = AiSamplerIterator(sampler, sg);
      // float samples = 6;
      // int tag = 0;
      // while (AiSamplerGetSample(sampit, &samples))
      // {
      //    bool ishit = AiTrace(ray,weight,result);
         
      //    if (ishit)
      //    {            
      //       result_weight += AtRGB(result.z);
      //       tag++;
      //    }
      // }
      // AtRGB test;
      // if(tag==0)
      //    test = AI_RGB_WHITE;
      // else
      //    test = AI_RGB_RED;

      // new, opacity must be premultiplied into other closures
      AtRGB fake_opacity = AtRGB(alpha);

      AtClosureList closures;
      closures.add(AiClosureEmission(sg,  result_color));
      if (AiMax(result_opacity.r, result_opacity.b, result_opacity.b) > AI_EPSILON)
      {
         closures *= 1- fake_opacity;
         closures.add(AiClosureTransparent(sg, fake_opacity));
      }
      closures.add(AiClosureMatte(sg, AI_RGB_WHITE - result_alpha));
      sg->out.CLOSURE() = closures;

}


node_loader
{
      if (i > 0)
         return false;

      node->methods     = LcHoudiniGlowMethods;
      node->output_type = AI_TYPE_CLOSURE;
      node->name        = "lc_houdini_glow";
      node->node_type   = AI_NODE_SHADER;
      strcpy(node->version, AI_VERSION);
      return true;
}