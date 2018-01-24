/*
   TODO:
   * Clustered interior map;
   * Light interface;						x
   * Fast and simple reflection;
   * Filter width adjust along depth; 	x
*/

#include <ai.h>
#include <algorithm>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <alUtil.h>
#include "interior.h"

#define FORWALLS FORI(NWALLS)
#define FORI(n) for(int i=0; i<n; i++)
#define CHECK(v) {sg->out.RGB=rgb(v); return;}

using std::string;

const char* SHADER_NAME = "Interior";
const int NWALLS = 6;
const float OPACITY_THRESHOLD = 0.95;
const string SLICE_INDEX_INDICATOR = "<SLICE>";
const size_t MAX_PATH_LENGTH = 1<<9;

AI_SHADER_NODE_EXPORT_METHODS(InteriorMtd);

/*
cd ~/git/shader/interior/;make;cp interior.so ~/git/shader/tmp/;
*/
enum Walls {
   kWallLeft,
   kWallRight,
   kWallBack,
   kWallCeil,
   kWallFloor,
   kWallFront,
   kWallInterior
};

enum InteriorParams {
   p_wallLeft,
   p_wallRight,
   p_wallBack,
   p_wallCeil,
   p_wallFloor,
   p_wallFront,
   p_interior,
   p_nSlices,
   p_depth,
   p_AOV_name
};


struct InteriorData {
   string wall_tex_paths[6];
   string interior_tex_path;
   string AOV_name;
   float depth;
   int n_slices;

   AtTextureHandle* wall_handles[6];
   AtTextureHandle** interior_handles;

   InteriorData(){
      FORWALLS
         wall_handles[i] = NULL;
      interior_handles = NULL;
   }

   void freeHandles(){
      // walls handles
      FORWALLS{
         if (wall_handles[i]) {
            AiTextureHandleDestroy(wall_handles[i]);
            wall_handles[i] = NULL;
         }
      }

      // interior handles
      if (n_slices && interior_handles){
         FORI(n_slices){
            AiTextureHandleDestroy(interior_handles[i]);
            interior_handles[i] = NULL;
         }
         delete[] interior_handles;
         interior_handles = NULL;
      }
   }

   bool createHandles(){
      // walls handles
      FORWALLS{
         if (wall_tex_paths[i].size())
            wall_handles[i] = AiTextureHandleCreate(wall_tex_paths[i].c_str());
      }

      // interior handles
      size_t indicator_pos = interior_tex_path.find(SLICE_INDEX_INDICATOR);
      if (indicator_pos == string::npos){
         AiMsgWarning("[%s Shader]: No %s is found in interior texture path.",
                      SHADER_NAME,
                      SLICE_INDEX_INDICATOR.c_str());
      } else if (n_slices) {
         string pattern = interior_tex_path;
         pattern.replace(indicator_pos,
                         SLICE_INDEX_INDICATOR.size(),
                         "%04d");
         char buffer[MAX_PATH_LENGTH];

         interior_handles = new AtTextureHandle*[n_slices];
         FORI(n_slices){

            sprintf(buffer, pattern.c_str(), i+1);
            interior_handles[i] = AiTextureHandleCreate(buffer);
         }
      }
   }
};


node_loader{
   if (i == 0){
      node->methods = InteriorMtd;
      node->output_type = AI_TYPE_RGB;
      node->name = SHADER_NAME;
      node->node_type = AI_NODE_SHADER;
      strcpy(node->version, AI_VERSION);
      return true;
   }else{
      return false;
   }
}


node_parameters
{
   string walls[] = {"left", "Right", "Back", "Ceil", "Floor", "Front"};
   string current_default;

   FORWALLS{
      current_default = "wall_";
      current_default.append(walls[i]);
      current_default.append(".tx");
      AiParameterStr(walls[i].c_str(), current_default.c_str());
   }
   AiParameterStr("interior", "interior.tx");
   AiParameterInt("n_slices", 8);
   AiParameterFlt("depth", 1);
   AiParameterStr("AOV_name", "direct_diffuse");
}


node_initialize
{
   InteriorData* data = new InteriorData();
   AiNodeSetLocalData(node, data);
}


node_update
{
   InteriorData* data = (InteriorData*)AiNodeGetLocalData(node);
   FORWALLS{
      data->wall_tex_paths[i] = params[i].STR;
   }
   data->interior_tex_path = params[p_interior].STR;
   data->n_slices = params[p_nSlices].INT;
   data->depth = -params[p_depth].FLT;
   data->AOV_name = params[p_AOV_name].STR;
   data->freeHandles();
   data->createHandles();
}

node_finish
{
   if (AiNodeGetLocalData(node)){
      InteriorData* data = (InteriorData*)AiNodeGetLocalData(node);
      data->freeHandles();
      delete data;
      AiNodeSetLocalData(node, NULL);
   }
}



