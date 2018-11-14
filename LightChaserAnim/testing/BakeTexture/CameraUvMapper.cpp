#include <ai.h>

#include "MayaUtils.h"
#include <vector>
#include <iostream>

#include <stdio.h>
#include <stdexcept>

#ifdef WIN32
#include <windows.h>
#endif

/**
*   Camera Shader that "deviates" the eye rays so that they intersect
*   the points in polymesh with desired UVs
*   Adapted from Kettle Bake shader  https://bitbucket.org/Kettle/kettle_uber/wiki/Kettle_Bake
*
*   Note that to work correctly, the polymesh triangle data must have been previously
*   initialized. Otherwise, the triangles won't be stored properly during node_initialize
*/
AI_CAMERA_NODE_EXPORT_METHODS(CameraUvMapperMtd);

class PolymeshUvMapper{
public:

   PolymeshUvMapper(AtNode* node, AtNode* camera_node);
   ~PolymeshUvMapper();

   struct BakeTriangle {
      BakeTriangle(unsigned int i, unsigned int j, unsigned int k)
      {
         idx[0] = i;
         idx[1] = j;
         idx[2] = k;
      }
      // current triangle's vertex indices. Will be really useful when we stop
      // duplicating the vertices for each triangle :-/
      unsigned int idx[3];
   };
   struct BakeVertex {
      BakeVertex(const AtVector& p, const AtVector& n, const AtVector2& u) :
      position(p), normal(n), uv(u) {}

      AtVector position;
      AtVector normal;
      AtVector2 uv;
   };

   struct Grid2DAccel {
      Grid2DAccel(unsigned int grid_size) : mGridSize(grid_size){}

      const unsigned int mGridSize;
      std::vector<std::vector<unsigned int> > mElements;
   };


   bool intersect(const unsigned int& element,
      const AtVector2& uv, AtVector& pout, AtVector& nout) const
   {
      const BakeTriangle &tri = mTriangles[element];
      const AtVector2& A = mVertices[tri.idx[0]].uv;
      const AtVector2& B = mVertices[tri.idx[1]].uv;
      const AtVector2& C = mVertices[tri.idx[2]].uv;

      const AtVector2 v0( C.x - A.x, C.y - A.y );
      const AtVector2 v1( B.x - A.x, B.y - A.y );
      const AtVector2 v2( uv.x - A.x, uv.y - A.y );

      const float dot00 = AiV2Dot(v0, v0);
      const float dot01 = AiV2Dot(v0, v1);
      const float dot02 = AiV2Dot(v0, v2);
      const float dot11 = AiV2Dot(v1, v1);
      const float dot12 = AiV2Dot(v1, v2);

      const float denom = (dot00 * dot11 - dot01 * dot01);
      if (denom == 0.0f)
         return false;

      const float invDenom = 1.0f / denom;
      AtVector2 bary;
      bary.x = (dot11 * dot02 - dot01 * dot12) * invDenom;
      bary.y = (dot00 * dot12 - dot01 * dot02) * invDenom;

      if ((bary.x >= -0.00001f) && (bary.y >= -0.00001f) && ((bary.x + bary.y) <= 1.00001f))
      {
         float w = 1.f - bary.x - bary.y;
         pout = mVertices[tri.idx[0]].position * w + mVertices[tri.idx[1]].position * bary.y
            + mVertices[tri.idx[2]].position * bary.x;

         nout = mVertices[tri.idx[0]].normal * w + mVertices[tri.idx[1]].normal * bary.y
            + mVertices[tri.idx[2]].normal * bary.x;
         nout = AiV3Normalize(nout);

         return true;
      }
      return false;
   }

   bool findSurfacePoint(const AtVector2& uv, AtVector& pout, AtVector& nout)
   {
      pout = AI_V3_ZERO;
      nout = AI_V3_ZERO;

      const unsigned int grid_max = mGrid->mGridSize - 1;
      const float grid_f = (float)mGrid->mGridSize;

      // get index in the grid for given UV coordinates
      const int grid_coord = AiClamp((int)(uv.x * grid_f), 0, (int)grid_max)
         + AiClamp((int)(uv.y * grid_f), 0, (int)grid_max) * mGrid->mGridSize;

      // looping over the triangles overlapping this grid element
      for (std::vector<unsigned int>::const_iterator it = mGrid->mElements[grid_coord].begin(); it != mGrid->mElements[grid_coord].end(); ++it)
      {
         if (intersect(*it, uv, pout, nout))
         {
            return true;
         }
      }
      return false;
   }

   static void* operator new(size_t s){ return AiMalloc(s); }
   static void operator delete(void* p){ AiFree(p); }

private:

