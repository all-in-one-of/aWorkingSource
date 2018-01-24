#include <ai.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <math.h>

#include "texture_blend.h"

AI_SHADER_NODE_EXPORT_METHODS(TextureBlendXMethods);

enum TextureBlendParams {
	p_color = 0,
	p_block,
};


node_parameters
{
   AiParameterRGB("color", 0.7f, 0.7f, 0.7f);
   AiParameterPNT2("uvBlock", 1.0f, 1.0f);
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
   AtPoint2 block = AiShaderEvalParamPnt2(p_block);

   float inU = sg->u;
   float inV = sg->v;
   float outU,outV;
   AtRGB image;   

   image = AiShaderEvalParamRGB(p_color);

   AtRGB image_u;
   AtRGB image_v;
   AtPoint2 move = lerpOffset(block);
   float center;
   // image_u
   outU = inU + move.x;
   outV = inV;
   //center = floor(outU) + 0.5f;
   //outU = center - (outU - center);
   center = floor(outV) + 0.5f;
   outV = center - (outV - center);
   sg->u = outU;
   sg->v = outV;
   image_u = AiShaderEvalParamRGB(p_color);

   sg->u = inU;
   sg->v = inV;
   AtPoint2 iuv = {sg->u,sg->v};
   AtRGB red = AiColorCreate(1.0f,0.0f,0.0f);
   AtRGB blue = AiColorCreate(0.0f,0.0f,1.0f);
   AtPoint2 blend;
   float len = 1/block.x;
   AtPoint2 fuv;
   fuv.x = (iuv.x-floor(iuv.x/len)*len)/len;
   fuv.y = (iuv.y-floor(iuv.y/len)*len)/len;
   float p = 1/SAMPLE;
   // blend u
   if(fuv.x > (1 - p) && fuv.x <= 1)
   {
      //sg->out.RGB = red;
      blend.x = 1 - ((fuv.x - (1 - p))*SAMPLE);
   }
   else if(fuv.x > 0 && fuv.x <= p)
   {
      blend.x = fuv.x*SAMPLE;
   }
   else
   {
      blend.x = 1;
   }
   // blend v
   if(fuv.y > (1 - p) && fuv.y <= 1)
   {
      blend.y = 1 - ((fuv.y - (1 - p))*SAMPLE);
   }
   else if(fuv.y > 0 && fuv.y <= p)
   {
      blend.y = fuv.y*SAMPLE;
   }
   else
   {
      blend.y = 1;
   }


   sg->out.RGB = lerp(image_u, image, blend.x);
   //sg->out.RGB = lerp(blue,lerp(red, image, blend.x),blend.y);   
   //sg->out.RGB = lerp(image_blend,lerp(image_blend, image, blend.x),blend.y);
   //sg->out.RGB = lerp(image_v,lerp(image_u, image, blend.x),blend.y);
   //sg->out.RGB = origin_image;

}

/*node_loader
{
   if (i > 0)
      return false;

   node->methods     = TextureBlendMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "TextureBlend";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}*/