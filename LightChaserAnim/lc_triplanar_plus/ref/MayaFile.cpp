#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <map>
#include <vector>

#include <ai.h>

#include "MayaUtils.h"

#define MAX_FILENAME 500

AI_SHADER_NODE_EXPORT_METHODS(MayaFileMtd);

namespace
{

// Modes: SHAPE_NAME, SHAPE_PATH, TILE and USER_PARAM has a variable token size,
//  so they will require break the file path where they are found to be able to
//  insert the evaluated token value in the correct place.
// Mode UDIM has a fixed token size = 4. So This token will not break the file path
//  where it is found, but it will be replaced by a 4 character mask that will be
//  updated in the evaluation of the token to improve performance
enum TokenModes
{
   SHAPE_NAME = 0,
   SHAPE_PATH,
   UDIM,
   TILE,
   UTILE,
   VTILE,
   USER_PARAM
};

enum MayaFileParams
{
   p_coverage = 0,
   p_translate_frame,
   p_rotate_frame,
   p_mirror_u,
   p_mirror_v,
   p_wrap_u,
   p_wrap_v,
   p_stagger,
   p_repeat,
   p_offset,
   p_rotate,
   p_filename,
   p_noise,
   p_mip_bias,
   p_uvset_name,
   p_filter,
   p_use_default_color,
   p_color_space,
   MAYA_COLOR_BALANCE_ENUM
};

};

struct TokenData
{
   TokenModes mode;
   int position;    // Position on AtImageData.origName of next string chunk after token
                    //  For UDIM token, as it does not break the current string chunk,
                    //  it is the distance from the end of the current chunk to the begining
                    //  of the UDIM mask as a negative number.

   int nextSize;    // Size of next string chunk after token. For the UDIM token, it does
                    //  not have any meaning so it will be 0
   void* extra;
   void* secondExtra; // for the default value
   int secondExtraLength; // for the default value

   TokenData()
   {
      extra = 0;
      secondExtra = 0;
      secondExtraLength = 0;
   }
};

typedef struct AtImageData
{
   char* origPath;        // Original file path without tokens
   char** processPath;    // Current evaluated file path (one for each thread)
   unsigned int startPos; // First position in the "processPath" where the first token will be added
   TokenData* tokens;
   unsigned int ntokens;
   AtTextureHandle* texture_handle;
   bool useCustomUVSet;
   AtString uvSetName;
   AtString color_space;
   int numThreads;

   static void* operator new(size_t s)
   {
      return AiMalloc(s);
   }
   
   static void operator delete(void* p)
   {
      AiFree(p);
   }
} AtImageData;

namespace { // anonymus namespace
   const char* filterNames[] = {"closest", "bilinear", "bicubic", "smart_bicubic", 0};

   inline int GetTokenOptionInt(std::string& token, int defaultOpt)
   {
      std::string::size_type len = token.length();
      // an option can be provided with <tokname:5>
      size_t foundsep = token.rfind(':');
      if (foundsep != std::string::npos)
         return atoi(token.substr(foundsep, len-(foundsep+1)).c_str());
      return defaultOpt;
   }

