#include <ai.h>


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <sstream>

#include "stdlib.h"
#include "SpeedtreeReader/SpeedTreeVertex.h"
#include "SpeedtreeReader/readSpeedTreeSrt.h"
#include "SpeedtreeReader/SpeedTreeVertex.h"

#include "Core/Core.h"
#include "Core/Wind.h"
#include "Core/CoordSys.h"

#define  int32 int
#define  FVector vec3
#define  FVector4 vec4

using namespace SpeedTree;
using namespace std;
using namespace glm;


AI_PROCEDURAL_NODE_EXPORT_METHODS(ST_DSO);

struct ProcArrays
{
    AtArray *nsidesArray;               //number of primitives and size of each primitive, "motion key":  motion blur time sample

    AtArray *vidxsArray ;               //mesh vertices, Anticlockwise, index group
    AtArray *vlistArray;                //final vertext list array
    AtArray *vdefaultlistArray;           //VERTEX_PROPERTY_POSITION, mesh vertices, real value,index ordered

    AtArray *uvidxsArray;               //uv vertices, the same order with vidx
    AtArray *uvlistArray;               //VERTEX_PROPERTY_DIFFUSE_TEXCOORDS,uv vertices, real value, index ordered

    AtArray *nidxsArray;                //normal vertices, index group
    AtArray *nlistArray;                //VERTEX_PROPERTY_NORMAL, normal vertices, real value, index ordered

    AtArray *tidxsArray;                //tangent vertices
    AtArray *tlistArray;                //VERTEX_PROPERTY_TANGENT, tangent vertices, real value


    AtArray *DIFFUSE_0_array;            //DIFFUSE_TEXCOORDS
    AtArray *DIFFUSE_0_idx;

    AtArray *LIGHTMAP_1_array;           //VERTEX_PROPERTY_LIGHTMAP_TEXCOORDS
    AtArray *LIGHTMAP_1_idx;

    AtArray *WIND_BRANCH_2_array;        //VERTEX_PROPERTY_WIND_BRANCH_DATA[0,1], save the branch level 1 data
    AtArray *WIND_BRANCH_2_idx;

    AtArray *WIND_BRANCH_2_L2_array;     //VERTEX_PROPERTY_WIND_BRANCH_DATA[2,3], save the branch level 2 data, WIND_BRANCH_DATA
    AtArray *WIND_BRANCH_2_L2_idx;

    AtArray *LOD_XY_3_array;             //VERTEX_PROPERTY_LEAF_CARD_LOD_SCALAR, or VERTEX_PROPERTY_LOD_POSITION
    AtArray *LOD_XY_3_idx;

    AtArray *LOD_Z_4_array;              //VERTEX_PROPERTY_LEAF_CARD_LOD_SCALAR, or VERTEX_PROPERTY_LOD_POSITION, or VERTEX_PROPERTY_BRANCH_SEAM_DIFFUSE
    AtArray *LOD_Z_4_idx;

    AtArray *Detail_UV_5_array;          //VERTEX_PROPERTY_DETAIL_TEXCOORDS, or VERTEX_PROPERTY_WIND_EXTRA_DATA
    AtArray *Detail_UV_5_idx;

    AtArray *Seam_6_array;               //VERTEX_PROPERTY_BRANCH_SEAM_DIFFUSE,or VERTEX_PROPERTY_WIND_EXTRA_DATA
    AtArray *Seam_6_idx;

    AtArray *Other_7_array;              //VERTEX_PROPERTY_WIND_FLAGS, A single value, applies to leaf geometry only. If less than zero, the main leaf wind algorithm should be applied. Otherwise, apply the alternate leaf wind algorithm.
    AtArray *Other_7_idx;

    int GeometryType;
    string meshName;
};

struct WindData
{
    FSpeedTreeData speedtreePreMotionData;           //a structure
    FSpeedTreeData speedtreeData;                    //a structure
    FSpeedTreeData speedtreeMotionData;              //a structure
    string srtFile;
    string configFile;
    string crntDirectory;

    float GlobalFrequency;
    float GustFrequency;
    float WindSpeed;
    AtVector WindDirection;

    string MeshName;
    int WindType;
    int LODType;
    int LODSmoothType;

    float BillboardThreshold;
    float frameNum;
    float crntTime;
    float fps;

    float SpeedKeyFrame_1;
    float SpeedKeyFrame_2;
    float SpeedKeyFrame_3;
    float SpeedKeyFrame_4;

    float SpeedResponseTime_1;
    float SpeedResponseTime_2;
    float SpeedResponseTime_3;
    float SpeedResponseTime_4;

    float SpeedKeyValue_1;
    float SpeedKeyValue_2;
    float SpeedKeyValue_3;
    float SpeedKeyValue_4;

    float DireKeyFrame_1;
    float DireKeyFrame_2;
    float DireKeyFrame_3;
    float DireKeyFrame_4;

    float DireResponseTime_1;
    float DireResponseTime_2;
    float DireResponseTime_3;
    float DireResponseTime_4;

    Vec3 DireKeyValue_1;
    Vec3 DireKeyValue_2;
    Vec3 DireKeyValue_3;
    Vec3 DireKeyValue_4;

    vector<float> SpeedKeyFrames;
    vector<float> SpeedResponseTimes;
    vector<float> SpeedKeyValues;
    vector<float> DireKeyFrames;
    vector<float> DireResponseTimes;
    vector<Vec3> DireKeyValues;

    bool GlobalMotion;
    bool CalDisp;               // calculate displacement
    bool MotionBlur;
    int MotionKey;
    float MotionLength;


    ReadSpeedTreeSrt *readSTS = new ReadSpeedTreeSrt();
    SLod *sLod = new SLod();
    vector<AtNode*> g_objects;
    ProcArrays* procArrays[4] = {new ProcArrays(), new ProcArrays(),new ProcArrays(),new ProcArrays()};

    int id = 0;

};


