#include <ai.h>
#include <ai_math.h>
#include <algorithm>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <climits>

#include "lc_fuse_normal.h"

AI_SHADER_NODE_EXPORT_METHODS(LcFuseNormalMtd);


node_initialize
{
   ShaderData* data = new ShaderData();
   data->sampler = NULL;
   AiNodeSetLocalData(node, data);
}


node_update
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
   data->enable = AiNodeGetBool(node, "enable");
   data->radius = AiNodeGetFlt(node, "radius");
   data->min_blend_dist = AiNodeGetFlt(node, "min_blend_dist");
   data->blend_layer = AiNodeGetBool(node, "blend_layer");
   data->nsamples = AiNodeGetInt(node, "nsamples");
   data->weight_curve = AiNodeGetFlt(node, "blend_bias")*0.5f+0.5f;   // map (-1,1) -> (0,1)
   data->mode = AiNodeGetInt(node, "render_mode");
   data->adv_offsetScale = AiNodeGetFlt(node, "adv_offsetScale");

   if (std::abs(data->radius) < AI_EPSILON)
      data->radius = 0.f;

   //AtNode* options = AiUniverseGetOptions();
   //if (AiNodeGetInt(options, "GI_diffuse_depth")==0)
   //   AiMsgWarning("%s requires GI_diffuse_depth set to at least 1, now 0.", SHADER_NAMES[kShaderIdFuseShading]);

   if (std::abs(data->min_blend_dist) < AI_EPSILON)
      data->min_blend_dist = 0.f;

   data->shader = AiNodeGetLink(node, "shader");
   if (data->shader == NULL) 
   {
      data->shader_value = AtRGBA(1,0,0,1);
   } 
   else 
   {
      const AtNodeEntry* nent = AiNodeGetNodeEntry(data->shader);
      data->shader_is_rgba = AiNodeEntryGetOutputType(nent)==AI_TYPE_RGBA;
      data->shader_is_closure = AiNodeEntryGetOutputType(nent)==AI_TYPE_CLOSURE;
   }

   if (data->sampler) 
   {
      AiSamplerDestroy(data->sampler);
   }
   static const uint32_t seed =static_cast<uint32_t>(AiNodeEntryGetNameAtString(AiNodeGetNodeEntry(node)).hash());
   data->sampler = AiSampler(seed,data->nsamples, 2);
}


node_finish
{
   if (AiNodeGetLocalData(node))
   {
      ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
      AiSamplerDestroy(data->sampler);

      delete data;
      AiNodeSetLocalData(node, NULL);
   }
}


shader_evaluate
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

   if (data->shader == NULL)
   {
      if (data->mode != kVisRadius)
      {
         sg->out.RGBA() = data->shader_value;
         return;
      }
      else
      {
         if (!(sg->Rt & AI_RAY_CAMERA)) 
         {
            sg->out.RGBA() = AI_RGBA_ZERO;
            return;
         }
      }
   }
   if (data->enable == false || data->radius == 0.0f || !(sg->Rt & AI_RAY_CAMERA))
   {
      AiShaderEvaluate(data->shader, sg);
      if (data->shader_is_rgba == false)
         sg->out.RGBA().a = 1.f;
      return;
   }

   AtVector oldN = sg->N;
   AtVector oldNf = sg->Nf;
   int blend_layer = getBlendLayer(sg);
   bool respect_layer = data->blend_layer;
   
   AtVector Pref;
   if (!AiUserGetVecFunc(AtString("Pref"), sg,Pref))
   {
      data->min_blend_dist = 0.f;
      AiMsgWarning("[%s] %s has no Rest data (%s).", "ShaderIdFuseNormal", AiNodeGetName(sg->Op), "Pref");
   }

   AtVector Nn = AiV3Normalize(sg->N),
            tangent = AiV3Normalize(sg->dPdu),
            bitangent = AiV3Normalize(sg->dPdv);

   float offset_dist = data->adv_offsetScale*data->radius;
   AtRay near_test_ray;
   AtShaderGlobals near_hitpoint;
   near_test_ray = AiMakeRay(AI_RAY_ALL_DIFFUSE, sg->P, &sg->N, offset_dist, sg);
   float in_radius = 0.f;

   // if too close, just blend.
   if (AiTraceProbe(near_test_ray, &near_hitpoint))
   {
      //sg->Nf = sg->N = AiV3Normalize(AiV3Normalize(near_hitpoint.N)+Nn);
      //sg->Nf = AiFaceViewer(sg);
      //in_radius = 1.0;
      sg->out.RGB() = AtRGB(1,0,0);
      sg->out.RGBA().a = 1.0;  

   }
   else 
   {
      AtRay ray;
      float samples[2];
      AtSamplerIterator *samit = AiSamplerIterator(data->sampler, sg);

      AtShaderGlobals hitpoint;
      AtVector N_total = AtVector(0.f, 0.f, 0.f);
      AtVector orig = sg->P + Nn*offset_dist;
      int n = 0;
      float trace_dist = data->radius*4.f;
      float weight_total = 0.f;
         
      // try disk
      float u, v;
      concentricSampleDisk(samples[0], samples[1], u, v);

      AtVector o, dir = u*tangent + v*bitangent;
      dir = AiV3Normalize(dir);
      ray = AiMakeRay(AI_RAY_ALL_DIFFUSE , orig, &dir, trace_dist, sg);

      // ==== gather ==== 
      if (AiTraceProbe(ray, &hitpoint)) 
      {
         sg->out.RGB() = AtRGB(1,1,0);
         sg->out.RGBA().a = 1.0;  
      }
      else
      {
         sg->out.RGB() = AtRGB(1,0,1);
         sg->out.RGBA().a = 1.0;           
      }

   }

   if (data->mode == kVisRadius)
   {
      if(in_radius==0.f)
      {
         sg->out.RGB() = AtRGB(1,0,0);
         sg->out.RGBA().a = 1.0;         
      }
      else
      {
         sg->out.RGB() = AtRGB(1,1,0);
         sg->out.RGBA().a = 1.0;                  
      }
   } 

}

node_loader
{
   if (i > 0)
      return false;
   node->methods     = LcFuseNormalMtd;
   node->output_type = AI_TYPE_RGBA;
   node->name        = "lc_fuse_normal";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;   
}
