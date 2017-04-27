#include <ai.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <math.h>


#include "texture_repetition.h"

AI_SHADER_NODE_EXPORT_METHODS(TextureRepetitionMethods);

enum TextureRepetitionParams { 
   p_texture,
   p_uvset,
};

void TextureFileOperation(AtPoint2 inUV,AtPoint2 inDu,AtPoint2 inDv,AtPoint2 &outUV,AtPoint2 &outDu,AtPoint2 &outDv,AtPoint2 noise,AtPoint2 offset,float rotate,AtPoint2 repeat)
{

   float outU = inUV.x;
   float outV = inUV.y;
   float outDuDx = inDu.x;
   float outDuDy = inDu.y;
   float outDvDx = inDv.x;
   float outDvDy = inDv.y;


   AtPoint2 thisBlock = hashBlock(inUV, repeat);

   // noise uv
   if (noise.x > 0.0f)
   {
      AtVector2 uv = {outU * 16, outV * 16};
      outU += noise.x * AiPerlin2(uv);
   }

   if (noise.y > 0.0f)
   {
      AtVector2 uv = {(1 - outU) * 16, (1 - outV) * 16};
      outV += noise.y * AiPerlin2(uv);
   }

   

   // for UVs, translate first, then rotate
   if(offset.x > 0 || offset.y > 0)
   {
      float offsetX = sin(AI_PI*(thisBlock.x+offset.x));
      float offsetY = cos(AI_PI*(thisBlock.y+offset.y));
      outU += offsetX;
      outV += offsetY;
      //outU += offset.x;
      //outV += offset.y;
   }

      

   int mirrorE = (int)thisBlock.x % 2;
   int mirrorO = (int)thisBlock.y % 2;
   // do mirror, stagger before rotation
   if (mirrorE == 0)
   {
      float center = floor(outV) + 0.5f;
      outV = center - (outV - center);

      outDuDy = -outDuDy;
      outDvDy = -outDvDy;
   }
   
   if (mirrorO == 0)
   {
      float center = floor(outU) + 0.5f;
      outU = center - (outU - center);

      outDuDx = -outDuDx;
      outDvDx = -outDvDx;
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
   
   // apply repetition factor
   outU *= repeat.x;
   outV *= repeat.y;

   // replace shader globals
   outUV.x = outU;
   outUV.y = outV;
   outDu.x = outDuDx;
   outDu.y = outDuDy;
   outDv.x = outDvDx;
   outDv.y = outDvDy;
}

node_parameters
{
   AiParameterSTR("texture", "");
}

node_initialize
{
    ShaderData *data = new ShaderData;
    const char *texname = params[p_texture].STR;
    data->texturehandle = AiTextureHandleCreate(texname);
    data->textureparams = new AtTextureParams;
    AiTextureParamsSetDefaults(data->textureparams);
    AiNodeSetLocalData(node, data);
}

node_finish
{
    ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
    AiTextureHandleDestroy(data->texturehandle);
    delete data->textureparams;
    delete data;
}

node_update
{
    ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
}


shader_evaluate
{
   AtPoint2 repeat;
   float rotate;
   AtPoint2 offset;
   AtPoint2 noise;

   if(AiStateGetMsgPnt2("ShaderMeg_TexRepe_repeat", &repeat))
   {
      AiStateGetMsgPnt2("ShaderMeg_TexRepe_repeat", &repeat);
   }
   else
   {
      repeat.x = 1.0f;
      repeat.y = 1.0f;
   }
   if(AiStateGetMsgFlt("ShaderMeg_TexRepe_rotate", &rotate))
   {
      AiStateGetMsgFlt("ShaderMeg_TexRepe_rotate", &rotate);  
   }
   else
   {
      rotate = 0.0f;
   }
   if(AiStateGetMsgPnt2("ShaderMeg_TexRepe_offset", &offset))
   {
      AiStateGetMsgPnt2("ShaderMeg_TexRepe_offset", &offset);
   }
   else
   {
      offset.x = 0.0f;
      offset.y = 0.0f;
   }
   if(AiStateGetMsgPnt2("ShaderMeg_TexRepe_noise", &noise))
   {
      AiStateGetMsgPnt2("ShaderMeg_TexRepe_noise", &noise);
   }
   else
   {
      noise.x = 0.0f;
      noise.y = 0.0f;
   }

   SGCache SGC,SGB;
   bool textureAccess = false;
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);

   SGC.initCache(sg);

   float inBlendU = sg->u;
   float inBlendV = sg->v;
   float inBlendDuDx = sg->dudx;
   float inBlendDuDy = sg->dudy;
   float inBlendDvDx = sg->dvdx;
   float inBlendDvDy = sg->dvdy;

   float outBlendU = inBlendU;
   float outBlendV = inBlendV;
   float outBlendDuDx = inBlendDuDx;
   float outBlendDuDy = inBlendDuDy;
   float outBlendDvDx = inBlendDvDx;
   float outBlendDvDy = inBlendDvDy;


   AtRGBA image,image_u,image_v;
   AtPoint2 move = lerpOffset(repeat);
   float center;
   AtPoint2 inUV,inDu,inDv,outUV,outDu,outDv;
   SGB.initCache(sg);

   // image
   outBlendU = inBlendU;
   outBlendV = inBlendV;

   sg->u = outBlendU;
   sg->v = outBlendV;


   inUV.x = sg->u;
   inUV.y = sg->v;
   inDu.x = sg->dudx;
   inDu.y = sg->dudy;
   inDv.x = sg->dvdx;
   inDv.y = sg->dvdy;   


   TextureFileOperation(inUV, inDu, inDv, outUV, outDu, outDv, noise, offset, rotate, repeat);

   // replace shader globals
   sg->u = outUV.x;
   sg->v = outUV.y;
   sg->dudx = outDu.x;
   sg->dudy = outDu.y;
   sg->dvdx = outDv.x;
   sg->dvdy = outDv.y;

   image = AiTextureHandleAccess(sg, data->texturehandle, data->textureparams, &textureAccess);
   SGB.restoreSG(sg);


   // image_u
   center = floor(inBlendU) + 0.5f;
   inBlendU = center - (inBlendU - center);
   //center = floor(inBlendV) + 0.5f;
   //inBlendV = center - (inBlendV - center);
   outBlendU = inBlendU + move.x;
   outBlendV = inBlendV;
   sg->u = outBlendU;
   sg->v = outBlendV;


   inUV.x = sg->u;
   inUV.y = sg->v;
   inDu.x = sg->dudx;
   inDu.y = sg->dudy;
   inDv.x = sg->dvdx;
   inDv.y = sg->dvdy;   


   TextureFileOperation(inUV, inDu, inDv, outUV, outDu, outDv, noise, offset, rotate, repeat);

   // replace shader globals
   sg->u = outUV.x;
   sg->v = outUV.y;
   sg->dudx = outDu.x;
   sg->dudy = outDu.y;
   sg->dvdx = outDv.x;
   sg->dvdy = outDv.y;

   image_u = AiTextureHandleAccess(sg, data->texturehandle, data->textureparams, &textureAccess);
   SGB.restoreSG(sg);

   
   // blend
   AtPoint2 iuv = {sg->u,sg->v};
   AtPoint2 blend;
   AtPoint2 len = {1/repeat.x,1/repeat.y};
   AtPoint2 fuv;
   fuv.x = (iuv.x-floor(iuv.x/len.x)*len.x)/len.x;
   fuv.y = (iuv.y-floor(iuv.y/len.y)*len.y)/len.y;
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

   sg->out.RGBA = lerp(image_u, image, blend.x);



   // recover sg uv value
   SGC.restoreSG(sg);
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