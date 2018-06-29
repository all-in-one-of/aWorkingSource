#include <ai.h>

#include "MayaUtils.h"

AI_SHADER_NODE_EXPORT_METHODS(MayaSnowMtd);

namespace
{
   enum MayaSnowParams
   {
      p_snowColor = 0,
      p_surfaceColor,
      p_threshold,
      p_depthDecay,
      p_thickness,
      p_wrap,
      p_local,
      p_placementMatrix,
      MAYA_COLOR_BALANCE_ENUM
   };
}

node_parameters
{
   AtMatrix id = AiM4Identity();

   AiParameterRGB("snowColor", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("surfaceColor", 0.5f, 0.0f, 0.0f);
   AiParameterFlt("threshold", 0.5f);
   AiParameterFlt("depthDecay", 5.0f);
   AiParameterFlt("thickness", 1.0f);
   AiParameterBool("wrap", true);
   AiParameterBool("local", false);
   AiParameterMtx("placementMatrix", id);
   AddMayaColorBalanceParams(params, nentry);

   AiMetaDataSetStr(nentry, NULL, "maya.name", "snow");
   AiMetaDataSetInt(nentry, NULL, "maya.id", 0x5254534e);
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

      AtRGB c = Mix(surfaceColor, snowColor, amount);

      sg->out.RGBA() = AtRGBA(c);
      MayaColorBalance(sg, node, p_defaultColor, sg->out.RGBA());
   }
   else
   {
      MayaDefaultColor(sg, node, p_defaultColor, sg->out.RGBA());
   }
   if (usePref) RestorePoints(sg, tmpPts);
}
