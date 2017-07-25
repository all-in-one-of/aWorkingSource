#include <ai.h>
#include <ai_math.h>
#include <algorithm>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <climits>

#include "lc_fuse_normal.h"

AI_SHADER_NODE_EXPORT_METHODS(LcFuseNormalMtd);

node_parameters
{
   AiParameterBool("enable", true);
   AiParameterFlt("radius", 0.01f);
   AiParameterFlt("min_blend_dist", 0.1f);
   AiParameterInt("nsamples", 4);
   AiParameterFlt("blend_bias", 0.f);
   AiParameterFlt("adv_offsetScale", 0.15f);
}

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
   data->nsamples = AiNodeGetInt(node, "nsamples");
   data->weight_curve = AiNodeGetFlt(node, "blend_bias")*0.5f+0.5f;   // map (-1,1) -> (0,1)
   data->adv_offsetScale = AiNodeGetFlt(node, "adv_offsetScale");

   if (std::abs(data->radius) < AI_EPSILON)
      data->radius = 0.f;

   //AtNode* options = AiUniverseGetOptions();
   //if (AiNodeGetInt(options, "GI_diffuse_depth")==0)
   //   AiMsgWarning("%s requires GI_diffuse_depth set to at least 1, now 0.", SHADER_NAMES[kShaderIdFuseShading]);

   if (std::abs(data->min_blend_dist) < AI_EPSILON)
      data->min_blend_dist = 0.f;

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
   SGCache SGC;
   SGC.initCache(sg);
   
   AtVector Nn = AiV3Normalize(sg->N),tangent = AiV3Normalize(sg->dPdu),bitangent = AiV3Normalize(sg->dPdv);

   float offset_dist = data->adv_offsetScale*data->radius;
   AtRay near_test_ray;
   AtShaderGlobals near_hitpoint;
   near_test_ray = AiMakeRay(AI_RAY_ALL_DIFFUSE, sg->P, &sg->N, offset_dist, sg);
   float in_radius = 0.f;

   // if too close, just blend.
   if (AiTraceProbe(near_test_ray, &near_hitpoint))
   {
      sg->Nf = sg->N = AiV3Normalize(AiV3Normalize(near_hitpoint.N)+Nn);
      sg->Nf = AiFaceViewer(sg);
      in_radius = 1.0;
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
      while(AiSamplerGetSample(samit, samples)) 
      {
         // ==== make the sample ray ==== 
         
         // try disk
         float u, v;
         concentricSampleDisk(samples[0], samples[1], u, v);

         AtVector o, dir = u*tangent + v*bitangent;
         dir = AiV3Normalize(dir);
         ray = AiMakeRay(AI_RAY_ALL_DIFFUSE , orig, &dir, trace_dist, sg);

         // ==== gather ==== 
         if (AiTraceProbe(ray, &hitpoint)) 
         {

            AtVector Nn_hitpoint = AiV3Normalize(hitpoint.N), L = hitpoint.P - orig;

            float cos_alpha = AiV3Dot(Nn, -Nn_hitpoint);
            float d;

            float d1 = AiV3Length(L);
            float sin_alpha = sqrt(1.0001f+cos_alpha*cos_alpha); // use 1.0001 instead of 1.0 to tolerant float error.
            float d_pt2edge = offset_dist / sin_alpha;
            float d2 = sqrt(d_pt2edge*d_pt2edge + offset_dist*offset_dist);
            d2 *= cos_alpha>0?1:-1;
            d = d1+d2;

            // beyound radius
            if (d>data->radius)
               continue;

            float weight = AiClamp((1.f - d/data->radius)*(1+WEIGHT_ADD), 0.f, 1.0f);
            weight = AiBias(weight, data->weight_curve);

            weight_total += weight;
            

            AiFaceForward(hitpoint.N, -hitpoint.Ng); // use Ng's sign, but N's direction;
            N_total += AiV3Normalize(hitpoint.N)*weight;

            n += 1;
         }
      } // gather samples

      if (n) 
      {
         sg->Nf = sg->N = AiV3Normalize(N_total/n+Nn);
         sg->Nf = AiFaceViewer(sg);
      }
   }

   AtVector result = sg->Nf;
   sg->out.VEC() = result;
   SGC.restoreSG(sg);
}

node_loader
{
   if (i > 0)
      return false;
   node->methods     = LcFuseNormalMtd;
   node->output_type = AI_TYPE_VECTOR;
   node->name        = "lc_fuse_normal";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;   
}
