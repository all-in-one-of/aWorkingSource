#include <ai.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <math.h>


#include "texture_repetition.h"

AI_SHADER_NODE_EXPORT_METHODS(TextureRepetitionMethods);

enum TextureRepetitionParams { 
   p_color = 0,
   p_repeat,
   p_rotate,
   p_offset,
   p_noise,
   //p_mirror_u,
   //p_mirror_v,

};

node_parameters
{
   AiParameterRGB("color", 0.7f, 0.7f, 0.7f);
   AiParameterPNT2("repeatUV", 1.0f, 1.0f);
   AiParameterFLT("rotateUV", 15.0f);
   AiParameterPNT2("offsetUV", 0.0f, 0.0f);
   AiParameterPNT2("noiseUV", 0.00f, 0.00f);
   //AiParameterBOOL("mirrorU", true);
   //AiParameterBOOL("mirrorV", true);

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
   //sg->out.RGB = AiShaderEvalParamRGB(p_color);
   AtPoint2 repeat = AiShaderEvalParamPnt2(p_repeat);
   float rotate = AiShaderEvalParamFlt(p_rotate);
   AtPoint2 offset = AiShaderEvalParamPnt2(p_offset);
   AtPoint2 noise = AiShaderEvalParamPnt2(p_noise);
   //bool mirrorU = (AiShaderEvalParamBool(p_mirror_u) == true);
   //bool mirrorV = (AiShaderEvalParamBool(p_mirror_v) == true);
   
   //AiStateGetMsgPnt2("repeat_uv_value",&repeat);

   float inU = sg->u;
   float inV = sg->v;
   float inDuDx = sg->dudx;
   float inDuDy = sg->dudy;
   float inDvDx = sg->dvdx;
   float inDvDy = sg->dvdy;

   float outU = inU;
   float outV = inV;
   float outDuDx = inDuDx;
   float outDuDy = inDuDy;
   float outDvDx = inDvDx;
   float outDvDy = inDvDy;


   AtFloatUV inUV = {inU, inV};

   AtPoint2 thisBlock = hashBlock(inUV, repeat);

   // noise uv
   if (noise.x > 0.0f)
   {
      AtVector2 uv = {inU * 16, inV * 16};
      outU += noise.x * AiPerlin2(uv);
   }

   if (noise.y > 0.0f)
   {
      AtVector2 uv = {(1 - inU) * 16, (1 - inV) * 16};
      outV += noise.y * AiPerlin2(uv);
   }



   // apply repetition factor
   outU *= repeat.x;
   outV *= repeat.y;
   

   // for UVs, translate first, then rotate
   if(offset.x > 0 || offset.y > 0)
   {
      float offsetX = sin(AI_PI*(thisBlock.x+offset.x));
      float offsetY = cos(AI_PI*(thisBlock.y+offset.y));
      outU += offsetX;
      outV += offsetY;
   }


   int mirrorE = (int)thisBlock.x % 2;
   int mirrorO = (int)thisBlock.y % 2;

   // do mirror, stagger before rotation
   if (mirrorE == 0)
   {

      float center = floor(outV) + 0.5f;
      outV = center - (outV - center);

      //outDuDy = -outDuDy;
      //outDvDy = -outDvDy;
   }
   
   if (mirrorO == 0)
   {
      float center = floor(outU) + 0.5f;
      outU = center - (outU - center);

      //outDuDx = -outDuDx;
      //outDvDx = -outDvDx;
   }


   // finally rotate UV
   if (rotate <= -AI_EPSILON || rotate >= AI_EPSILON)
   {
      float x, y;
      float ca = cos(rotate);
      float sa = sin(rotate);

      x = outU - 0.5f;
      y = outV - 0.5f;
      outU = 0.5f + ca * x - sa * y;
      outV = 0.5f + ca * y + sa * x;

      x = outDuDx;
      y = outDuDy;
      outDuDx = ca * x - sa * y;
      outDuDy = ca * y + sa * x;

      x = outDvDx;
      y = outDvDy;
      outDvDx = ca * x - sa * y;
      outDvDy = ca * y + sa * x;
   }
   
   // replace shader globals
   sg->u = outU;
   sg->v = outV;
   sg->dudx = outDuDx;
   sg->dudy = outDuDy;
   sg->dvdx = outDvDx;
   sg->dvdy = outDvDy;

   sg->out.RGB = AiShaderEvalParamRGB(p_color);
   // recover sg uv value
   sg->u = inU;
   sg->v = inV;
   sg->dudx = inDuDx;
   sg->dudy = inDuDy;
   sg->dvdx = inDvDx;
   sg->dvdy = inDvDy;

}

/*node_loader
{
   if (i > 0)
      return false;

   node->methods     = TextureRepetitionMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "TextureRepetition";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}*/