enum STM_wind_dis_Params
{
    p_srtFile,
    p_configFile,

    p_globalFrequency,
    p_gustFrequency,
    p_windSpeed,
    p_windDirection,

    p_meshName,
    p_windType,
    p_LODType,
    p_LODSmoothType,

    p_speedKeyFrame_1,
    p_speedKeyFrame_2,
    p_speedKeyFrame_3,
    p_speedKeyFrame_4,

    p_speedResponseTime_1,
    p_speedResponseTime_2,
    p_speedResponseTime_3,
    p_speedResponseTime_4,

    p_speedKeyValue_1,
    p_speedKeyValue_2,
    p_speedKeyValue_3,
    p_speedKeyValue_4,

    p_direKeyFrame_1,
    p_direKeyFrame_2,
    p_direKeyFrame_3,
    p_direKeyFrame_4,

    p_direResponseTime_1,
    p_direResponseTime_2,
    p_direResponseTime_3,
    p_direResponseTime_4,

    p_direKeyValue_1,
    p_direKeyValue_2,
    p_direKeyValue_3,
    p_direKeyValue_4,

    p_globalMotion,
    p_calDisp,
    p_hasMotionBlur,
    p_motionKey,
    p_motionLength
};


//////////////////////////////////////////////////////////
// Procedural parameters

node_parameters
{
    AiParameterStr("srtFile", "");            // is used in procedural; a void*
    AiParameterStr("configFile", "");            // is used in procedural; a void*

    AiParameterFlt("globalFrequency", 0.0);
    AiParameterFlt("gustFrequency", 0.0);
    AiParameterFlt("windSpeed", 0.0);
    AiParameterVec("windDirection", 1, 0, 0);

    AiParameterStr("meshName", "");
    AiParameterInt("windType", 6);
    AiParameterInt("LODType", 0);
    AiParameterInt("LODSmoothType", 0);

    AiParameterFlt("speedKeyFrame_1", 1.0);
    AiParameterFlt("speedKeyFrame_2", -1.0);
    AiParameterFlt("speedKeyFrame_3", -1.0);
    AiParameterFlt("speedKeyFrame_4", -1.0);

    AiParameterFlt("speedResponseTime_1", 1.0);
    AiParameterFlt("speedResponseTime_2", -1.0);
    AiParameterFlt("speedResponseTime_3", -1.0);
    AiParameterFlt("speedResponseTime_4", -1.0);

    AiParameterFlt("speedKeyValue_1", 0.3);
    AiParameterFlt("speedKeyValue_2", -1.0);
    AiParameterFlt("speedKeyValue_3", -1.0);
    AiParameterFlt("speedKeyValue_4", -1.0);

    AiParameterFlt("direKeyFrame_1", 1.0);
    AiParameterFlt("direKeyFrame_2", -1.0);
    AiParameterFlt("direKeyFrame_3", -1.0);
    AiParameterFlt("direKeyFrame_4", -1.0);

    AiParameterFlt("direResponseTime_1", 1.0);
    AiParameterFlt("direResponseTime_2", -1.0);
    AiParameterFlt("direResponseTime_3", -1.0);
    AiParameterFlt("direResponseTime_4", -1.0);

    AiParameterVec("direKeyValue_1", 1.0, 0.0, 0.0);
    AiParameterVec("direKeyValue_2", -1.0, -1.0, -1.0);
    AiParameterVec("direKeyValue_3", -1.0, -1.0, -1.0);
    AiParameterVec("direKeyValue_4", -1.0, -1.0, -1.0);

    AiParameterBool("globalMotion", false);
    AiParameterBool("calDisp", true);
    AiParameterBool("hasMotionBlur", false);
    AiParameterInt("motionKey", 3);
    AiParameterFlt("motionLength", 0.25);


}


static const char* getUniqueName( char* buf, const char* basename )
{
   static unsigned int g_counter = 0;
   sprintf( buf, "%s_%X", basename, g_counter++ );
   return buf;
}

static void Vec3ToAtArray(vector<Vec3> srtPnt, AtArray* arnoldArray)
{
    for (unsigned int i = 0; i < srtPnt.size(); ++i)
    {
        AtVector tmp;
        tmp.x = float(srtPnt[i][0]);
        tmp.y = float(srtPnt[i][1]);
        tmp.z = float(srtPnt[i][2]);
        AiArraySetVec(arnoldArray, i, tmp);
    }
}

static void Vec2ToAtArray(vector<Vec3> srtPnt, AtArray* arnoldArray)
{
    for (unsigned int i = 0; i < srtPnt.size(); ++i)
    {
        AtVector2 tmp;
        tmp.x = float(srtPnt[i][0]);
        tmp.y = float(srtPnt[i][1]);
        AiArraySetVec2(arnoldArray, i, tmp);
    }
}

static void uintToAtArray(vector<int> srtInt, AtArray* arnoldArray)
{
    for (unsigned int i = 0; i < srtInt.size(); ++i)
        AiArraySetUInt(arnoldArray, i, srtInt[i]);
}


