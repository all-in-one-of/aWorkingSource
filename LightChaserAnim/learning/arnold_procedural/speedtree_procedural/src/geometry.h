#pragma once

#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "constants.h"

using namespace std;

// Procedural parameters
struct SpeedTreeData
{
   AtString filename;
   int threads;
   float offset;
   AtArray *motionSamples;

   int id;
   int num;
};

AtNode* create_example_cube(const AtNode* parent, SpeedTreeData *data)
{
   AtNode *polymesh = AiNode( "polymesh", "SpeedTreeProceduralMesh", parent);
   AiNodeSetBool( polymesh, "opaque", true );
   AiNodeSetBool( polymesh, "smoothing", false );
   AtMatrix mat = AiM4Identity();
   AtVector translation = AtVector(5*data->id,0,0);
   mat = AiM4Translation(translation);
   AiNodeSetMatrix ( polymesh, "matrix", mat );

   static const int vertices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7, 6, 7, 0, 0, 7, 1, 1, 7, 3, 3, 7, 5, 6, 0, 4, 4, 0, 2};
   vector<uint8_t> vertices_list (vertices, vertices + 
      sizeof(vertices) / sizeof(vertices[0]) );

   static const float point[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> point_list (point, point + 
      sizeof(point) / sizeof(point[0]) );

   static const float normal[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> normal_list (normal, normal + 
      sizeof(normal) / sizeof(normal[0]) );

   unsigned int vertexCount = point_list.size()/3;
   unsigned int polyCount = vertices_list.size()/3;
   // create array to hold poly data
   AtArray *pointArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );
   AtArray *nSidesArray = AiArrayAllocate( (uint32_t) polyCount, 1, AI_TYPE_UINT );
   AtArray *vertexArray = AiArrayAllocate( (uint32_t) polyCount * 3, 1, AI_TYPE_UINT );

   AtArray *normalArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );

   for ( unsigned int i = 0; i < vertexCount; i++ ) {
      // set position data
      int index = i * 3;
      AtVector tmpPoint = AtVector ( point[index], point[index + 1], point[index + 2] );
      AiArraySetVec( pointArray, i, tmpPoint );

      AtVector tmpNormal = AtVector ( normal[index], normal[index + 1], normal[index + 2] );
      AiArraySetVec( normalArray, i, tmpNormal );
   }
   // get triangle data
   for ( unsigned int i = 0; i < polyCount; i++ ) {
     // set position data
     int index = i * 3;
     AiArraySetUInt( nSidesArray, i, 3 );
     AiArraySetUInt( vertexArray, index, vertices[index] );
     AiArraySetUInt( vertexArray, index + 1, vertices[index + 1] );
     AiArraySetUInt( vertexArray, index + 2, vertices[index + 2] );
   }

   AiNodeSetArray( polymesh, "vlist", pointArray );
   AiNodeSetArray( polymesh, "nsides", nSidesArray );
   AiNodeSetArray( polymesh, "vidxs", vertexArray );

   AiNodeSetArray( polymesh, "nlist", normalArray );
   AiNodeSetArray( polymesh, "nidxs", AiArrayCopy(vertexArray) );

   AiNodeDeclare( polymesh, AtString(SPEEDTREE_SHADER), "constant INT");
   AiNodeSetInt( polymesh, AtString(SPEEDTREE_SHADER),  data->id);
   data->id ++;

   return polymesh;
}

