#include <ai.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "MayaUtils.h"

AI_SHADER_NODE_EXPORT_METHODS(TextureRepetition);

enum TextureRepetitionParams { 
   p_color = 0,
   p_repeat,
   p_offset,
   p_rotate,
};

node_parameters
{
   AiParameterRGB("color", 0.7f, 0.7f, 0.7f);
   AiParameterPNT2("repeatUV", 1.0f, 1.0f);
   AiParameterFLT("rotateUV", 0.0f);

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
   AtPoint2 repeat = AiShaderEvalParamPnt2(p_repeat);
   float rotate = AiShaderEvalParamFlt(p_rotate);
   
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



   //outDuDx *= repeat.x;
   //outDuDy *= repeat.x;
   //outDvDx *= repeat.y;
   //outDvDy *= repeat.y;

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

      //x = outDuDx;
      //y = outDuDy;
      //outDuDx = ca * x - sa * y;
      //outDuDy = ca * y + sa * x;

      //x = outDvDx;
      //y = outDvDy;
      //outDvDx = ca * x - sa * y;
      //outDvDy = ca * y + sa * x;         

   }

   outU *= repeat.x;
   outV *= repeat.y;


   // replace shader globals
   sg->u = outU;
   sg->v = outV;
   //sg->dudx = outDuDx;
   //sg->dudy = outDuDy;
   //sg->dvdx = outDvDx;
   //sg->dvdy = outDvDy;

   sg->out.RGB = AiShaderEvalParamRGB(p_color);
}

node_loader
{
   if (i > 0)
      return false;

   node->methods     = TextureRepetition;
   node->output_type = AI_TYPE_RGB;
   node->name        = "TextureRepetition";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}