static void srt2AtArray(const SDrawCall& sDrawCall, WindData* windData)
{
    ReadSpeedTreeSrt *readSTS = windData->readSTS;

    // init AtArray-s
    AiMsgInfo("sDrawCall.m_nNumVertices: %d", sDrawCall.m_nNumVertices);
    AiMsgInfo("sDrawCall.m_nNumIndices: %d", sDrawCall.m_nNumIndices);

    int motion_key = 1;
    if (windData->MotionBlur)
        motion_key = windData->MotionKey;
    AiMsgInfo("windData->MotionBlur %s", (windData->MotionBlur? " True": "False"));
    AiMsgInfo("motion_key: %d", motion_key);

    {
    windData->procArrays[windData->id]->nsidesArray = AiArrayAllocate(sDrawCall.m_nNumIndices/3, 1, AI_TYPE_UINT);        //number of primitives and size of each primitive, "motion key":  motion blur time sample

    windData->procArrays[windData->id]->vidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);        //mesh vertices, Anticlockwise, index group
    windData->procArrays[windData->id]->vlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, motion_key, AI_TYPE_VECTOR);        //final vertex position
    windData->procArrays[windData->id]->vdefaultlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, motion_key, AI_TYPE_VECTOR);        //VERTEX_PROPERTY_POSITION, mesh vertices, real value,index ordered

    windData->procArrays[windData->id]->uvidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);        //uv vertices, the same order with vidx
    windData->procArrays[windData->id]->uvlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);    //VERTEX_PROPERTY_DIFFUSE_TEXCOORDS,uv vertices, real value, index ordered

    windData->procArrays[windData->id]->nidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices,  1, AI_TYPE_UINT);        //normal vertices, index group
    windData->procArrays[windData->id]->nlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, motion_key, AI_TYPE_VECTOR);        //VERTEX_PROPERTY_NORMAL, normal vertices, real value, index ordered

    windData->procArrays[windData->id]->tidxsArray = AiArrayAllocate(sDrawCall.m_nNumIndices,  1, AI_TYPE_UINT);        //tangent vertices
    windData->procArrays[windData->id]->tlistArray = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR);        //VERTEX_PROPERTY_TANGENT, tangent vertices, real value

    windData->procArrays[windData->id]->DIFFUSE_0_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->DIFFUSE_0_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);            //DIFFUSE_TEXCOORDS

    windData->procArrays[windData->id]->LIGHTMAP_1_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->LIGHTMAP_1_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);        //VERTEX_PROPERTY_LIGHTMAP_TEXCOORDS

    windData->procArrays[windData->id]->WIND_BRANCH_2_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->WIND_BRANCH_2_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);        //VERTEX_PROPERTY_WIND_BRANCH_DATA[0,1], save the branch level 1 data

    windData->procArrays[windData->id]->WIND_BRANCH_2_L2_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->WIND_BRANCH_2_L2_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);    //VERTEX_PROPERTY_WIND_BRANCH_DATA[2,3], save the branch level 2 data, WIND_BRANCH_DATA

    windData->procArrays[windData->id]->LOD_XY_3_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->LOD_XY_3_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);            //VERTEX_PROPERTY_LEAF_CARD_LOD_SCALAR, or VERTEX_PROPERTY_LOD_POSITION

    windData->procArrays[windData->id]->LOD_Z_4_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->LOD_Z_4_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);            //VERTEX_PROPERTY_LEAF_CARD_LOD_SCALAR, or VERTEX_PROPERTY_LOD_POSITION, or VERTEX_PROPERTY_BRANCH_SEAM_DIFFUSE

    windData->procArrays[windData->id]->Detail_UV_5_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->Detail_UV_5_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);        //VERTEX_PROPERTY_DETAIL_TEXCOORDS, or VERTEX_PROPERTY_WIND_EXTRA_DATA

    windData->procArrays[windData->id]->Seam_6_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->Seam_6_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);            //VERTEX_PROPERTY_BRANCH_SEAM_DIFFUSE,or VERTEX_PROPERTY_WIND_EXTRA_DATA

    windData->procArrays[windData->id]->Other_7_idx = AiArrayAllocate(sDrawCall.m_nNumIndices, 1, AI_TYPE_UINT);
    windData->procArrays[windData->id]->Other_7_array = AiArrayAllocate(sDrawCall.m_nNumVertices, 1, AI_TYPE_VECTOR2);            //VERTEX_PROPERTY_WIND_FLAGS, A single value, applies to leaf geometry only. If less than zero, the main leaf wind algorithm should be applied. Otherwise, apply the alternate leaf wind algorithm.
    }

    // assign values
    {
    uintToAtArray(readSTS->nsides, windData->procArrays[windData->id]->nsidesArray);
    uintToAtArray(readSTS->vidxs, windData->procArrays[windData->id]->vidxsArray);
    Vec3ToAtArray(readSTS->vertexlist, windData->procArrays[windData->id]->vdefaultlistArray);

    uintToAtArray(readSTS->uvidxsArray, windData->procArrays[windData->id]->uvidxsArray);
    Vec2ToAtArray(readSTS->uvlistArray, windData->procArrays[windData->id]->uvlistArray);

    uintToAtArray(readSTS->nidxs, windData->procArrays[windData->id]->nidxsArray);
    Vec3ToAtArray(readSTS->nlist, windData->procArrays[windData->id]->nlistArray);

    uintToAtArray(readSTS->tidxs, windData->procArrays[windData->id]->tidxsArray);
    Vec3ToAtArray(readSTS->tlist, windData->procArrays[windData->id]->tlistArray);

    uintToAtArray(readSTS->DIFFUSE_0_idx, windData->procArrays[windData->id]->DIFFUSE_0_idx);
    Vec2ToAtArray(readSTS->DIFFUSE_0_array, windData->procArrays[windData->id]->DIFFUSE_0_array);

    uintToAtArray(readSTS->LIGHTMAP_1_idx, windData->procArrays[windData->id]->LIGHTMAP_1_idx);
    Vec2ToAtArray(readSTS->LIGHTMAP_1_array, windData->procArrays[windData->id]->LIGHTMAP_1_array);

    uintToAtArray(readSTS->WIND_BRANCH_2_idx, windData->procArrays[windData->id]->WIND_BRANCH_2_idx);
    Vec2ToAtArray(readSTS->WIND_BRANCH_2_array, windData->procArrays[windData->id]->WIND_BRANCH_2_array);

    uintToAtArray(readSTS->WIND_BRANCH_2_L2_idx, windData->procArrays[windData->id]->WIND_BRANCH_2_L2_idx);
    Vec2ToAtArray(readSTS->WIND_BRANCH_2_L2_array, windData->procArrays[windData->id]->WIND_BRANCH_2_L2_array);

    uintToAtArray(readSTS->LOD_XY_3_idx, windData->procArrays[windData->id]->LOD_XY_3_idx);
    Vec2ToAtArray(readSTS->LOD_XY_3_array, windData->procArrays[windData->id]->LOD_XY_3_array);

    uintToAtArray(readSTS->LOD_Z_4_idx, windData->procArrays[windData->id]->LOD_Z_4_idx);
    Vec2ToAtArray(readSTS->LOD_Z_4_array, windData->procArrays[windData->id]->LOD_Z_4_array);

    uintToAtArray(readSTS->Detail_UV_5_idx, windData->procArrays[windData->id]->Detail_UV_5_idx);
    Vec2ToAtArray(readSTS->Detail_UV_5_array, windData->procArrays[windData->id]->Detail_UV_5_array);

    uintToAtArray(readSTS->Seam_6_idx, windData->procArrays[windData->id]->Seam_6_idx);
    Vec2ToAtArray(readSTS->Seam_6_array, windData->procArrays[windData->id]->Seam_6_array);

    uintToAtArray(readSTS->Other_7_idx, windData->procArrays[windData->id]->Other_7_idx);
    Vec2ToAtArray(readSTS->Other_7_array, windData->procArrays[windData->id]->Other_7_array);
    }

    windData->procArrays[windData->id]->GeometryType = readSTS->GeometryType;

    // calculate value
    FVertexParameters vtxParam;
    for (unsigned int i = 0; i < sDrawCall.m_nNumVertices; ++i)
    {
        AtVector vertex = AiArrayGetVec(windData->procArrays[windData->id]->vdefaultlistArray, i);
        vtxParam.WorldPosition = Vec3(vertex.x, vertex.y, vertex.z);    // P: shading point in world-space. Vec3

        vtxParam.InstanceLocalToWorld = glm::float4x4(1.0f);                              // sg->
        vtxParam.InstanceWorldToLocal = glm::float4x4(1.0f);                              // sg->Minv
        vtxParam.InstanceLocalPosition = Vec3(vertex.x, vertex.y, vertex.z);              // sg->Po

        AtVector normal = AiArrayGetVec(windData->procArrays[windData->id]->nlistArray, i);
        vtxParam.Normal = Vec3(normal.x, normal.y, normal.z);               // N: shading normal, Vec3

        AtVector tangent = AiArrayGetVec(windData->procArrays[windData->id]->tlistArray, i);
        vtxParam.Tangent = Vec3(tangent.x, tangent.y, tangent.z);           // Tangent: shading normal, Vec3

        AtVector2 diffuse_0 = AiArrayGetVec2(windData->procArrays[windData->id]->DIFFUSE_0_array, i);
        vtxParam.TexCoords[0] = Vec3(diffuse_0.x, diffuse_0.y, 0.0f);

        AtVector2 lightmap_1 = AiArrayGetVec2(windData->procArrays[windData->id]->LIGHTMAP_1_array, i);
        vtxParam.TexCoords[1] = Vec3(lightmap_1.x, lightmap_1.y, 0.0f);

        AtVector2 wind_branch_2 = AiArrayGetVec2(windData->procArrays[windData->id]->WIND_BRANCH_2_array, i);
        vtxParam.TexCoords[2] = Vec3(wind_branch_2.x, wind_branch_2.y, 0.0f);

        AtVector2 lod_xy_3 = AiArrayGetVec2(windData->procArrays[windData->id]->LOD_XY_3_array, i);
        vtxParam.TexCoords[3] = Vec3(lod_xy_3.x, lod_xy_3.y, 0.0f);

        AtVector2 lod_z_4 = AiArrayGetVec2(windData->procArrays[windData->id]->LOD_Z_4_array, i);
        vtxParam.TexCoords[4] = Vec3(lod_z_4.x, lod_z_4.y, 0.0f);

        AtVector2 detail_uv_5 = AiArrayGetVec2(windData->procArrays[windData->id]->Detail_UV_5_array, i);
        vtxParam.TexCoords[5] = Vec3(detail_uv_5.x, detail_uv_5.y, 0.0f);

        AtVector2 seam_6 = AiArrayGetVec2(windData->procArrays[windData->id]->Seam_6_array, i);
        vtxParam.TexCoords[6] = Vec3(seam_6.x, seam_6.y, 0.0f);

        AtVector2 other_7 = AiArrayGetVec2(windData->procArrays[windData->id]->Other_7_array, i);
        vtxParam.TexCoords[7] = Vec3(other_7.x, other_7.y, 0.0f);

        AtVector2 wind_branch_2_l2 = AiArrayGetVec2(windData->procArrays[windData->id]->WIND_BRANCH_2_L2_array, i);
        vtxParam.TexCoords[8] = Vec3(wind_branch_2_l2.x, wind_branch_2_l2.y, 0.0f);

        if (!windData->MotionBlur)
        {
            vec3 offset = GetSpeedTreeVertexOffset(vtxParam, windData->procArrays[windData->id]->GeometryType,  windData->WindType, windData->LODSmoothType,
                                                    0.1, windData->GlobalMotion, windData->speedtreeData);
            // reset pos
            AtVector tmp;
            tmp.x = vertex.x + float(offset.x);
            tmp.y = vertex.y + float(offset.y);
            tmp.z = vertex.z + float(offset.z);
            AiMsgInfo("fin_vertex index %d :( %f %f %f)\n", i, tmp.x, tmp.y, tmp.z);
            AiArraySetVec(windData->procArrays[windData->id]->vlistArray, i, tmp);
            {
    //        AiMsgInfo("Mesh Name: %s", windData->procArrays[windData->id]->meshName.c_str());
    //        AiMsgInfo("offset index %d :( %f %f %f)", i, offset.x, offset.y, offset.z);
    //        AiMsgInfo(" vtxParam.WorldPosition: %f %f %f \n\
    //                    vtxParam.Normal: %f %f %f \n\
    //                    vtxParam.Tangent: %f %f %f \n\
    //                    vtxParam.TexCoords[0](diffuse_0): %f %f %f\n\
    //                    vtxParam.TexCoords[1](lightmap_1): %f %f %f\n\
    //                    vtxParam.TexCoords[2](wind_branch_2): %f %f %f\n\
    //                    vtxParam.TexCoords[3](lod_xy_3): %f %f %f\n\
    //                    vtxParam.TexCoords[4](lod_z_4): %f %f %f\n\
    //                    vtxParam.TexCoords[5](detail_uv_5): %f %f %f\n\
    //                    vtxParam.TexCoords[6](seam_6): %f %f %f\n\
    //                    vtxParam.TexCoords[7](other_7): %f %f %f\n\
    //                    vtxParam.TexCoords[8](wind_branch_2_l2): %f %f %f\n\
    //                    ",
    //                    vtxParam.WorldPosition.x, vtxParam.WorldPosition.y, vtxParam.WorldPosition.z,
    //                    vtxParam.Normal.x, vtxParam.Normal.y, vtxParam.Normal.z,
    //                    vtxParam.Tangent.x, vtxParam.Tangent.y, vtxParam.Tangent.z,
    //                    vtxParam.TexCoords[0].x, vtxParam.TexCoords[0].y, vtxParam.TexCoords[0].z,
    //                    vtxParam.TexCoords[1].x, vtxParam.TexCoords[1].y, vtxParam.TexCoords[1].z,
    //                    vtxParam.TexCoords[2].x, vtxParam.TexCoords[2].y, vtxParam.TexCoords[2].z,
    //                    vtxParam.TexCoords[3].x, vtxParam.TexCoords[3].y, vtxParam.TexCoords[3].z,
    //                    vtxParam.TexCoords[4].x, vtxParam.TexCoords[4].y, vtxParam.TexCoords[4].z,
    //                    vtxParam.TexCoords[5].x, vtxParam.TexCoords[5].y, vtxParam.TexCoords[5].z,
    //                    vtxParam.TexCoords[6].x, vtxParam.TexCoords[6].y, vtxParam.TexCoords[6].z,
    //                    vtxParam.TexCoords[7].x, vtxParam.TexCoords[7].y, vtxParam.TexCoords[7].z,
    //                    vtxParam.TexCoords[8].x, vtxParam.TexCoords[8].y, vtxParam.TexCoords[8].z
    //                    );
            }

        }

        else
        {
            // pre motion key
            vec3 offset_pre_motion = GetSpeedTreeVertexOffset(vtxParam, windData->procArrays[windData->id]->GeometryType, windData->WindType, windData->LODSmoothType,
                                                                0.1, windData->GlobalMotion, windData->speedtreePreMotionData);
            AtVector tmp_pre_m;
            tmp_pre_m.x = vertex.x + float(offset_pre_motion.x);
            tmp_pre_m.y = vertex.y + float(offset_pre_motion.y);
            tmp_pre_m.z = vertex.z + float(offset_pre_motion.z);
            AiArraySetVec(windData->procArrays[windData->id]->vlistArray, i, tmp_pre_m);
            AiArraySetVec(windData->procArrays[windData->id]->nlistArray, i, normal);
            AiMsgInfo("pre mot_vertex index %d :( %f %f %f)", i, tmp_pre_m.x, tmp_pre_m.y, tmp_pre_m.z);

            //frame
            vec3 offset = GetSpeedTreeVertexOffset(vtxParam, windData->procArrays[windData->id]->GeometryType, windData->WindType, windData->LODSmoothType,
                                                    0.1, windData->GlobalMotion, windData->speedtreeData);
            AtVector tmp;
            tmp.x = vertex.x + float(offset.x);
            tmp.y = vertex.y + float(offset.y);
            tmp.z = vertex.z + float(offset.z);
            AiArraySetVec(windData->procArrays[windData->id]->vlistArray, (i + sDrawCall.m_nNumVertices), tmp);
            AiArraySetVec(windData->procArrays[windData->id]->nlistArray, (i + sDrawCall.m_nNumVertices), normal);
            AiMsgInfo("vertex index %d :( %f %f %f)", i, tmp.x, tmp.y, tmp.z);

            // post motion key
            vec3 offset_motion = GetSpeedTreeVertexOffset(vtxParam, windData->procArrays[windData->id]->GeometryType, windData->WindType, windData->LODSmoothType,
                                                            0.1, windData->GlobalMotion, windData->speedtreeMotionData);
            AtVector tmp_m;
            tmp_m.x = vertex.x + float(offset_motion.x);
            tmp_m.y = vertex.y + float(offset_motion.y);
            tmp_m.z = vertex.z + float(offset_motion.z);
            AiArraySetVec(windData->procArrays[windData->id]->vlistArray, (i + sDrawCall.m_nNumVertices * 2), tmp_m);
            AiArraySetVec(windData->procArrays[windData->id]->nlistArray, (i + sDrawCall.m_nNumVertices * 2), normal);
            AiMsgInfo("post mot_vertex index %d :( %f %f %f)", i, tmp_m.x, tmp_m.y, tmp_m.z);
            AiMsgInfo("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        }
    }
}


static AtNode* createMesh(WindData *&windData, int windType, int LODType, const AtNode* node)
{
    const SLod& sLod = *(windData->sLod);
    const SDrawCall& sDrawCall = windData->readSTS->readSDrawCall(windData->id, sLod);
//    cout << "sDrawCall.m_nNumVertices: " << sDrawCall.m_nNumVertices << endl;
    const SRenderState* renderState = windData->readSTS->readRenderState(sDrawCall);
    string meshName = windData->readSTS->setMeshName(renderState);

    char buf[512];
    const char* uni_meshName = getUniqueName(buf, meshName.c_str());
    AiMsgInfo("Mesh Name %s", uni_meshName);

    windData->readSTS->readSrtData(windData->WindType, windData->LODType, sDrawCall);

    windData->procArrays[windData->id]->meshName = uni_meshName;
    srt2AtArray(sDrawCall, windData);


    AtNode *meshNode = AiNode("polymesh", uni_meshName, node);
    windData->g_objects.push_back(meshNode);

    AiNodeSetStr(meshNode, "name", uni_meshName);
//        AiNodeSetArray(meshNode, "shader", shaderArray);
//        AiNodeSetArray(meshNode, "disp_map", dis_shaderArray);
    AiNodeSetArray(meshNode, "nsides", windData->procArrays[windData->id]->nsidesArray);        // e.g. AtByte nsides[540] = {4, 4, 4, 4, 4,..., }    4 sides of each primitive, totally 540 primitives
    AiNodeSetArray(meshNode, "vidxs", windData->procArrays[windData->id]->vidxsArray);
    AiNodeSetArray(meshNode, "vlist", windData->procArrays[windData->id]->vlistArray);
    AiNodeSetArray(meshNode, "uvidxs", windData->procArrays[windData->id]->uvidxsArray);
    AiNodeSetArray(meshNode, "uvlist", windData->procArrays[windData->id]->uvlistArray);
    AiNodeSetArray(meshNode, "nidxs", windData->procArrays[windData->id]->nidxsArray);
    AiNodeSetArray(meshNode, "nlist", windData->procArrays[windData->id]->nlistArray);
    AiNodeSetBool(meshNode, "smoothing", true);
    AiNodeSetFlt(meshNode, "disp_padding", 5.0f);

    windData->id++;

    return meshNode;
}


// static int ProceduralInit(AtNode* node, void** user_ptr)
procedural_init
{
   AiMsgInfo("++++++++++++ ST_DSO::procedural_init ++++++++++++");

   WindData *windData = new WindData();
   *user_ptr = windData;

    windData->srtFile            = AiNodeGetStr(node, "srtFile");
    windData->GlobalFrequency    = AiNodeGetFlt(node, "globalFrequency");
    windData->GustFrequency      = AiNodeGetFlt(node, "gustFrequency");
    windData->WindSpeed          = AiNodeGetFlt(node, "windSpeed");
    windData->WindDirection      = AiNodeGetVec(node, "windDirection");

    windData->WindType           = AiNodeGetInt(node, "windType");
    windData->LODType            = AiNodeGetInt(node, "LODType");
    windData->LODSmoothType      = AiNodeGetInt(node, "LODSmoothType");
//    windData->GeometryType       = AiNodeGetInt(node, "geometryType");

    windData->BillboardThreshold = 0.1;
    windData->MeshName           = AiNodeGetStr(node, "meshName");
    windData->GlobalMotion       = AiNodeGetBool(node, "globalMotion");
    windData->CalDisp            = AiNodeGetBool(node, "calDisp");
    windData->MotionBlur         = AiNodeGetBool(node, "hasMotionBlur");
    windData->MotionKey          = AiNodeGetInt(node, "motionKey");
    windData->MotionLength       = AiNodeGetFlt(node, "motionLength");
    windData->id                 = 0;

    windData->SpeedKeyFrames.push_back(AiNodeGetFlt(node, "speedKeyFrame_1"));
    windData->SpeedKeyFrames.push_back(AiNodeGetFlt(node, "speedKeyFrame_2"));
    windData->SpeedKeyFrames.push_back(AiNodeGetFlt(node, "speedKeyFrame_3"));
    windData->SpeedKeyFrames.push_back(AiNodeGetFlt(node, "speedKeyFrame_4"));

    windData->SpeedResponseTimes.push_back(AiNodeGetFlt(node, "speedResponseTime_1"));
    windData->SpeedResponseTimes.push_back(AiNodeGetFlt(node, "speedResponseTime_2"));
    windData->SpeedResponseTimes.push_back(AiNodeGetFlt(node, "speedResponseTime_3"));
    windData->SpeedResponseTimes.push_back(AiNodeGetFlt(node, "speedResponseTime_4"));

    windData->SpeedKeyValues.push_back(AiNodeGetFlt(node, "speedKeyValue_1"));
    windData->SpeedKeyValues.push_back(AiNodeGetFlt(node, "speedKeyValue_2"));
    windData->SpeedKeyValues.push_back(AiNodeGetFlt(node, "speedKeyValue_3"));
    windData->SpeedKeyValues.push_back(AiNodeGetFlt(node, "speedKeyValue_4"));

    windData->DireKeyFrames.push_back(AiNodeGetFlt(node, "direKeyFrame_1"));
    windData->DireKeyFrames.push_back(AiNodeGetFlt(node, "direKeyFrame_2"));
    windData->DireKeyFrames.push_back(AiNodeGetFlt(node, "direKeyFrame_3"));
    windData->DireKeyFrames.push_back(AiNodeGetFlt(node, "direKeyFrame_4"));

    windData->DireResponseTimes.push_back(AiNodeGetFlt(node, "direResponseTime_1"));
    windData->DireResponseTimes.push_back(AiNodeGetFlt(node, "direResponseTime_2"));
    windData->DireResponseTimes.push_back(AiNodeGetFlt(node, "direResponseTime_3"));
    windData->DireResponseTimes.push_back(AiNodeGetFlt(node, "direResponseTime_4"));

    AtVector tmp1 = AiNodeGetVec(node, "direKeyValue_1");
    windData->DireKeyValues.push_back(Vec3(tmp1.x, tmp1.y, tmp1.z));
    AtVector tmp2 = AiNodeGetVec(node, "direKeyValue_1");
    windData->DireKeyValues.push_back(Vec3(tmp2.x, tmp2.y, tmp2.z));
    AtVector tmp3 = AiNodeGetVec(node, "direKeyValue_1");
    windData->DireKeyValues.push_back(Vec3(tmp3.x, tmp3.y, tmp3.z));
    AtVector tmp4 = AiNodeGetVec(node, "direKeyValue_1");
    windData->DireKeyValues.push_back(Vec3(tmp4.x, tmp4.y, tmp4.z));

    if (windData->CalDisp)
    {
        // get current frame & fps, the global render settings are stored in the options node,(Maya)
        AtNode* options = AiUniverseGetOptions();
        windData->frameNum = AiNodeGetFlt(options, "frame");
        if (AiNodeGetFlt(options, "fps"))
            windData->fps = float(AiNodeGetFlt(options, "fps"));
        else
            windData->fps = 24.0f;

                // calculate current time
        windData->crntTime = 0.0f;
        if (windData->frameNum < 1.0f)
            windData->frameNum = 1.0f;
        windData->crntTime = windData->frameNum / windData->fps;

        AiMsgInfo("frame_num: %f\n", windData->frameNum);
        AiMsgInfo("crntTime: %f\n", windData->crntTime);
        AiMsgInfo("motionLength: %f\n", windData->MotionLength);

//        windData->readSTS = new ReadSpeedTreeSrt();
        Vec3 windDirection = Vec3(windData->WindDirection.x, windData->WindDirection.y, windData->WindDirection.z);
        //const SLod& sLod
        *(windData->sLod) = windData->readSTS->readSRT( windData->srtFile.c_str(), windData->WindSpeed, windData->GustFrequency,
                                                        windDirection, windData->GlobalMotion, windData->LODType);

        windData->readSTS->setKeyframes(windData->SpeedKeyFrames, windData->SpeedResponseTimes, windData->SpeedKeyValues,
                                        windData->DireKeyFrames, windData->DireResponseTimes, windData->DireKeyValues);
    }

    //Sets local data pointer in the node.This can be used to store custom data managed by the user.
    AiNodeSetLocalData(node, windData);
    return true;
}

// static int ProceduralCleanup(const AtNode* node, void* user_ptr)
procedural_cleanup
{
    AiMsgInfo("++++++++++++ ST_DSO::procedural_cleanup ++++++++++++");
    WindData *windData = (WindData*)user_ptr;
    delete windData;

    return true;
}

// static int ProceduralNumNodes(const AtNode* node, void* user_ptr)
procedural_num_nodes
{
    AiMsgInfo("++++++++++++ ST_DSO::procedural_num_nodes ++++++++++++");
    if (user_ptr != NULL)
    {
        WindData* args = reinterpret_cast<WindData*>( user_ptr );
        AiMsgInfo("procedural node count: %d", int(args->readSTS->meshCount));
        return (int) args->readSTS->meshCount;
    }
    else
        return NULL;
}

// static AtNode* ProceduralGetNode(const AtNode* node, void* user_ptr, int i)
procedural_get_node
{
    AiMsgInfo("++++++++++++ ST_DSO::procedural_get_node ++++++++++++");
    WindData *windData = (WindData*)user_ptr;

    if (!windData->MotionBlur)
    {
        float* windShaderValues = windData->readSTS->readWindData(windData->frameNum, 1.0, windData->fps);
        windData->speedtreeData = windData->readSTS->assignWindData(windShaderValues, windData->frameNum);
        {
    //    AiMsgInfo("windData->speedtreeData:");
    //    AiMsgInfo("windData->frameNum: %f", windData->frameNum);
    //    AiMsgInfo("WindAnimation : %f %f %f %f \n\
    //               WindVector : %f %f %f %f \n\
    //               WindGlobal : %f %f %f %f \n\
    //               WindBranch : %f %f %f %f \n\
    //               WindBranchTwitch : %f %f %f %f \n\
    //               WindBranchWhip : %f %f %f %f \n\
    //               WindBranchAnchor : %f %f %f %f \n\
    //               WindTurbulences : %f %f %f %f \n\
    //               WindLeaf1Ripple : %f %f %f %f \n\
    //               WindLeaf1Tumble : %f %f %f %f \n\
    //               WindFrondRipple : %f %f %f %f \n\
    //               WindRollingNoise : %f %f %f %f \n\
    //                ",
    //               windData->speedtreeData.WindAnimation.x, windData->speedtreeData.WindAnimation.y, windData->speedtreeData.WindAnimation.z, windData->speedtreeData.WindAnimation.w,
    //               windData->speedtreeData.WindVector.x, windData->speedtreeData.WindVector.y, windData->speedtreeData.WindVector.z, windData->speedtreeData.WindVector.w,
    //               windData->speedtreeData.WindGlobal.x, windData->speedtreeData.WindGlobal.y, windData->speedtreeData.WindGlobal.z, windData->speedtreeData.WindGlobal.w,
    //               windData->speedtreeData.WindBranch.x, windData->speedtreeData.WindBranch.y, windData->speedtreeData.WindBranch.z, windData->speedtreeData.WindBranch.w,
    //               windData->speedtreeData.WindBranchTwitch.x, windData->speedtreeData.WindBranchTwitch.y, windData->speedtreeData.WindBranchTwitch.z, windData->speedtreeData.WindBranchTwitch.w,
    //               windData->speedtreeData.WindBranchWhip.x, windData->speedtreeData.WindBranchWhip.y, windData->speedtreeData.WindBranchWhip.z, windData->speedtreeData.WindBranchWhip.w,
    //               windData->speedtreeData.WindBranchAnchor.x, windData->speedtreeData.WindBranchAnchor.y, windData->speedtreeData.WindBranchAnchor.z, windData->speedtreeData.WindBranchAnchor.w,
    //               windData->speedtreeData.WindTurbulences.x, windData->speedtreeData.WindTurbulences.y, windData->speedtreeData.WindTurbulences.z, windData->speedtreeData.WindTurbulences.w,
    //               windData->speedtreeData.WindLeaf1Ripple.x, windData->speedtreeData.WindLeaf1Ripple.y, windData->speedtreeData.WindLeaf1Ripple.z, windData->speedtreeData.WindLeaf1Ripple.w,
    //               windData->speedtreeData.WindLeaf1Tumble.x, windData->speedtreeData.WindLeaf1Tumble.y, windData->speedtreeData.WindLeaf1Tumble.z, windData->speedtreeData.WindLeaf1Tumble.w,
    //               windData->speedtreeData.WindFrondRipple.x, windData->speedtreeData.WindFrondRipple.y, windData->speedtreeData.WindFrondRipple.z, windData->speedtreeData.WindFrondRipple.w,
    //               windData->speedtreeData.WindRollingNoise.x, windData->speedtreeData.WindRollingNoise.y, windData->speedtreeData.WindRollingNoise.z, windData->speedtreeData.WindRollingNoise.w
    //               );
        }
    }

    else
    {
        // pre motion key
        windData->readSTS->readWindData((windData->frameNum - 1), 1.0, windData->fps);
        float* preMotionWindShaderValues = windData->readSTS->readMotionWindData(windData->frameNum, -(windData->MotionLength), windData->fps);
        windData->speedtreePreMotionData = windData->readSTS->assignWindData(preMotionWindShaderValues, (windData->frameNum - windData->MotionLength));
        // frame
        float* windShaderValues = windData->readSTS->readMotionWindData(windData->frameNum, 0.0, windData->fps);
        windData->speedtreeData = windData->readSTS->assignWindData(windShaderValues, windData->frameNum);
        //post motion key
        float* motionWindShaderValues = windData->readSTS->readMotionWindData(windData->frameNum, windData->MotionLength, windData->fps);
        windData->speedtreeMotionData = windData->readSTS->assignWindData(motionWindShaderValues, (windData->frameNum + windData->MotionLength));
        {
//        AiMsgInfo("windData->speedtreeMotionData:");
//        AiMsgInfo("windData->frameNum: %f", windData->frameNum);
//        AiMsgInfo("WindAnimation : %f %f %f %f \n\
//                   WindVector : %f %f %f %f \n\
//                   WindGlobal : %f %f %f %f \n\
//                   WindBranch : %f %f %f %f \n\
//                   WindBranchTwitch : %f %f %f %f \n\
//                   WindBranchWhip : %f %f %f %f \n\
//                   WindBranchAnchor : %f %f %f %f \n\
//                   WindTurbulences : %f %f %f %f \n\
//                   WindLeaf1Ripple : %f %f %f %f \n\
//                   WindLeaf1Tumble : %f %f %f %f \n\
//                   WindFrondRipple : %f %f %f %f \n\
//                   WindRollingNoise : %f %f %f %f \n\
//                    ",
//                   windData->speedtreeMotionData.WindAnimation.x, windData->speedtreeMotionData.WindAnimation.y, windData->speedtreeMotionData.WindAnimation.z, windData->speedtreeMotionData.WindAnimation.w,
//                   windData->speedtreeMotionData.WindVector.x, windData->speedtreeMotionData.WindVector.y, windData->speedtreeMotionData.WindVector.z, windData->speedtreeMotionData.WindVector.w,
//                   windData->speedtreeMotionData.WindGlobal.x, windData->speedtreeMotionData.WindGlobal.y, windData->speedtreeMotionData.WindGlobal.z, windData->speedtreeMotionData.WindGlobal.w,
//                   windData->speedtreeMotionData.WindBranch.x, windData->speedtreeMotionData.WindBranch.y, windData->speedtreeMotionData.WindBranch.z, windData->speedtreeMotionData.WindBranch.w,
//                   windData->speedtreeMotionData.WindBranchTwitch.x, windData->speedtreeMotionData.WindBranchTwitch.y, windData->speedtreeMotionData.WindBranchTwitch.z, windData->speedtreeMotionData.WindBranchTwitch.w,
//                   windData->speedtreeMotionData.WindBranchWhip.x, windData->speedtreeMotionData.WindBranchWhip.y, windData->speedtreeMotionData.WindBranchWhip.z, windData->speedtreeMotionData.WindBranchWhip.w,
//                   windData->speedtreeMotionData.WindBranchAnchor.x, windData->speedtreeMotionData.WindBranchAnchor.y, windData->speedtreeMotionData.WindBranchAnchor.z, windData->speedtreeMotionData.WindBranchAnchor.w,
//                   windData->speedtreeMotionData.WindTurbulences.x, windData->speedtreeMotionData.WindTurbulences.y, windData->speedtreeMotionData.WindTurbulences.z, windData->speedtreeMotionData.WindTurbulences.w,
//                   windData->speedtreeMotionData.WindLeaf1Ripple.x, windData->speedtreeMotionData.WindLeaf1Ripple.y, windData->speedtreeMotionData.WindLeaf1Ripple.z, windData->speedtreeMotionData.WindLeaf1Ripple.w,
//                   windData->speedtreeMotionData.WindLeaf1Tumble.x, windData->speedtreeMotionData.WindLeaf1Tumble.y, windData->speedtreeMotionData.WindLeaf1Tumble.z, windData->speedtreeMotionData.WindLeaf1Tumble.w,
//                   windData->speedtreeMotionData.WindFrondRipple.x, windData->speedtreeMotionData.WindFrondRipple.y, windData->speedtreeMotionData.WindFrondRipple.z, windData->speedtreeMotionData.WindFrondRipple.w,
//                   windData->speedtreeMotionData.WindRollingNoise.x, windData->speedtreeMotionData.WindRollingNoise.y, windData->speedtreeMotionData.WindRollingNoise.z, windData->speedtreeMotionData.WindRollingNoise.w
//                   );
        }
    }

    AtNode* mesh_node = createMesh(windData, windData->WindType, windData->LODType, node);

    return mesh_node;

}

node_loader
{
   if (i>0)
      return false;

   node->methods      = ST_DSO;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "ST_DSO";
   node->node_type    = AI_NODE_SHAPE_PROCEDURAL;
   strcpy(node->version, AI_VERSION);

   return true;
}