AtNode* create_triangle_cube(const AtNode* parent, SpeedTreeData *data)
{
   AtNode *polymesh = AiNode( "polymesh", "SpeedTreeProceduralMesh", parent);
   AiNodeSetBool( polymesh, "opaque", true );
   AiNodeSetBool( polymesh, "smoothing", false );
   AtMatrix mat = AiM4Identity();
   AtVector translation = AtVector(5*data->id,0,0);
   mat = AiM4Translation(translation);
   AiNodeSetMatrix ( polymesh, "matrix", mat );

   static const int vertices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7, 6, 7, 0, 0, 7, 1, 1, 7, 3, 3, 7, 5, 6, 0, 4, 4, 0, 2};
   vector<uint8_t> vertices_list (vertices, vertices + 
      sizeof(vertices) / sizeof(vertices[0]) );

   static const float point[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> point_list (point, point + 
      sizeof(point) / sizeof(point[0]) );

   static const float normal[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> normal_list (normal, normal + 
      sizeof(normal) / sizeof(normal[0]) );

   unsigned int vertexCount = point_list.size()/3;
   unsigned int polyCount = vertices_list.size()/3;
   // create array to hold poly data
   AtArray *pointArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );
   AtArray *nSidesArray = AiArrayAllocate( (uint32_t) polyCount, 1, AI_TYPE_UINT );
   AtArray *vertexArray = AiArrayAllocate( (uint32_t) polyCount * 3, 1, AI_TYPE_UINT );

   AtArray *normalArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );

   for ( unsigned int i = 0; i < vertexCount; i++ ) {
      // set position data
      int index = i * 3;
      AtVector tmpPoint = AtVector ( point[index], point[index + 1], point[index + 2] );
      AiArraySetVec( pointArray, i, tmpPoint );

      AtVector tmpNormal = AtVector ( normal[index], normal[index + 1], normal[index + 2] );
      AiArraySetVec( normalArray, i, tmpNormal );
   }
   // get triangle data
   for ( unsigned int i = 0; i < polyCount; i++ ) {
     // set position data
     int index = i * 3;
     AiArraySetUInt( nSidesArray, i, 3 );
     AiArraySetUInt( vertexArray, index, vertices[index] );
     AiArraySetUInt( vertexArray, index + 1, vertices[index + 1] );
     AiArraySetUInt( vertexArray, index + 2, vertices[index + 2] );
   }

   AiNodeSetArray( polymesh, "vlist", pointArray );
   AiNodeSetArray( polymesh, "nsides", nSidesArray );
   AiNodeSetArray( polymesh, "vidxs", vertexArray );

   AiNodeSetArray( polymesh, "nlist", normalArray );
   AiNodeSetArray( polymesh, "nidxs", AiArrayCopy(vertexArray) );

   AiNodeDeclare( polymesh, AtString(SPEEDTREE_SHADER), "constant INT");
   AiNodeSetInt( polymesh, AtString(SPEEDTREE_SHADER),  data->id);
   data->id ++;

   return polymesh;
}

AtNode* create_quadrangle_cube(const AtNode* parent, SpeedTreeData *data)
{
   AtNode *polymesh = AiNode( "polymesh", "SpeedTreeProceduralMesh", parent);
   AiNodeSetBool( polymesh, "opaque", true );
   AiNodeSetBool( polymesh, "smoothing", false );
   AtMatrix mat = AiM4Identity();
   AtVector translation = AtVector(5*data->id,0,0);
   mat = AiM4Translation(translation);
   AiNodeSetMatrix ( polymesh, "matrix", mat );

   static const int vertices[] = {0, 1, 3, 2, 2, 3, 5, 4, 4, 5, 7, 6, 6, 7, 1, 0, 1, 7, 5, 3, 6, 0, 2, 4};
   vector<uint8_t> vertices_list (vertices, vertices + 
      sizeof(vertices) / sizeof(vertices[0]) );

   static const float point[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> point_list (point, point + 
      sizeof(point) / sizeof(point[0]) );

   static const float normal[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5};
   vector<float> normal_list (normal, normal + 
      sizeof(normal) / sizeof(normal[0]) );

   unsigned int vertexCount = point_list.size()/3;
   // create array to hold poly data
   AtArray *pointArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );
   AtArray *normalArray = AiArrayAllocate( (uint32_t) vertexCount, 1, AI_TYPE_VECTOR );

   unsigned int polyCount = vertices_list.size()/4;
   AtArray *nSidesArray = AiArrayAllocate( (uint32_t) polyCount, 1, AI_TYPE_UINT );
   AtArray *vertexArray = AiArrayAllocate( (uint32_t) polyCount * 4, 1, AI_TYPE_UINT );


   for ( unsigned int i = 0; i < vertexCount; i++ ) {
      // set position data
      int index = i * 3;
      AtVector tmpPoint = AtVector ( point[index], point[index + 1], point[index + 2] );
      AiArraySetVec( pointArray, i, tmpPoint );

      AtVector tmpNormal = AtVector ( normal[index], normal[index + 1], normal[index + 2] );
      AiArraySetVec( normalArray, i, tmpNormal );
   }
   // get triangle data
   for ( unsigned int i = 0; i < polyCount; i++ ) {
     // set position data
     int index = i * 4;
     AiArraySetUInt( nSidesArray, i, 4 );
     AiArraySetUInt( vertexArray, index, vertices[index] );
     AiArraySetUInt( vertexArray, index + 1, vertices[index + 1] );
     AiArraySetUInt( vertexArray, index + 2, vertices[index + 2] );
     AiArraySetUInt( vertexArray, index + 3, vertices[index + 3] );
   }

   AiNodeSetArray( polymesh, "vlist", pointArray );
   AiNodeSetArray( polymesh, "nsides", nSidesArray );
   AiNodeSetArray( polymesh, "vidxs", vertexArray );

   AiNodeSetArray( polymesh, "nlist", normalArray );
   AiNodeSetArray( polymesh, "nidxs", AiArrayCopy(vertexArray) );

   AiNodeDeclare( polymesh, AtString(SPEEDTREE_SHADER), "constant INT");
   AiNodeSetInt( polymesh, AtString(SPEEDTREE_SHADER),  data->id);
   data->id ++;

   return polymesh;
}