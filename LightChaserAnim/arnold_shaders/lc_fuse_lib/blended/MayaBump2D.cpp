#include <ai.h>
#include <algorithm>

AI_SHADER_NODE_EXPORT_METHODS(MayaBump2DMtd);

static const char* useAsNames[] = {"bump", "tangent_normal", "object_normal", 0}; 

enum mayaBump2DModes{
   BM_BUMP = 0,
   BM_TANGENT_NORMAL,
   BM_OBJECT_NORMAL
};

node_parameters
{
   AiParameterFlt("bump_map", 0.f);
   AiParameterFlt("bump_height", 1.f);
   AiParameterRGB("normal_map", 0.f, 0.f, 1.f);
   AiParameterBool("flip_r", true);
   AiParameterBool("flip_g", true);
   AiParameterBool("swap_tangents", false);
   AiParameterBool("use_derivatives", true);
   AiParameterBool("gamma_correct", true);
   AiParameterEnum("use_as", 0, useAsNames)
   AiParameterRGBA("shader", 0.f, 0.f, 0.f, 1.f);
   AiMetaDataSetBool(mds, NULL, "maya.hide", true);
}

enum mayaBump2DParams {
   p_bump_map,
   p_bump_height,
   p_normal_map,
   p_flip_r,
   p_flip_g,
   p_swap_tangents,
   p_gamma_correct,
   p_use_derivatives,
   p_use_as,
   p_shader
};

struct mayaBump2DData{
   AtRGBA shaderValue;
   AtNode* bumpMap;
   AtNode* shader;
   float bumpMultiplier;
   float gammaCorrect;
   int bumpMode;
   bool isShaderRGBA;
   bool flipR, flipG, swapTangents, useDerivatives;
};

node_initialize
{
   mayaBump2DData* data = (mayaBump2DData*)AiMalloc(sizeof(mayaBump2DData));
   data->bumpMap = 0;
   data->shader = 0;
   data->bumpMode = 0;
   data->bumpMultiplier = 1.f;
   data->gammaCorrect = 1.f;
   data->flipR = true;
   data->flipG = true;
   data->swapTangents = false;
   data->useDerivatives = true;
   data->isShaderRGBA = false;   
   AiNodeSetLocalData(node, data);
}

node_update
{
   mayaBump2DData* data = (mayaBump2DData*)AiNodeGetLocalData(node);
   data->bumpMode = AiNodeGetInt(node, "use_as");
   data->bumpMap = AiNodeGetLink(node, "bump_map");
   data->shader = AiNodeGetLink(node, "shader");
   if (data->shader == 0)
      data->shaderValue = AiNodeGetRGBA(node, "shader");
   else
   {
      const AtNodeEntry* nentry = AiNodeGetNodeEntry(data->shader);
      if (AiNodeEntryGetOutputType(nentry) != AI_TYPE_RGBA)
         data->isShaderRGBA = false;
      else
         data->isShaderRGBA = true;
   }
   AtNode* options = AiUniverseGetOptions();
   data->bumpMultiplier = AiNodeGetFlt(options, "bump_multiplier");
   if (AiNodeGetBool(options, "ignore_bump"))
      data->bumpMap = 0;
   if (ABS(data->bumpMultiplier) < AI_EPSILON)
      data->bumpMap = 0;
   data->flipR = AiNodeGetBool(node, "flip_r");
   data->flipG = AiNodeGetBool(node, "flip_g");
   data->swapTangents = AiNodeGetBool(node, "swap_tangents");
   data->useDerivatives = AiNodeGetBool(node, "use_derivatives");
   data->gammaCorrect = 1.f;
   if (AiNodeGetBool(node, "gamma_correct"))
      data->gammaCorrect = AiNodeGetFlt(options, "texture_gamma");
}

node_finish
{
   
}

struct BumpEvalData{
   float bumpHeight;
   AtNode* bumpMap;
};

static float BumpFunction(AtShaderGlobals* sg, void* d)
{
   BumpEvalData* data = (BumpEvalData*)d;
   AiShaderEvaluate(data->bumpMap, sg);
   return (sg->out.FLT - .5f) * data->bumpHeight;
}

shader_evaluate
{
   mayaBump2DData* data = (mayaBump2DData*)AiNodeGetLocalData(node);
   if (data->shader == 0 || sg->Op == 0)
   {
      sg->out.RGBA = data->shaderValue;
      return;
   }
   
   if (sg->Rt & AI_RAY_DIFFUSE || sg->Rt & AI_RAY_SHADOW || (data->bumpMode == BM_BUMP && data->bumpMap == 0))
   {      
      AiShaderEvaluate(data->shader, sg);
      if (data->isShaderRGBA == false)
         sg->out.RGBA.a = 1.f;
      return;
   }
   
   AtVector oldN = sg->N;
   AtVector oldNf = sg->Nf;
   
   if (data->bumpMode == BM_BUMP) // do classic bump mapping
   {
      const float bumpHeight = AiShaderEvalParamFlt(p_bump_height) * data->bumpMultiplier;
      if (ABS(bumpHeight) > AI_EPSILON)
      {
         BumpEvalData evalData;
         evalData.bumpHeight = bumpHeight;
         evalData.bumpMap = data->bumpMap;
         sg->N = AiShaderGlobalsEvaluateBump(sg, BumpFunction, &evalData);
         AiFaceForward(&sg->N, oldN);
         sg->N = -sg->N;
         AiFaceViewer(sg, sg->Nf);         
      }
   }
   else if(data->bumpMode == BM_TANGENT_NORMAL) // tangent space normal mapping
   {
      AtRGB normalMap = AiShaderEvalParamRGB(p_normal_map);
      if (data->gammaCorrect != 1.f)
         AiColorGamma(&normalMap, data->gammaCorrect);
      normalMap = (normalMap - .5f) * 2.f;
      AtVector tangent = sg->dPdu;
      AtVector bitangent = sg->dPdv;
      if (!data->useDerivatives)
      {
         AiUDataGetVec("tangent", &tangent);
         AiUDataGetVec("bitangent", &bitangent);
      }
      tangent = AiV3Normalize(tangent);
      bitangent = AiV3Normalize(bitangent);
      if (data->flipR)
         normalMap.r *= -1.f;
      if (data->flipG)
         normalMap.g *= -1.f;
      if (data->swapTangents)
         std::swap(tangent, bitangent);
      sg->N = normalMap.r * tangent +
              normalMap.g * bitangent +
              normalMap.b * oldN;
      sg->N = AiV3Normalize(sg->N);
      if (!AiV3Exists(sg->N))
         sg->N = oldN;
      else
      {
         sg->Nf = sg->N;
         AiFaceViewer(sg, sg->Nf);
      }
   }
   else // object space normal mapping
   {
      AtRGB normalMap = AiShaderEvalParamRGB(p_normal_map);
      if (data->gammaCorrect != 1.f)
         AiColorGamma(&normalMap, data->gammaCorrect);
      AtVector normalMapV = {normalMap.r, normalMap.g, normalMap.b};
      AiM4VectorByMatrixMult(&sg->N, sg->M, &normalMapV);
      sg->Nf = sg->N;
      AiFaceViewer(sg, sg->Nf);
   }
   
   AiShaderEvaluate(data->shader, sg);
   if (data->isShaderRGBA == false)
      sg->out.RGBA.a = 1.f;
   
   sg->N = oldN;
   sg->Nf = oldNf;
}
