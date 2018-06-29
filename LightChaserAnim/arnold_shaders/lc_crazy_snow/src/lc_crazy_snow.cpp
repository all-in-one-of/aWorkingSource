#include <ai.h>

#include "MayaUtils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcCrazySnowMtd);

enum SpaceEnum
{
   CS_PREF = 0,
   CS_WOLD,
   CS_LOCL,
};

static const char* spaceNames[] =
{
   "Pref",
   "Wrold",
   "Local",
   NULL
};

enum Params
{
   p_snowColor = 0,
   p_surfaceColor,
   p_threshold,
   p_depthDecay,
   p_thickness,
   p_wrap,
   p_coord_space,
   p_local,
   p_placementMatrix,
   p_translation,
   p_rotation,
   p_scale,
   p_defaultColor,     
   p_colorGain,        
   p_colorOffset,      
   p_alphaGain,        
   p_alphaOffset,      
   p_alphaIsLuminance, 
   p_invert,           
   p_exposure
};

node_parameters
{
   AtMatrix id = AiM4Identity();

   AiParameterRGB("snowColor", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("surfaceColor", 0.5f, 0.0f, 0.0f);
   AiParameterFlt("threshold", 0.5f);
   AiParameterFlt("depthDecay", 5.0f);
   AiParameterFlt("thickness", 1.0f);
   AiParameterBool("wrap", true);
   AiParameterEnum("coord_space", 0, spaceNames);
   AiParameterBool("local", false);
   AiParameterMtx("placementMatrix", id);
   AiParameterVec("translation", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("rotation", 0.0f);
   AiParameterVec("sclae", 1.0f, 1.0f, 1.0f);
   AiParameterRGB ("defaultColor", 0.5f, 0.5f, 0.5f);
   AiParameterRGB ("colorGain", 1.0f, 1.0f, 1.0f);
   AiParameterRGB ("colorOffset", 0.0f, 0.0f, 0.0f);
   AiParameterFlt ("alphaGain", 1.0f);
   AiParameterFlt ("alphaOffset", 0.0f);
   AiParameterBool("alphaIsLuminance", false);
   AiParameterBool("invert", false);
   AiParameterFlt ("exposure", 0.0f);
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
   AtMatrix *placementMatrix = AiShaderEvalParamMtx(p_placementMatrix);
   // AtMatrix TranslationMatrix = AiM4Translation(translation);
   // AtMatrix RotationMatrix = AiM4RotationY(rotation);
   // AtMatrix ScaleMatrix = AiM4Scaling(sclae);
   // AtMatrix TransformedMat = AiM4Mult(AiM4Mult(TranslationMatrix,RotationMatrix), ScaleMatrix);

   bool local = AiShaderEvalParamBool(p_local);
   bool wrap = AiShaderEvalParamBool(p_wrap);

   AtVector tmpPts;
   bool usePref = SetRefererencePoints(sg, tmpPts);

   AtVector P = AiM4PointByMatrixMult(*placementMatrix, (local ? sg->Po : sg->P));

   if (wrap || ((-1.0f <= P.x && P.x <= 1.0f) &&
                (-1.0f <= P.y && P.y <= 1.0f) &&
                (-1.0f <= P.z && P.z <= 1.0f)))
   {
      AtRGB snowColor = AiShaderEvalParamRGB(p_snowColor);
      AtRGB surfaceColor = AiShaderEvalParamRGB(p_surfaceColor);
      float threshold = AiShaderEvalParamFlt(p_threshold);
      float depthDecay = AiShaderEvalParamFlt(p_depthDecay);
      float thickness = AiShaderEvalParamFlt(p_thickness);
      AtVector N;

      AtVector U = AtVector(0.0f, 1.0f, 0.0f);

      if (local)
      {
         N = AiM4VectorByMatrixMult(sg->Minv, sg->N);
      }
      else
      {
         N = sg->N;
      }

      N = AiM4VectorByMatrixMult(*placementMatrix, N);
      N = AiV3Normalize(N);

      float NdU = AiV3Dot(N, U);

      float amount = 0.0f;

      if (NdU > threshold)
      {
         amount = (1.0f - expf(-(NdU - threshold) * depthDecay) ) * thickness;
      }

      AtRGB result = Mix(surfaceColor, snowColor, amount);

      sg->out.RGBA() = AtRGBA(result);
         
   }
   else
   {
      const AtRGB defaultColor   = AiShaderEvalParamRGB(p_defaultColor);  //p_defaultColor);
      sg->out.RGBA() = AtRGBA(defaultColor);
      sg->out.RGBA().a = 0.0f;

   }
   if (usePref) RestorePoints(sg, tmpPts);
}

node_loader
{
   if (i > 0)
      return false;

   node->methods     = LcCrazySnowMtd;
   node->output_type = AI_TYPE_RGBA;
   node->name        = "lc_crazy_snow";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}