   static inline void ComputeUDIMLookup(AtShaderGlobals *sg,
                                        float &udim_u, float &udim_v,
                                        int &col, int &row,
                                        int udim_dim, const AtString &uvSetName)
   {
      AtVector2 uvs[3];
      if (AiShaderGlobalsGetVertexUVs(sg, uvSetName, uvs))
      {
         // if triangles never crossed a tile boundary we could use the UV centroid for
         // lookups. Since they might (rare) we make sure by construction that the lookup
         // is inside the triangle. We nudge the lookup point towards the centroid
         // to prevent fp errors in case the triangle grazes a tile's edge (#4624 &co)
         float udim_bu = AiClamp(sg->bu, 0.0f, 1.0f);
         float udim_bv = AiClamp(sg->bv, 0.0f, 1.0f);
         float udim_bw = 1.0f - udim_bu - udim_bv;

         udim_bu = AiLerp(AI_EPSILON, udim_bu, (1.0f / 3.0f));
         udim_bv = AiLerp(AI_EPSILON, udim_bv, (1.0f / 3.0f));
         udim_bw = AiLerp(AI_EPSILON, udim_bw, (1.0f / 3.0f));

         float adjusted_u = udim_bu * uvs[1].x + udim_bv * uvs[2].x + udim_bw * uvs[0].x;
         float adjusted_v = udim_bu * uvs[1].y + udim_bv * uvs[2].y + udim_bw * uvs[0].y;

         row = static_cast<int>(ceilf(adjusted_v) - 1 );
         col = static_cast<int>(ceilf(adjusted_u) - 1 );
         // rows cannot be negative, and there are idata->udim_dim columns
         row = AiMax(row, 0);
         col = AiClamp(col, 0, udim_dim - 1);
         udim_u = AiClamp(sg->u - col, 0.0f, 1.0f);
         udim_v = AiClamp(sg->v - row, 0.0f, 1.0f);
      }
      else
      {
         // otherwise just use the current uv coordinates (less reliable)
         udim_u = fmodf(sg->u, 1);
         udim_v = fmodf(sg->v, 1);
         row = static_cast<int>(ceilf(sg->v) - 1 );
         col = static_cast<int>(ceilf(sg->u) - 1 );

         if (udim_u == 0 && sg->u > 0)
            udim_u = 1;
         if (udim_v == 0 && sg->v > 0)
            udim_v = 1;

         // compute tile and (u,v) position for lookup, take into account that
         // rows cannot be negative, and there are idata->udim_dim columns
         udim_u = col < 0 ? 0 : (col >= udim_dim ? 1 : udim_u);
         udim_v = row < 0 ? 0 : udim_v;
         row = AiMax(row, 0);
         col = AiClamp(col, 0, udim_dim - 1);
      }
   }
}

node_parameters
{
   AiParameterVec2("coverage", 1.0f, 1.0f);
   AiParameterVec2("translateFrame", 0.0f, 0.0f);
   AiParameterFlt("rotateFrame", 0.0f);
   AiParameterBool("mirrorU", false);
   AiParameterBool("mirrorV", false);
   AiParameterBool("wrapU", true);
   AiParameterBool("wrapV", true);
   AiParameterBool("stagger", false);
   AiParameterVec2("repeatUV", 1.0f, 1.0f);
   AiParameterVec2("offsetUV", 0.0f, 0.0f);
   AiParameterFlt("rotateUV", 0.0f);
   AiParameterStr("filename", "");
   AiParameterVec2("noiseUV", 0.0f, 0.0f);
   AiParameterInt("mipBias", 0);
   AiParameterStr("uvSetName", "");
   AiParameterEnum("filter", 3, filterNames);
   AiParameterBool("useDefaultColor", true);
   AiParameterStr("color_space", "");
   AddMayaColorBalanceParams(params, nentry);   
   
   AiMetaDataSetBool(nentry, NULL, "maya.hide", true);
}

node_initialize
{
}