   std::vector<BakeTriangle>  mTriangles;
   std::vector<BakeVertex> mVertices;
   Grid2DAccel* mGrid;
};

static const AtString polymesh_str("polymesh");

PolymeshUvMapper::PolymeshUvMapper(AtNode* node, AtNode* camera_node)
{

   if (node != 0 && AiNodeIs(node, polymesh_str))
   {
      AtMatrix localToWorld = AiNodeGetMatrix(node, "matrix");

      int gridSize = AiNodeGetInt(camera_node, "grid_size");
      float u_offset = AiNodeGetFlt(camera_node, "u_offset");
      float v_offset = AiNodeGetFlt(camera_node, "v_offset");
      AtString uv_set = AiNodeGetStr(camera_node, "uv_set");
      float u_scale = AiNodeGetFlt(camera_node, "u_scale");
      float v_scale = AiNodeGetFlt(camera_node, "v_scale");
      mGrid = new Grid2DAccel(gridSize);

      const unsigned int gridCount = gridSize * gridSize;
      mGrid->mElements.resize(gridCount);

      float bbox[4];
      unsigned int coords[4];
      const unsigned int grid_max = gridSize - 1;
      const float grid_f = (float)gridSize;

      AtShaderGlobals* sg = AiShaderGlobals();
      sg->Op = node;
      sg->fi = 0;
      sg->tid = 0;
      sg->Rt = AI_RAY_CAMERA;
      sg->time = 0.f;

      AtVector localPos[3], worldPos[3];
      AtVector localNormal[3], worldNormal[3];
      AtVector2 uv[3];

      unsigned int triangleIndex = 0;
      unsigned int vertexIndex = 0;
      bool no_uvs = true;


      // looping over triangles in (subdivided/displaced) mesh
      while (AiShaderGlobalsGetTriangle(sg, 0, localPos))
      {
         if (!AiShaderGlobalsGetVertexUVs(sg, uv_set, uv))
         {

            // If I don't have any UVs, how can I ever render this as a texture ?
            // let's skip this polygon, hoping other ones will have UVs
            sg->fi = ++triangleIndex;
            continue;
         }

         uv[0].x += u_offset;
         uv[1].x += u_offset;
         uv[2].x += u_offset;
         uv[0].y += v_offset;
         uv[1].y += v_offset;
         uv[2].y += v_offset;
         uv[0].x *= u_scale;
         uv[1].x *= u_scale;
         uv[2].x *= u_scale;
         uv[0].y *= v_scale;
         uv[1].y *= v_scale;
         uv[2].y *= v_scale;

         if (!AiShaderGlobalsGetVertexNormals(sg, 0, localNormal))
         {
            // no normals (should that happen ?)
            // so let's compute a geometric normal
            AtVector edgeU = localPos[1] - localPos[0];
            AtVector edgeV = localPos[2] - localPos[0];
            localNormal[0] = AiV3Cross(edgeU, edgeV);
            float Nlength = AiV3Length(localNormal[0]);
            if (Nlength > AI_EPSILON) localNormal[0] /= Nlength;
            else
            {
               // ugh... null size triangle. Skip it...
               sg->fi = ++triangleIndex;
               continue;
            }
            // let's assign the same geometric normal to 3 vertices
            localNormal[1] = localNormal[2] = localNormal[0];
         }
         no_uvs = false;

         mTriangles.push_back(BakeTriangle(vertexIndex, vertexIndex + 1, vertexIndex + 2));

         // Please God forgive me for duplicating the vertices at each triangle :-/
         for (int j = 0; j < 3; ++j)
         {
            // convert local vertices to world
            worldPos[j]    = AiM4PointByMatrixMult (localToWorld, localPos[j]);
            worldNormal[j] = AiM4VectorByMatrixMult(localToWorld, localNormal[j]);
            mVertices.push_back(BakeVertex(worldPos[j], worldNormal[j], uv[j]));
         }

         // get Uv Bbox for this Triangle
         bbox[0] = AiMin(uv[0].x, uv[1].x, uv[2].x);
         bbox[1] = AiMin(uv[0].y, uv[1].y, uv[2].y);
         bbox[2] = AiMax(uv[0].x, uv[1].x, uv[2].x);
         bbox[3] = AiMax(uv[0].y, uv[1].y, uv[2].y);

         // convert that into XY grid coordinates
         coords[0] = (unsigned int)AiClamp((int)((bbox[0] - 0.001f) * grid_f), 0, (int)grid_max);
         coords[1] = (unsigned int)AiClamp((int)((bbox[1] - 0.001f) * grid_f), 0, (int)grid_max);
         coords[2] = (unsigned int)AiClamp((int)((bbox[2] + 0.001f) * grid_f), 0, (int)grid_max) + 1;
         coords[3] = (unsigned int)AiClamp((int)((bbox[3] + 0.001f) * grid_f), 0, (int)grid_max) + 1;

         for (unsigned int y = coords[1]; y < coords[3]; ++y)
         {
            unsigned int gridIndex = y * gridSize + coords[0];
            for (unsigned int x = coords[0]; x < coords[2]; ++x, ++gridIndex)
            {
               mGrid->mElements[gridIndex].push_back(triangleIndex);
            }
         }

         vertexIndex += 3;
         sg->fi = ++triangleIndex;
      }

      AiShaderGlobalsDestroy(sg);
      if (triangleIndex == 0 || no_uvs)
      {
         std::string errLog = "CameraUvMapper : Polymesh";
         errLog += AiNodeGetName(node);

         if (no_uvs)  errLog += "doesn't have any valid UVs => Impossible to Render as a Texture";
         else         errLog += "is empty or hasn't been properly initialized";

         AiMsgError("%s", errLog.c_str());

      }
   }
   else
      throw(std::runtime_error("Object is not a polymesh"));
}

