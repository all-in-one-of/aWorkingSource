#include "shader_utils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcTextureRepetitionMethods);

enum TextureRepetitionParams { 
   p_texture,
   p_uvset_name,
};

void TextureFileOperation(AtVector2 inUV,AtVector2 inDu,AtVector2 inDv,AtVector2 &outUV,AtVector2 &outDu,AtVector2 &outDv,AtVector2 noise,AtVector2 offset,float rotate,AtVector2 repeat)
{

   float outU = inUV.x;
   float outV = inUV.y;
   float outDuDx = inDu.x;
   float outDuDy = inDu.y;
   float outDvDx = inDv.x;
   float outDvDy = inDv.y;


   AtVector2 thisBlock = hashBlock(inUV, repeat);

   // noise uv
   if (noise.x > 0.0f)
   {
      AtVector2 uv = AtVector2(outU * 16, outV * 16);
      outU += noise.x * AiPerlin2(uv);
   }

   if (noise.y > 0.0f)
   {
      AtVector2 uv = AtVector2((1 - outU) * 16, (1 - outV) * 16);
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
   AiParameterStr("texture", "");
   AiParameterStr("uvSetName", "");
}

node_initialize
{
   ShaderData *data = new ShaderData;
   const char *texname = AiNodeGetStr(node, "texture");
   data->texturehandle = AiTextureHandleCreate(texname);
   AiTextureParamsSetDefaults(data->textureparams);
   AiNodeSetLocalData(node, data);
}

node_finish
{
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   AiTextureHandleDestroy(data->texturehandle);
   delete data;
}

node_update
{
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   data->uvSetName = AiNodeGetStr(node, "uvSetName");
   data->useCustomUVSet = data->uvSetName.length() > 0;
}


shader_evaluate
{
   AtVector2 repeat;
   float rotate;
   AtVector2 offset;
   AtVector2 noise;

   if(AiStateGetMsgVec2(AtString("ShaderMeg_TexRepe_repeat"), &repeat))
   {
      AiStateGetMsgVec2(AtString("ShaderMeg_TexRepe_repeat"), &repeat);
   }
   else
   {
      repeat.x = 1.0f;
      repeat.y = 1.0f;
   }
   if(AiStateGetMsgFlt(AtString("ShaderMeg_TexRepe_rotate"), &rotate))
   {
      AiStateGetMsgFlt(AtString("ShaderMeg_TexRepe_rotate"), &rotate);  
   }
   else
   {
      rotate = 0.0f;
   }
   if(AiStateGetMsgVec2(AtString("ShaderMeg_TexRepe_offset"), &offset))
   {
      AiStateGetMsgVec2(AtString("ShaderMeg_TexRepe_offset"), &offset);
   }
   else
   {
      offset.x = 0.0f;
      offset.y = 0.0f;
   }
   if(AiStateGetMsgVec2(AtString("ShaderMeg_TexRepe_noise"), &noise))
   {
      AiStateGetMsgVec2(AtString("ShaderMeg_TexRepe_noise"), &noise);
   }
   else
   {
      noise.x = 0.0f;
      noise.y = 0.0f;
   }

   SGCache SGC,SGB;
   bool textureAccess = false;
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);


   // uvset
   if (data->useCustomUVSet)
   {
      AtVector2 altuv;
      if (AiUDataGetVec2(data->uvSetName, altuv))
      {         
         sg->u = altuv.x;
         sg->v = altuv.y;
         AtVector2 altuvDx, altuvDy; 
         if (AiUDataGetDxyDerivativesVec2(data->uvSetName, altuvDx, altuvDy)) 
         { 
            sg->dudx = altuvDx.x; 
            sg->dvdx = altuvDx.y; 
            sg->dudy = altuvDy.x; 
            sg->dvdy = altuvDy.y; 
         }
         else
            sg->dudx = sg->dudy = sg->dvdx = sg->dvdy = 0.0f;
      }
   }

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
   AtVector2 move = lerpOffset(repeat);
   float center;
   AtVector2 inUV,inDu,inDv,outUV,outDu,outDv;
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
   AtVector2 iuv = AtVector2(sg->u,sg->v);
   AtVector2 blend;
   AtVector2 len = AtVector2(1/repeat.x,1/repeat.y);
   AtVector2 fuv;
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

   sg->out.RGBA() = lerp(image_u, image, blend.x);
   //sg->out.RGBA() = image;



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