node_update
{
   // cleanup
   // since node_update may be called multiple times and a static memory allocation in node_init
   // will not suffice, we need to free memory between updates
   Finish(node);

   AtImageData *idata = new AtImageData;;
   idata->uvSetName = AiNodeGetStr(node, "uvSetName");
   idata->color_space = AiNodeGetStr(node, "color_space");
   idata->useCustomUVSet = idata->uvSetName.length() > 0;
   idata->origPath = NULL;
   idata->processPath = NULL;
   idata->startPos = 0;
   idata->tokens = NULL;      
   idata->ntokens = 0;
   idata->texture_handle = NULL;   
   
   // Check if the filename has a shading network. If it does, we will fallback to the name based
   // lookup and avoid the token computation
   if (!AiNodeGetLink(node, "filename"))
   {
      const char *filename = AiNodeGetStr(node, "filename");

      // calculate file tokens
      std::vector<TokenData> tokens;

      std::string fname(filename);
      std::string newfname("");
      std::map< std::string, std::string > processedTokens;
      std::map< std::string, std::string >::iterator iter;
      std::string::size_type lastPos = 0;
      // Find first "non-delimiter".
      std::string::size_type pos = 0;
      std::string::size_type end = fname.length();
      bool opened = false;
      int prevToken = -1;
      int firstBreak = 0;
      bool breakFound = false;
      while ( pos < end && pos != std::string::npos &&
            lastPos < end && lastPos != std::string::npos)
      {
         lastPos = pos;
         // Find next "non-delimiter"
         if (opened)
         {
            pos = fname.find(">", lastPos);
            opened = false;
            if (pos != std::string::npos)
               pos = pos + 1;
         }
         else
         {
            pos = fname.find("<", lastPos);
            opened = true;
         }

         // Found a token, add it to the vector.
         std::string sub = fname.substr(lastPos, pos - lastPos);
         if (opened)
         {
            // a non-token file path part. "sub" has a chunk of text without tokens.
            newfname += sub;
            // If we are at the end of the file path and there is a token that broke the file path chunk, update its "nextSize" attribute
            if (pos == std::string::npos && prevToken >= 0)
               tokens[prevToken].nextSize = (int)newfname.size() - tokens[prevToken].position;
            // If we have not found a token that break the first file path text, increase the size of the "firstBreak" value
            if (!breakFound)
               firstBreak = (int)newfname.size();
            // If last token was a UDIM
            if (tokens.size() > 0 && tokens.back().mode == UDIM)
               // Update it to be the position of the begining of the UDIM mask
               //  relative to the end of the string chunk. Example:
               //   text####text
               //       |
               //       \------- = "-8"
               tokens.back().position = tokens.back().position - (int) newfname.size();
         }
         else
         {
            // process the tokens
            if (sub == "<shapeName>")
            {
               TokenData data;
               data.mode = SHAPE_NAME;
               data.position = (int) newfname.size();
               data.extra = NULL;
               data.nextSize = 0;
               // If a previous token broke the file path chunk, update its "nextSize" attribute
               if (prevToken >= 0)
                  tokens[prevToken].nextSize = (int) newfname.size() - tokens[prevToken].position;
               tokens.push_back(data);
               // Set this token as the last that broke the text chunk
               prevToken = (int) tokens.size()-1;
               breakFound = true;
            }
            else if (sub == "<shapePath>")
            {
               TokenData data;
               data.mode = SHAPE_PATH;
               data.position = (int) newfname.size();
               data.extra = NULL;
               data.nextSize = 0;
               // If a previous token broke the file path chunk, update its "nextSize" attribute
               if (prevToken >= 0)
                  tokens[prevToken].nextSize = (int) newfname.size() - tokens[prevToken].position;
               tokens.push_back(data);
               // Set this token as the last that broke the text chunk
               prevToken = (int) tokens.size()-1;
               breakFound = true;
            }
            else if (sub.substr(0, 6) == "<attr:")
            {
               std::string attr = sub.substr(6, sub.length()-7);
               TokenData data;
               data.mode = USER_PARAM;
               data.position = (int) newfname.size();
               size_t spacePos = attr.find(" ");
               data.secondExtra = 0;
               if (spacePos != std::string::npos)
               {
                  std::string rest = attr.substr(spacePos + 1);
                  if (rest.substr(0, 8) == "default:")
                  {
                     rest = rest.substr(8);
                     data.secondExtra = AiMalloc(rest.size() + 1);
                     data.secondExtraLength = (int)rest.size();
                     strcpy((char*)data.secondExtra, rest.c_str());
                     ((char*)data.secondExtra)[rest.size()] = 0;
                  }
                  attr = attr.substr(0, spacePos);
               }  
               data.extra = AiMalloc(attr.size() + 1);
               strcpy((char*)data.extra, attr.c_str());
               ((char*)data.extra)[attr.size()] = 0;
               data.nextSize = 0;
               // If a previous token broke the file path chunk, update its "nextSize" attribute
               if (prevToken >= 0)
                  tokens[prevToken].nextSize = (int) newfname.size() - tokens[prevToken].position;
               tokens.push_back(data);
               // Set this token as the last that broke the text chunk
               prevToken = (int) tokens.size()-1;
               breakFound = true;
            }
            else if ((sub.substr(0, 5) == "<udim") || (sub.substr(0, 5) == "<UDIM"))
            {
               /*
               mari/zbrush/cyslice-style
               Mari has a 4 digit number equal to 1000 + (u + 1 + v*10). UV tile [0,0] x (1,1) is assigned 1001
               UV tile [0,1]x(1,2) is assigned 1011 etc. 
               */
               int dim = GetTokenOptionInt(sub, 10);
               dim = dim <= 0 ? 1 : dim;

               TokenData data;
               data.mode = UDIM;
               // Position will be updated when the end of the text chunk is reached
               data.position = (int) newfname.size();
               data.extra = AiMalloc(sizeof(int));
               *((int*)data.extra) = dim;

               tokens.push_back(data);
               // This token does not break the text chunk, we only add a 4 characters
               //  mask to be replaced at evaluation time
               newfname += "1001";
               // Just in case file name ends here
               if (!breakFound)
                  firstBreak += 4;
            }
            else if (sub.substr(0, 5) == "<tile")
            {
               // default offset
               int offset = GetTokenOptionInt(sub, 1);

               TokenData data;
               data.mode = TILE;
               data.position = (int) newfname.size();
               data.extra = AiMalloc(sizeof(int));
               *((int*)data.extra) = offset;
               data.nextSize = 0;
               // If a previous token broke the file path chunk, update its "nextSize" attribute
               if (prevToken >= 0)
                  tokens[prevToken].nextSize = (int) newfname.size() - tokens[prevToken].position;
               tokens.push_back(data);
               // Set this token as the last that broke the text chunk
               prevToken = (int) tokens.size()-1;
               breakFound = true;
            }
            else if (sub.substr(0, 6) == "<utile" || sub.substr(0, 3) == "<u>" || sub.substr(0, 3) == "<U>")
            {
               // default offset
               int offset = GetTokenOptionInt(sub, 1);
               if (sub.substr(0, 3) == "<u>")
                  offset -= 1;

               TokenData data;
               data.mode = UTILE;
               data.position = (int) newfname.size();
               data.extra = AiMalloc(sizeof(int));
               *((int*)data.extra) = offset;
               data.nextSize = 0;
               // If a previous token broke the file path chunk, update its "nextSize" attribute
               if (prevToken >= 0)
                  tokens[prevToken].nextSize = (int) newfname.size() - tokens[prevToken].position;
               tokens.push_back(data);
               // Set this token as the last that broke the text chunk
               prevToken = (int) tokens.size()-1;
               breakFound = true;
            }
            else if (sub.substr(0, 6) == "<vtile" || sub.substr(0, 3) == "<v>" || sub.substr(0, 3) == "<V>")
            {
               // default offset
               int offset = GetTokenOptionInt(sub, 1);
               if (sub.substr(0, 3) == "<v>")
                  offset -= 1;

               TokenData data;
               data.mode = VTILE;
               data.position = (int) newfname.size();
               data.extra = AiMalloc(sizeof(int));
               *((int*)data.extra) = offset;
               data.nextSize = 0;
               // If a previous token broke the file path chunk, update its "nextSize" attribute
               if (prevToken >= 0)
                  tokens[prevToken].nextSize = (int) newfname.size() - tokens[prevToken].position;
               tokens.push_back(data);
               // Set this token as the last that broke the text chunk
               prevToken = (int) tokens.size()-1;
               breakFound = true;
            }
            else
            {
               AiMsgError("unknown token %s", sub.c_str());
               newfname += sub;
            }
         }
      }

      idata->ntokens = (unsigned int)tokens.size();
      if (tokens.size())
      {
         idata->tokens = (TokenData*) AiMalloc(sizeof(TokenData) * tokens.size());
         int k = 0;
         for (std::vector<TokenData>::const_iterator it = tokens.begin(); it != tokens.end(); ++it, ++k)
            idata->tokens[k] = *it;

         idata->origPath = (char*) AiMalloc(newfname.size() + 1);
         strcpy(idata->origPath, newfname.c_str());

         // For each thread, create a processPath with the first text chunk already copied to it.
         AtNode* nodeOpt = AiUniverseGetOptions();
         idata->numThreads = AiNodeGetInt(nodeOpt, "threads");
         idata->processPath = (char**) AiMalloc(sizeof(char*) * idata->numThreads);
         for(int k = 0; k < idata->numThreads; k++)
         {
            idata->processPath[k] = (char*) AiMalloc(sizeof(char) * MAX_FILENAME);
            memcpy(idata->processPath[k],idata->origPath,firstBreak);
            idata->startPos = firstBreak;
            idata->processPath[k][firstBreak] = 0;
         }
      }
      else
         idata->texture_handle = AiTextureHandleCreate(AiNodeGetStr(node, "filename"), idata->color_space);
   }
   AiNodeSetLocalData(node, idata);   
}

