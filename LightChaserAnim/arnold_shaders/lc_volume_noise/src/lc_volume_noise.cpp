#include <ai.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "MayaUtils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcVloumeNoiseMethods);

namespace
{

enum MayaVolumeNoiseParams
{
   p_threshold,
   p_amplitude,
   p_ratio,
   p_frequencyRatio,
   p_depthMax,
   p_inflection,
   p_time,
   p_frequency,
   p_scale,
   p_origin,
   p_implode,
   p_implodeCenter,
   p_noiseType,
   p_density,
   p_spottyness,
   p_sizeRand,
   p_randomness,
   p_falloff,
   p_numWaves,
   p_placementMatrix,
   p_wrap,
   p_local,
   MAYA_COLOR_BALANCE_ENUM
};

enum NoiseType
{
   NT_PERLIN = 0,
   NT_BILLOW,
   NT_VOLUME_WAVE,
   NT_WISPY,
   NT_SPACETIME
};

enum FalloffType
{
   FT_LINEAR = 0,
   FT_SMOOTH,
   FT_FAST,
   FT_BUBBLE
};

static const char* NoiseTypeNames[] =
{
   "Perlin",
   "Billow",
   "VolumeWave",
   "Wispy",
   "SpaceTime",
   0
};

static const char* FalloffTypeNames[] =
{
   "Linear",
   "Smooth",
   "Fast",
   "Bubble",
   0
};

};

