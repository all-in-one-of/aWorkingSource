#pragma once

#include <cmath>
#include <ai.h>
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>

#include <maya/MTypes.h>

#include <vector>
#include <map>
#include <string>

#include <XgRenderAPI.h>

#ifdef _WIN32
#if MAYA_API_VERSION < 201600
#undef min
#undef max
#endif
#endif
#include <XgRenderAPIUtils.h>

typedef AtNode* (*InitArnoldNodeFnc)(const char *);

extern bool s_bCleanDescriptionCache;


using namespace XGenRenderAPI;

// Forward Decl
struct AtNode;

struct XgMergedData;
class XgMutex;

namespace XGenArnold
{  
   class Procedural : public ProceduralCallbacks
   {
   public:
      Procedural();
      virtual ~Procedural();

      // Arnold Entry points.
      int Init(AtNode* node);
      int Cleanup();
      int NumNodes();
      AtNode* GetNode(int i);
      void SetInitCallback(InitArnoldNodeFnc cb) {m_initArnoldFunc = cb;}
      // XGenRenderAPI::ProceduralCallbacks
      virtual void flush(  const char* in_geom, PrimitiveCache* in_cache );
      virtual void log( const char* in_str ){}

      virtual bool get( EBoolAttribute ) const;
      virtual const char* get( EStringAttribute ) const;
      virtual float get( EFloatAttribute ) const;
      virtual const float* get( EFloatArrayAttribute ) const;
      virtual unsigned int getSize( EFloatArrayAttribute ) const;

      virtual const char* getOverride( const char* in_name ) const;
      virtual bool getArchiveBoundingBox( const char* in_filename, bbox& out_bbox ) const;
      virtual void getTransform( float in_time, mat44& out_mat ) const;
      
      static XgMutex* m_mutex;

   private:
      // These 4 methods are protected by the global mutex.
      bool nextFace( bbox& b, unsigned int& f );
      bool initPatchRenderer( const char* in_params );
      bool initFaceRenderer( Procedural* pProc, unsigned int f );
      bool render();

      void flushSplines( const char *geomName, PrimitiveCache* pc );
      void flushCards( const char *geomName, PrimitiveCache* pc );
      void flushSpheres( const char *geomName, PrimitiveCache* pc );
      void flushArchives( const char *geomName, PrimitiveCache* pc );

      bool getString( AtNode* in_node, const char* in_name, const char*& out_value, bool in_user=false ) const;
      bool getFloat( AtNode* in_node, const char* in_name, float& out_value, bool in_user=false  ) const;

      unsigned int getArraySize( AtNode* in_node, const char* in_name, int in_eType, bool in_user=false  ) const;
      bool getFloatArray( AtNode* in_node, const char* in_name, const float*& out_value, bool in_user=false  ) const;
      bool getMatrixArray( AtNode* in_node, const char* in_name, const AtMatrix*& out_value, bool in_user=false  ) const;

      void pushCustomParams( AtNode* in_node, PrimitiveCache* pc, unsigned int cacheCount = 0 );

      static void convertMatrix( const AtMatrix in_mat, mat44& out_mat );

      const char* getUniqueName( char* buf, const char* basename );

      AtNode* getArchiveProceduralNode( const char* file_name, const char* instance_name, /*const bbox& arcbox, */double frame );

      AtNode* m_node;
      AtNode* m_node_face;
      AtNode* m_options;
      AtNode* m_camera;
      AtNode* m_sphere;
      AtArray* m_shaders;

      PatchRenderer* m_patch;
      std::vector<FaceRenderer*> m_faces;

      std::vector<AtNode*> m_nodes;
      XgMergedData *m_merged_data;

#ifdef XGEN_RENDER_API_PARALLEL
      ParallelRenderer* m_parallel;
#endif

      mutable std::map<std::string, bbox> m_bboxes;
      InitArnoldNodeFnc m_initArnoldFunc;
   };

   class ProceduralWrapper
   {
   public:
      ProceduralWrapper( Procedural* p, bool cleanup=true )
      {
         m_proc = p;
         m_cleanup = cleanup;
      }
      ~ProceduralWrapper()
      {
         // FIXME I'm seeing some commented code mentionning a procedural created
         // so cleanup to top-level patch data. But I don't understand if this situation
         // is still valid or not. We need to delete the patch at some point, until now
         // it wasn't cleaned at all. This prevented the refresh when the xgen_procedural DSO 
         // is used in a standin (#2714)
         if( m_proc/*  && m_cleanup */)
            delete m_proc;

         m_proc = NULL;
      }

      int Init(AtNode* node) { return m_proc->Init(node); }
      int Cleanup() { return m_proc->Cleanup(); }
      int NumNodes() { return m_cleanup ? 0 : m_proc->NumNodes(); }
      AtNode* GetNode(int i) { return m_cleanup ? NULL : m_proc->GetNode(i); }
      void SetInitCallback(InitArnoldNodeFnc cb) {m_proc->SetInitCallback(cb);}



   private:
      Procedural* m_proc;
      bool m_cleanup;
   };

   inline int getHash(AtNode* node)
   {
      return (int)AiNodeGetStr(node, AtString("name")).hash();
   }
};