node_finish
{
   AtImageData *idata = (AtImageData*) AiNodeGetLocalData(node);
   if (idata != NULL)
   {
      // Freeing ShaderData
      if(idata->texture_handle != NULL)
         AiTextureHandleDestroy(idata->texture_handle);      
         
      if (idata->ntokens > 0)
      {
         TokenData* token = idata->tokens;
         for (unsigned int i=0; i < idata->ntokens; i++, token++)
         {
            if (token->extra != NULL)
               AiFree(token->extra);
            if (token->secondExtra != NULL)
               AiFree(token->secondExtra);
         }
         AiFree(idata->tokens);
         AiFree(idata->origPath);

         for(int k = 0; k < idata->numThreads; k++)
         {
            if (idata->processPath[k] != NULL)
               AiFree(idata->processPath[k]);
         }
         AiFree(idata->processPath);
      }
         
     delete idata;
   }
}

shader_evaluate
{
   AtImageData *idata = (AtImageData*) AiNodeGetLocalData(node);
   
   AtVector2 coverage = AiShaderEvalParamVec2(p_coverage);
   AtVector2 translate = AiShaderEvalParamVec2(p_translate_frame);
   float frotate = AiShaderEvalParamFlt(p_rotate_frame);
   bool mirrorU = (AiShaderEvalParamBool(p_mirror_u) == true);
   bool mirrorV = (AiShaderEvalParamBool(p_mirror_v) == true);
   bool wrapU = (AiShaderEvalParamBool(p_wrap_u) == true);
   bool wrapV = (AiShaderEvalParamBool(p_wrap_v) == true);
   bool stagger = (AiShaderEvalParamBool(p_stagger) == true);
   AtVector2 repeat = AiShaderEvalParamVec2(p_repeat);
   AtVector2 offset = AiShaderEvalParamVec2(p_offset);
   float rotate = AiShaderEvalParamFlt(p_rotate);
   AtVector2 noise = AiShaderEvalParamVec2(p_noise);

   const float oldU = sg->u;
   const float oldV = sg->v;   
   const float oldUdx = sg->dudx;
   const float oldUdy = sg->dudy;
   const float oldVdx = sg->dvdx;
   const float oldVdy = sg->dvdy;
   if (idata->useCustomUVSet)
   {
      AtVector2 altuv;
      if (AiUDataGetVec2(idata->uvSetName, altuv))
      {         
         sg->u = altuv.x;
         sg->v = altuv.y;
         AtVector2 altuvDx, altuvDy; 
         if (AiUDataGetDxyDerivativesVec2(idata->uvSetName, altuvDx, altuvDy))
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

   if (noise.x > 0.0f)
   {
      AtVector2 uv(inU * 16, inV * 16);
      outU += noise.x * AiPerlin2(uv);
   }

   if (noise.y > 0.0f)
   {
      AtVector2 uv((1 - inU) * 16, (1 - inV) * 16);
      outV += noise.y * AiPerlin2(uv);
   }

   // for frame, rotate first then translate
   if (frotate <= -AI_EPSILON || frotate >= AI_EPSILON)
   {
      float x, y;
      float ca = cos(frotate);
      float sa = sin(frotate);

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

   outU -= translate.x;
   outV -= translate.y;

   // If coverage.x or coverage.y are <= 1.0f
   //   check of the wrapped u or v coordinades respectively wraps in a valid range
   // If wrap is off, check incoming coordinate is in the range [0, 1]
   if (Mod(outU, 1.0f) > coverage.x ||
       Mod(outV, 1.0f) > coverage.y ||
       (!wrapU && (outU < 0 || outU > coverage.x)) ||
       (!wrapV && (outV < 0 || outV > coverage.y)))
   {      
      MayaDefaultColor(sg, node, p_defaultColor, sg->out.RGBA());      
      // restore shader globals
      sg->u = oldU;
      sg->v = oldV;
      sg->dudx = oldUdx; 
      sg->dudy = oldUdy; 
      sg->dvdx = oldVdx; 
      sg->dvdy = oldVdy;
   }
   else
   {
      float icx = 1.0f / coverage.x;
      float icy = 1.0f / coverage.y;

      if (coverage.x < 1.0f)
      {
         outU = Mod(outU, 1.0f);
      }

      if (coverage.y < 1.0f)
      {
         outV = Mod(outV, 1.0f);
      }

      outU *= icx;
      outV *= icy;

      // apply repetition factor
      outU *= repeat.x;
      outV *= repeat.y;

      // for UVs, translate first, then rotate
      outU += offset.x;
      outV += offset.y;

      outDuDx *= icx * repeat.x;
      outDuDy *= icx * repeat.x;
      outDvDx *= icy * repeat.y;
      outDvDy *= icy * repeat.y;

      // do mirror, stagger before rotation
      if (Mod(outV, 2.0f) >= 1.0f)
      {
         if (stagger)
         {
            outU += 0.5f;
         }

         if (mirrorV)
         {
            float center = floor(outV) + 0.5f;
            outV = center - (outV - center);

            outDuDy = -outDuDy;
            outDvDy = -outDvDy;
         }
      }
      
      if (mirrorU && Mod(outU, 2.0f) >= 1.0f)
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
      
      // replace shader globals
      sg->u = outU;
      sg->v = outV;
      sg->dudx = outDuDx;
      sg->dudy = outDuDy;
      sg->dvdx = outDvDx;
      sg->dvdy = outDvDy;

      // do texture lookup
      AtTextureParams texparams;
      AiTextureParamsSetDefaults(texparams);
      texparams.mipmap_bias = AiShaderEvalParamInt(p_mip_bias);
      texparams.filter = AiShaderEvalParamInt(p_filter);
      if ((sg->Rt & AI_RAY_DIFFUSE_REFLECT) && (texparams.filter > AI_TEXTURE_BILINEAR))
         texparams.filter = AI_TEXTURE_BILINEAR;
      bool success = true;
      bool useDefaultColor = AiShaderEvalParamBool(p_use_default_color);
      bool* successP = useDefaultColor ? &success : 0;
      if (idata->ntokens > 0 && sg->tid < idata->numThreads)
      {
         TokenData* token = idata->tokens;
         unsigned int pos = 0;
         pos = idata->startPos;
         for (unsigned int i=0; (i < idata->ntokens) && success; i++, token++)
         {
            switch(token->mode)
            {
               // calculate the value of the map
               case SHAPE_NAME:
               {
                  // short shape name
                  AtNode* shape = sg->Op;
                  
                  const char* shapeName = AiNodeGetName(shape);
                  int lastSep = -1;
                  int it = 0;
                  for(char c = '0';((c = (shapeName[it])) != '\0') && (c != '@');++it)
                  {
                     if (c == '|')
                        lastSep = it;
                  }
                  if (lastSep == -1)
                  {
                     memcpy(&(idata->processPath[sg->tid][pos]),shapeName,it);
                     pos += (unsigned int) it;
                  }
                  else
                  {
                     lastSep += 1;
                     const unsigned int memcpySize = (unsigned int)(it - lastSep);
                     memcpy(&(idata->processPath[sg->tid][pos]), shapeName + lastSep, memcpySize);
                     pos += memcpySize;
                  }
                  // Copy next text chunk to the "processPath"
                  memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                  pos += token->nextSize;
                  // Set the end of the string
                  idata->processPath[sg->tid][pos] = 0;
                  break;
               }
               case SHAPE_PATH:
               {
                  // full path with underscores for illegal characters
                  AtNode* shape = sg->Op;
                  const char* shapeName = AiNodeGetName(shape);
                  char c;
                  while(((c = *(shapeName++)) != '\0') && (c != '@'))
                  {
                     if ((c == '|') || (c == ':'))
                        c = '_';
                     idata->processPath[sg->tid][pos++] = c;
                  }
                  // Copy next text chunk to the "processPath"
                  memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                  pos += token->nextSize;
                  // Set the end of the string
                  idata->processPath[sg->tid][pos] = 0;
                  break;
               }
               case USER_PARAM:
               {
                  // user attributes
                  AtString value;
                  if (AiUDataGetStr(AtString((const char*)token->extra), value)) // TODO: store as AtString
                  {
                     size_t len = value.length();
                     memcpy(&(idata->processPath[sg->tid][pos]),value.c_str(),len);
                     pos += (unsigned int) len;
                     // Copy next text chunk to the "processPath"
                     memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                     pos += token->nextSize;
                     // Set the end of the string
                     idata->processPath[sg->tid][pos] = 0;
                  }
                  else
                  {
                     if (token->secondExtra != 0)
                     {
                        int len = token->secondExtraLength;
                        memcpy(&(idata->processPath[sg->tid][pos]),token->secondExtra,len);
                        pos += (unsigned int) len;
                        // Copy next text chunk to the "processPath"
                        memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                        pos += token->nextSize;
                        // Set the end of the string
                        idata->processPath[sg->tid][pos] = 0;
                     }
                     else
                     {
                        // TODO: only warn once
                        // AiMsgWarning("could not find user attribute %s for token %s", attr.c_str(), sub.c_str());
                        idata->processPath[sg->tid][pos] = 0;
                        success = false;
                        const char* shapeName = AiNodeGetName(sg->shader);
                        AiMsgWarning("[MayaFile] Could not find user attribute %s for file node %s, setting to default color", (const char*)token->extra, shapeName);
                     }
                  }
                  break;
               }
               case UDIM:
               {
                  // default dimension
                  int* ptr = (int*)token->extra;
                  int dim = *ptr;
                  
                  /*float udim_u = fmodf(outU, 1.0f);
                  float udim_v = fmodf(outV, 1.0f);
                  int row = static_cast<int>(ceilf(outV) - 1.0f);
                  int col = static_cast<int>(ceilf(outU) - 1.0f);

                  udim_u = col < 0 ? 0.0f : (col >= dim ? 1.0f : udim_u);
                  udim_v = row < 0 ? 0.0f : udim_v;

                  row = AiMax(row, 0);
                  col = AiClamp(col, 0, dim - 1);

                  const float eps = static_cast<float>(dim) / 65536.0f;
                  AdjustUDIMLookup(sg, udim_u, udim_v, col, row, eps, dim);*/
                  int col, row;
                  float udim_u, udim_v;
                  if (idata->useCustomUVSet)
                     ComputeUDIMLookup(sg, udim_u, udim_v, col, row, dim, idata->uvSetName);
                  else
                  {
                     static const AtString emptyStr("");
                     ComputeUDIMLookup(sg, udim_u, udim_v, col, row, dim, emptyStr);
                  }
                  

                  int mariCode = 1001 + col + (row * dim);

                  // Update the UDIM mask in the "processPath"
                  idata->processPath[sg->tid][pos + token->position+3] = (mariCode%10) + '0';
                  mariCode /= 10;
                  idata->processPath[sg->tid][pos + token->position+2] = (mariCode%10) + '0';
                  mariCode /= 10;
                  idata->processPath[sg->tid][pos + token->position+1] = (mariCode%10) + '0';
                  mariCode /= 10;
                  idata->processPath[sg->tid][pos + token->position+0] = (mariCode%10) + '0';
                  
                  sg->u = udim_u;
                  sg->v = udim_v;
                  
                  texparams.wrap_s = AI_WRAP_MIRROR;
                  texparams.wrap_t = AI_WRAP_MIRROR;
                  
                  break;
               }
               case TILE:
               {
                  // mudbox-style uv tiles: u1_v1
                  // Mudbox default format is to use _uU_vV in the file name where U = u + 1 and V = v + 1 are the tile coordinates plus one.
                  // UV tile [0,0] x (1,1) is marked with _u1_v1
                  // UV tile [0,1]x(1,2) is marked with _u1_v2 etc.

                  // default offset
                  int* ptr = (int*)token->extra;
                  int offset = *ptr;

                  int col = static_cast<int>(floorf(inU)) + offset;
                  int row = static_cast<int>(floorf(inV)) + offset;
                  char buf[16];
                  sprintf(buf, "_u%d_v%d", col, row);
                  int len = (int) strlen(buf);
                  memcpy(&(idata->processPath[sg->tid][pos]),buf,len);
                  pos += len;
                  // Copy next text chunk to the "processPath"
                  memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                  pos += token->nextSize;
                  // Set the end of the string
                  idata->processPath[sg->tid][pos] = 0;
                  break;
               }
               case UTILE:
               {
                  // default offset
                  int* ptr = (int*)token->extra;
                  int offset = *ptr;

                  int col = static_cast<int>(floorf(inU)) + offset;
                  char buf[8];
                  sprintf(buf, "%d", col);
                  int len = (int) strlen(buf);
                  memcpy(&(idata->processPath[sg->tid][pos]),buf,len);
                  pos += len;
                  // Copy next text chunk to the "processPath"
                  memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                  pos += token->nextSize;
                  // Set the end of the string
                  idata->processPath[sg->tid][pos] = 0;
                  break;
               }
               case VTILE:
               {
                  // default offset
                  int* ptr = (int*)token->extra;
                  int offset = *ptr;

                  int row = static_cast<int>(floorf(inV)) + offset;
                  char buf[8];
                  sprintf(buf, "%d", row);
                  int len = (int) strlen(buf);
                  memcpy(&(idata->processPath[sg->tid][pos]),buf,len);
                  pos += len;
                  // Copy next text chunk to the "processPath"
                  memcpy(&(idata->processPath[sg->tid][pos]),&(idata->origPath[token->position]),token->nextSize);
                  pos += token->nextSize;
                  // Set the end of the string
                  idata->processPath[sg->tid][pos] = 0;
                  break;
               }
            }// switch
         }

         if (success)
            sg->out.RGBA() = AiTextureAccess(sg, AtString(idata->processPath[sg->tid]), idata->color_space, texparams, successP);
      }
      else if (idata->texture_handle != NULL)
      {
         sg->out.RGBA() = AiTextureHandleAccess(sg, idata->texture_handle, texparams, successP);
      }
      else
      {       
         sg->out.RGBA() = AiTextureAccess(sg, AiShaderEvalParamStr(p_filename), idata->color_space, texparams, successP);
      }
      sg->u = oldU;
      sg->v = oldV;
      sg->dudx = oldUdx; 
      sg->dudy = oldUdy; 
      sg->dvdx = oldVdx; 
      sg->dvdy = oldVdy;
      if (useDefaultColor && !success)
         MayaDefaultColor(sg, node, p_defaultColor, sg->out.RGBA());
      else if (success)
         MayaColorBalance(sg, node, p_defaultColor, sg->out.RGBA());     
   }
}