PolymeshUvMapper::~PolymeshUvMapper()
{
   delete mGrid;
}


node_parameters
{
   AiParameterStr("polymesh", "");
   AiParameterFlt("offset", 0.1f);
   AiParameterInt("grid_size", 16);
   AiParameterFlt("u_offset", 0.0f);
   AiParameterFlt("v_offset", 0.0f);
   AiParameterStr("uv_set", "");
   AiParameterFlt("u_scale", 1.0f);
   AiParameterFlt("v_scale", 1.0f);

   AiMetaDataSetStr(nentry, NULL, "_synonym", "cameraUvMapper");
   AiMetaDataSetStr(nentry, NULL, "maya.name", "aiCameraUvMapper");
}

struct CameraUvMapperData{
   PolymeshUvMapper* uvMapper;
   float p_offset;
   CameraUvMapperData() : uvMapper(0), p_offset(0.001f){}

   ~CameraUvMapperData(){ clearUvMapper(); }

   void allocateUvMapper(AtNode* polymesh, AtNode* camera) {
      clearUvMapper();
      uvMapper = new PolymeshUvMapper(polymesh, camera);

   }
   void clearUvMapper() {
      if (uvMapper)
      {
         delete uvMapper;
         uvMapper = 0;
      }
   }
   static void* operator new(size_t s){ return AiMalloc((unsigned long)s); }
   static void operator delete(void* p){ AiFree(p); }
};

node_initialize
{
   // Unfortunately at this stage we're not sure that
   // the polymesh triangles have been correctly initialized
   // Until we find a solution, we need to run first a Free render,
   // and then abort it. We also need to make sure that preserve_scene_data
   // is true.

   AiCameraInitialize(node);

   CameraUvMapperData* data = new CameraUvMapperData();
   AiNodeSetLocalData(node, data);

   const char* polymesh = AiNodeGetStr(node, "polymesh");
   AtNode* input_node = AiNodeLookUpByName(polymesh);

   if (!AiNodeGetBool(AiUniverseGetOptions(), "preserve_scene_data"))
   {
      data->clearUvMapper();
      AiMsgError("Preserve Scene Data is not set!");
   }
   else
   {
      if (input_node == 0)
      {
         AiMsgError("The input object can't be found!");
         AiMsgError("%s", polymesh);
      }
      else
      {
         try
         {

            data->allocateUvMapper(input_node, node);
            data->p_offset = AiNodeGetFlt(node, "offset");
         }
         catch (std::exception ex)
         {
            data->clearUvMapper();
            AiMsgError("[CameraUvMapper] Exception caught at Node Initialize %s", ex.what());
         }
      }
   }
}

node_update
{
}

node_finish
{
   CameraUvMapperData *data = (CameraUvMapperData*)AiNodeGetLocalData(node);
   delete data;
}

camera_create_ray
{

   CameraUvMapperData *data = (CameraUvMapperData*)AiNodeGetLocalData(node);
   output.weight = 0.0f;

   if (data->uvMapper == 0)
      return;

   const AtVector2 screen_uv( (input.sx + 1.0f) * 0.5f, (input.sy + 1.0f) * 0.5f );

   AtVector position;
   AtVector normal;

   if (data->uvMapper->findSurfacePoint(screen_uv, position, normal))
   {
      output.dir = -normal;
      output.origin = position + data->p_offset * normal;

      // could we compute derivatives ?
      output.dDdx = AI_V3_ZERO;
      output.dDdy = AI_V3_ZERO;
      output.dOdx = AI_V3_ZERO;
      output.dOdy = AI_V3_ZERO;
      output.weight = 1.0f;
   }
}

camera_reverse_ray
{
//TODO: implement this!
   return true;
}