node_parameters
{
   AtMatrix id = AiM4Identity();

   AiParameterFlt("threshold", 0.0f);
   AiParameterFlt("amplitude", 1.0f);
   AiParameterFlt("ratio", 0.707f);
   AiParameterFlt("frequencyRatio", 2.0f);
   AiParameterInt("depthMax", 3);
   AiParameterBool("inflection", false);
   AiParameterFlt("time", 0.0f);
   AiParameterFlt("frequency", 8.0f);
   AiParameterVec("scale", 1.0f, 1.0f, 1.0f);
   AiParameterVec("origin", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("implode", 0.0f);
   AiParameterVec("implodeCenter", 0.0f, 0.0f, 0.0f);
   AiParameterEnum("noiseType", 1, NoiseTypeNames);
   AiParameterFlt("density", 1.0f); // for: billow
   AiParameterFlt("spottyness", 0.1f); // for: billow
   AiParameterFlt("sizeRand", 0.0f); // for: billow
   AiParameterFlt("randomness", 1.0f); // for: billow
   AiParameterEnum("falloff", 2, FalloffTypeNames); // for: billow
   AiParameterInt("numWaves", 5); // for: VolumeWave
   AiParameterMtx("placementMatrix", id);
   AiParameterBool("wrap", true);
   AiParameterBool("local", false);
   AddMayaColorBalanceParams(params, nentry);
   
   AiMetaDataSetStr(nentry, NULL, "maya.name", "volumeNoise");
   AiMetaDataSetInt(nentry, NULL, "maya.id", 0x52545633);
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
   float threshold = AiShaderEvalParamFlt(p_threshold);
   float amplitude = AiShaderEvalParamFlt(p_amplitude);
   float ratio = AiShaderEvalParamFlt(p_ratio);
   float frequencyRatio = AiShaderEvalParamFlt(p_frequencyRatio);
   int depthMax = AiShaderEvalParamInt(p_depthMax);
   bool inflection = AiShaderEvalParamBool(p_inflection);
   float time = AiShaderEvalParamFlt(p_time);
   float frequency = AiShaderEvalParamFlt(p_frequency);
   AtVector origin = AiShaderEvalParamVec(p_origin);
   AtVector scale = AiShaderEvalParamVec(p_scale);
   float implode = AiShaderEvalParamFlt(p_implode);
   AtVector implodeCenter = AiShaderEvalParamVec(p_implodeCenter);
   int noiseType = AiShaderEvalParamInt(p_noiseType);
   float density = AiShaderEvalParamFlt(p_density);
   float spottyness = AiShaderEvalParamFlt(p_spottyness);
   float sizeRand = AiShaderEvalParamFlt(p_sizeRand);
   float randomness = AiShaderEvalParamFlt(p_randomness);
   int falloff = AiShaderEvalParamInt(p_falloff);
   int numWaves = AiShaderEvalParamInt(p_numWaves);
   AtMatrix *placementMatrix = AiShaderEvalParamMtx(p_placementMatrix);
   bool wrap = AiShaderEvalParamBool(p_wrap);
   bool local = AiShaderEvalParamBool(p_local);

   AtVector tmpPts;
   bool usePref = SetRefererencePoints(sg, tmpPts);

   AtVector P = AiM4PointByMatrixMult(*placementMatrix, (local ? sg->Po : sg->P));

   if (wrap || ((-1.0f <= P.x && P.x <= 1.0f) &&
                (-1.0f <= P.y && P.y <= 1.0f) &&
                (-1.0f <= P.z && P.z <= 1.0f)))
   {
      // should we transform implodeCenter and origin?
      // is this the right order?
      P -= origin;
      P /= scale;

      if (implode)
      {
         AtVector dp = P - implodeCenter;
         dp = dp / pow(AiV3Length(dp), implode);
         P = implodeCenter + dp;
      }

      int depth[2] = {0, depthMax};
      float ripples[3] = {1.0f, 1.0f, 1.0f};

      P *= frequency;

      float noiseVal = 0.0f;

      switch (noiseType)
      {
      case NT_PERLIN:
         if (inflection)
         {
            noiseVal = amplitude * fTurbulence(sg, P, time, 1.0f, frequencyRatio, depth, ratio, ripples);
         }
         else
         {
            noiseVal = fBm(sg, P, time, amplitude, depth, 1.0f, frequencyRatio, ratio);
         }
         break;
      case NT_BILLOW:
         {
            float radius = static_cast<float>(sqrt(0.5f * density));
            
            float pixelSize = float(AI_EPSILON);
            float nyquist = 2.0f * pixelSize;
            float pixel = 1.0f;
            int depthTop = 0;
            if ((frequencyRatio - AI_EPSILON) > 1.0f)
            {
               while (pixel > nyquist)
               {
                  pixel /= frequencyRatio;
                  depthTop++;
               }
            }
            else
            {
               pixel = nyquist;
               depthTop = depthMax;
            }
            
            depthMax = depthMax > depthTop ? depthTop : depthMax;
            
            noiseVal = BillowNoise(P, time, 3, radius, sizeRand, randomness, falloff, spottyness, depthMax, frequencyRatio, ratio, amplitude);
         }
         break;
      case NT_VOLUME_WAVE:
         {
            float amp = amplitude;
            float timeRatio = 1.0f / frequencyRatio;
            float twopi = 2.0f * static_cast<float>(AI_PI);

            noiseVal = 0.0f;
            time *= twopi;

            for (int i=0; i<depthMax; ++i)
            {
               float waveVal = 0.0f;

               for (int j=0; j<numWaves; ++j)
               {
                  float tmp = twopi * (0.5f * (1 + i) * (1 + j));

                  AtVector v, d;

                  v = AtVector(tmp, 0, 0);
                  d.x = AiPerlin3(v);
                  
                  v = AtVector(0, tmp, 0);
                  d.y = AiPerlin3(v);
                  
                  v = AtVector(0, 0, tmp);
                  d.z = AiPerlin3(v);

                  d = AiV3Normalize(d);

                  waveVal += (float) cos(twopi * AiV3Dot(P, d) + time);
               }

               if(isnan(waveVal))
                  break;

               waveVal /= numWaves;

               if (inflection)
               {
                  waveVal = fabs(waveVal);
               }

               noiseVal += amp * waveVal;

               amp *= ratio;
               P *= frequencyRatio;
               time *= timeRatio;
            }

            if (!inflection)
            {
               noiseVal = 0.5f * noiseVal + 0.5f;
            }
         }
         break;
      case NT_WISPY:
         P += AiPerlin3(P / 2.0f) * 1.3f;
         if (inflection)
         {
            noiseVal = amplitude * fTurbulence(sg, P, time, 1.0f, frequencyRatio, depth, ratio, ripples);
         }
         else
         {
            noiseVal = fBm(sg, P, time, amplitude, depth, 1.0f, frequencyRatio, ratio);
         }
         break;
      case NT_SPACETIME:
         if (inflection)
         {
            noiseVal = amplitude * fTurbulence(sg, P, time, 1.0f, frequencyRatio, depth, ratio, ripples);
         }
         else
         {
            noiseVal = fBm(sg, P, time, amplitude, depth, 1.0f, frequencyRatio, ratio);
         }
         break;
      }

      noiseVal = AiClamp(noiseVal+threshold, 0.0f, 1.0f);

      sg->out.RGBA() = AtRGBA(noiseVal, noiseVal, noiseVal, 1.0f);
      MayaColorBalance(sg, node, p_defaultColor, sg->out.RGBA());
   }
   else
   {
      MayaDefaultColor(sg, node, p_defaultColor, sg->out.RGBA());
   }
   if (usePref) RestorePoints(sg, tmpPts);
}

node_loader
{
   if (i > 0)
      return false;

   node->methods     = LcVloumeNoiseMethods;
   node->output_type = AI_TYPE_RGBA;
   node->name        = "lc_volume_noise";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}