void uvSpace(AtMatrix out, const AtShaderGlobals* sg){
   AiM4Identity(out);
   AtVector dpdu = AiV3Normalize(sg->dPdu),
            dpdv = AiV3Normalize(sg->dPdv),
            N = AiV3Normalize(sg->N);
   out[0][0] = dpdu.x;
   out[1][0] = dpdu.y;
   out[2][0] = dpdu.z;
   out[0][1] = dpdv.x;
   out[1][1] = dpdv.y;
   out[2][1] = dpdv.z;
   out[0][2] = N.x;
   out[1][2] = N.y;
   out[2][2] = N.z;
}

shader_evaluate
{
   InteriorData* data = (InteriorData*)AiNodeGetLocalData(node);
   SgUvManager sm(sg);

   // texture params
   AtTextureParams tex_params;
   AiTextureParamsSetDefaults(&tex_params);
   tex_params.filter = AI_TEXTURE_BICUBIC; // TODO: parameter
   if ((sg->Rt & AI_RAY_DIFFUSE) && (tex_params.filter > AI_TEXTURE_BILINEAR))
         tex_params.filter = AI_TEXTURE_BILINEAR;
   Walls wall;

   // check if hit the front wall
   AtRGBA C = AI_RGBA_BLACK;
   bool suc = false;
   if (data->wall_handles[kWallFront]){
      C = AiTextureHandleAccess(sg, data->wall_handles[kWallFront], &tex_params, &suc);
   }
   if (!suc){
      //AiMsgWarning("[%s Shader]: texture of front wall could not be accessed: %s", SHADER_NAME, data->wall_tex_paths[kWallFront].c_str());
      ;
   }
   if (C.a>OPACITY_THRESHOLD){
      sg->out.RGBA = C;
      AiAOVSetRGB(sg, data->AOV_name.c_str(), C.rgb());
      return;
   }


   // solver the s, t, wall
   AtVector Iw = sg->Rd * sg->Rl, I;
   AtMatrix uv_space;
   uvSpace(uv_space, sg);
   AiM4VectorByMatrixMult(&I, uv_space, &Iw);
   AtPoint P = aivec(sg->u, sg->v, 0.0);
   AtVector side = filterStep(AI_V3_ZERO, I);

   float s, t, z;
   float f = (side.y - P.y)/I.y;
   s = f * I.x + P.x;
   z = t = f * I.z;

   int flag = 1;
   bool cap = 1;        // TODO: parameter
   if(s<1 && s>0){
      if (z<data->depth && cap){
         flag = 0;
      }else{
         if (side.y>0)
            wall = kWallCeil;
         else
            wall = kWallFloor;
      }
   }
   else{
      f = (side.x-P.x)/I.x;
      int left = s<0;
      s = f*I.y+P.y;
      z = t = f*I.z+P.z;
      if (z<data->depth && cap){
         flag = 0;
      }else{
         if (left)
            wall = kWallLeft;
         else
            wall = kWallRight;
      }
   }

   if (!flag){
      s = I.x*data->depth/I.z+P.x;
      t = I.y*data->depth/I.z+P.y;
      z = data->depth;
      wall = kWallBack;
   }
   else{
	   // depth distort
	   t/= -data->depth;
	}

   // interior
   if (data->n_slices) {
      sm.push();
      int nslices_to_hit = data->n_slices/data->depth * z;
      float s_interior, t_interior, depth_interior;
      //float a = C.a;
      char buffer[MAX_PATH_LENGTH];
      AtRGBA c_interior;
      FORI(nslices_to_hit){
         depth_interior = float(i)/data->n_slices*data->depth;
         s_interior = I.x*depth_interior/I.z + P.x;
         t_interior = I.y*depth_interior/I.z + P.y;
         sm.set(s_interior, t_interior);
         c_interior = AiTextureHandleAccess(sg, data->interior_handles[i], &tex_params, &suc);
         C = over(c_interior, C);

         if (C.a>OPACITY_THRESHOLD){
            sg->out.RGBA = C;
            AiAOVSetRGB(sg, data->AOV_name.c_str(), C.rgb());
            sm.pop();
            return;
         }
      }
      sm.pop();
   }

   // direction
   float _s;
   switch(wall) {
      case kWallBack: t = t; break;
      case kWallLeft: _s = s; s = -t; t = _s; break;
      case kWallRight: _s = s; s = t; t = _s; break;
      case kWallFloor: t = -t; break;
   }
   s = s-quickFloor(s);
   t = t-quickFloor(t);


   // access texture
   AtRGBA c_inner;
   if (data->wall_handles[wall]){
      sm.push(s, t);
      c_inner = AiTextureHandleAccess(sg, data->wall_handles[wall], &tex_params, &suc);
      sm.pop();
   }
   if (!suc){
      AiMsgWarning("[%s Shader]: texture of wall could not be accessed: %s", SHADER_NAME, data->wall_tex_paths[wall].c_str());
      c_inner = AI_RGBA_GREEN;
   }
   sg->out.RGBA = over(c_inner, C);
   AiAOVSetRGB(sg, data->AOV_name.c_str(), sg->out.RGBA.rgb());
}
