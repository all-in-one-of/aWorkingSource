#include "XgArnoldExpand.h"

#define _USE_MATH_DEFINES
#include <cmath>
//#define M_PI 3.1415926535897932384626433832795

#include <ai.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <iostream>
#include <fstream>
#include <fcntl.h>

#include "XgExternalAPI.h"

#include <XgRenderAPI.h>
#include <XgRenderAPIUtils.h>


using namespace XGenRenderAPI;

using namespace XGenRenderAPI::Utils;
using namespace XGenArnold;
using namespace std;


// For maya > 2015 we need to call xgapi::initConfig
#if MAYA_API_VERSION > 201500

// We have linking problems in OSX for versions < 2017
#ifdef _DARWIN
#if MAYA_API_VERSION >= 201700
#define MTOA_XG_INIT_CONFIG 1
#endif
#else // not DARWIN
#define MTOA_XG_INIT_CONFIG 1
#endif

#endif 


bool s_bCleanDescriptionCache = true;

struct XgMergedData
{
   struct Data
   {
      std::string name;
      std::vector<AtArray*> arrays;
   };
   std::vector<Data> data;

   XgMergedData() {}

   ~XgMergedData()
   {
      for (size_t i = 0; i < data.size(); i++)
         for (size_t j = 0; j < data[i].arrays.size(); j++)
            AiArrayDestroy(data[i].arrays[j]);
   }

   void add_array(const char *name, AtArray *array)
   {
      for (size_t i = 0; i < data.size(); i++)
         if (data[i].name == name)
            return data[i].arrays.push_back(array);

      Data udata;
      udata.name = name;
      udata.arrays.push_back(array);
      data.push_back(udata);
   }

   void merge_into_node(AtNode *node)
   {
      for (size_t i = 0; i < data.size(); i++)
         merge_arrays(node, data[i].name.c_str(), data[i].arrays);
   }

   void merge_arrays(AtNode *node, const char *name, std::vector<AtArray*>& arrays)
   {
      // count total number of elements
      unsigned nelements = 0;
      for (size_t i = 0; i < arrays.size(); i++)
      {
         AtArray *array = arrays[i];
         nelements += AiArrayGetNumElements(array);
      }

      if (nelements == 0)
         return;

      // create new array
      AtArray *first_array = arrays[0];
      AtArray *concat_array = AiArrayAllocate(nelements, AiArrayGetNumKeys(first_array), AiArrayGetType(first_array));
      // FIXME Arnold5 make sure we're doing the right thing
      char *concat_array_data = (char *)AiArrayMap(concat_array);
      if (concat_array_data == NULL)
         return;

      size_t key_size = AiArrayGetKeySize(concat_array);
      size_t elements_offset = 0;

      for (size_t i = 0; i < arrays.size(); i++)
      {
         AtArray *array = arrays[i];
         char *array_data = (char *)AiArrayMap(array);
         if (array_data == NULL)
            continue; // shouldn't happen

         // copy array data into concatenated array data
         size_t array_key_size = AiArrayGetKeySize(array);
         uint8_t array_num_keys = AiArrayGetNumKeys(array);

         for (int k = 0; k < array_num_keys; k++)
         {
            memcpy(concat_array_data + k *key_size + elements_offset, array_data + k * array_key_size, array_key_size);
         }
         elements_offset += array_key_size;
      }

      // Now need to unmap all these arrays
      for (size_t i = 0; i < arrays.size(); i++)
         AiArrayUnmap(arrays[i]);

      // set new array in first node, no need to unmap it
      AiNodeSetArray(node, name, concat_array);
   }
};


class XgMutex
{
public:
   XgMutex()
   {
      AiCritSecInitRecursive(&mMutex);
   }
   ~XgMutex()
   {
      AiCritSecClose(&mMutex);
   }
   void enter()
   {
      AiCritSecEnter(&mMutex);
   }
   void leave()
   {
      AiCritSecLeave(&mMutex);
   }
   AtCritSec mMutex;
};

XgMutex* Procedural::m_mutex = new XgMutex();

#define MAX_NAME_SIZE 65535

#define XGDebug( x ) {}
#define XGError( x ) {}
#define XGDebugLevel 4

#define XGRenderAPIError XGError
#define XGRenderAPIWarning XGWarning
#define XGRenderAPIStats XGStats
#define XGRenderAPIDebug XGDebug




namespace XGenArnold
{


Procedural::Procedural()
: m_node( NULL )
, m_node_face( NULL )
, m_options( NULL )
, m_camera( NULL )
, m_sphere( NULL )
, m_shaders( NULL )
, m_patch( NULL )
, m_merged_data ( NULL )
, m_initArnoldFunc( NULL )
#ifdef XGEN_RENDER_API_PARALLEL
, m_parallel ( NULL )
#endif
{
   //m_mutex = new XgMutex();
}

Procedural::~Procedural()
{
   if( m_patch )
   {
      delete m_patch;
      //delete m_mutex;
      m_patch = NULL;
      //m_mutex = NULL;
   }

#ifdef XGEN_RENDER_API_PARALLEL
    delete m_parallel;
#endif
}

bool Procedural::nextFace( bbox& b, unsigned int& f )
{
   m_mutex->enter();
   bool result = false;
   if(m_patch)
      result = m_patch->nextFace( b, f );
   m_mutex->leave();
   return result;
}

bool Procedural::initPatchRenderer( const char* in_params )
{
   m_mutex->enter();
   m_patch = PatchRenderer::init( (ProceduralCallbacks*)this, in_params );
   /*const char* params = "-debug 1 -warning 1 -stats 1  -shutter 0.0 -file ${XGEN_ROOT}/../scen/untitled__collection9.xgen -palette collection9 -geom ${XGEN_ROOT}/../scen/untitled__collection9.abc -patch pSphere1  -description description10 -fps 24.0 -frame 1.000000";
   m_patch = PatchRenderer::init( (ProceduralCallbacks*)this, params );*/
   bool result = (m_patch!=NULL);
#ifdef XGEN_RENDER_API_PARALLEL
   if (result && AiNodeGetBool( m_node, "xgen_multithreading" ))
   {
       m_parallel = ParallelRenderer::init( m_patch );
   }
#endif
   m_mutex->leave();
   return result;
}

bool Procedural::initFaceRenderer( Procedural* pProc, unsigned int f )
{
   m_mutex->enter();
   FaceRenderer* tmp_face = FaceRenderer::init( m_patch, f, pProc );
   bool result = tmp_face!=NULL;
   if(result)
      pProc->m_faces.push_back( tmp_face );
   m_mutex->leave();
   return result;
}

bool Procedural::render()
{
   m_mutex->enter();
#ifdef XGEN_RENDER_API_PARALLEL
   if (m_parallel && m_parallel->canRunInParallel())
   {
       std::vector<FaceRenderer*>::iterator it;
       for (it = m_faces.begin(); it != m_faces.end(); ++it)
       {
           m_parallel->enqueue(*it);
       }
       m_parallel->spawnAndWait();
   }
   else
   {
       // Fallback
       std::vector<FaceRenderer*>::iterator it;
       for (it = m_faces.begin(); it != m_faces.end(); ++it)
       {
          (*it)->render();
       }
   }
#else
   for (std::vector<FaceRenderer*>::iterator it = m_faces.begin() ; it != m_faces.end(); ++it)
      (*it)->render();
#endif
   m_mutex->leave();
   return true;
}
/*
const char* Procedural::getUniqueName( char* buf, const char* basename )
{
   static unsigned int g_counter = 0;
   sprintf( buf, "%s__%X", basename, g_counter++ );
   return buf;
}*/

int Procedural::Init(AtNode* node)
{
   // Temporary fix to be able to render the clumping modifier outside Maya
#ifdef WIN32
   _fmode = _O_BINARY;
#endif

   AtString parameters = AiNodeGetStr( node, "data" );
   
   m_options = AiUniverseGetOptions();
   m_camera = AiUniverseGetCamera();
   

#ifdef MTOA_XG_INIT_CONFIG
   char* xgenConfigPath = getenv("XGEN_CONFIG_PATH");
   if(xgenConfigPath != NULL)
      xgapi::initConfig(string(xgenConfigPath));
#endif
      
   static AtString cleanupStr("cleanup");
   // Cleanup Init
   if( parameters == cleanupStr)
   {
      // Noop!
   }

   // Patch Init
   else if( m_patch==NULL && m_faces.size()==0 )
   {
      m_node = node;
      m_shaders = AiNodeGetArray( m_node, "xgen_shader" );

      string strParentName = AiNodeGetName( m_node );
      
      // Create a sphere shape node
      {
         m_sphere = AiNode("sphere");
         string nodeName = strParentName + string("_sphere_shape");
         AiNodeSetStr( m_sphere, "name", nodeName.c_str());
         AiNodeSetFlt( m_sphere, "radius", 0.5f );
         AiNodeSetVec( m_sphere, "center", 0.0f, 0.0f, 0.0f );
         AiNodeSetByte( m_sphere, "visibility", 0 );
         AiNodeSetFlt(m_sphere, "motion_start", AiNodeGetFlt(m_node, "motion_start"));
         AiNodeSetFlt(m_sphere, "motion_end", AiNodeGetFlt(m_node, "motion_end"));

         m_nodes.push_back( m_sphere );
      }

      // This is where we link our callbacks to the PatchRenderer.
      initPatchRenderer( parameters.c_str() );

      bbox b = {AI_BIG, -AI_BIG, AI_BIG, -AI_BIG, AI_BIG, -AI_BIG};
      bbox total = {AI_BIG, -AI_BIG, AI_BIG, -AI_BIG, AI_BIG, -AI_BIG};
      unsigned int f = -1;
      //while( nextFace( b, f ) )
      {
         // Skip camera culled bounding boxes.
         //if( isEmpty( b ) )
         //   continue;

         //string strFaceProcName = strParentName + string("_face");// + itoa( f );

         /*Procedural* pProc = new Procedural();
         pProc->m_node = m_node;
         pProc->m_sphere = m_sphere;
         pProc->m_shaders = m_shaders;*/

   
         while( nextFace( b, f ) )
         {
            // Skip camera culled bounding boxes.
            if( isEmpty( b ) )
               continue;

            total.xmin = total.xmin < b.xmin ? total.xmin : b.xmin;
            total.ymin = total.ymin < b.ymin ? total.ymin : b.ymin;
            total.zmin = total.zmin < b.zmin ? total.zmin : b.zmin;

            total.xmax = total.xmax > b.xmax ? total.xmax : b.xmax;
            total.ymax = total.ymax > b.ymax ? total.ymax : b.ymax;
            total.zmax = total.zmax > b.zmax ? total.zmax : b.zmax;

            initFaceRenderer( this, f );
         }

         m_node_face = m_node;
         // Clone ourself, this will help us keep all the user parameters.
         // We could also provide a back pointer to the original top level node.
         /*AtNode* nodeFaceProc = AiNode( "procedural" );
         pProc->m_node_face = nodeFaceProc;

         // Change name, dso, userdata, and bounding box
         AiNodeSetStr( nodeFaceProc, "name", getUniqueName(buf,strFaceProcName.c_str()) );
         AiNodeSetStr( nodeFaceProc, "dso", strParentDso.c_str() );
         AiNodeSetPtr( nodeFaceProc, "userptr", (void*)new ProceduralWrapper( pProc, false ) );
         AiNodeSetPnt( nodeFaceProc, "min", (float)total.xmin, (float)total.ymin, (float)total.zmin );
         AiNodeSetPnt( nodeFaceProc, "max", (float)total.xmax, (float)total.ymax, (float)total.zmax );

         m_nodes.push_back( nodeFaceProc );*/
      }

      // Add a cleanup procedural that will be responsible to cleanup the Top Level Patch data.
      /*{
         AtNode* nodeCleanupProc = AiNode( "procedural" );
         string strCleanupProcName =  strParentName + "_cleanup";

         AiNodeSetStr( nodeCleanupProc, "name", getUniqueName(buf,strCleanupProcName.c_str()) );
         AiNodeSetStr( nodeCleanupProc, "dso", strParentDso.c_str() );
         AiNodeSetStr( nodeCleanupProc, "data", "cleanup" );
         AiNodeSetPtr( nodeCleanupProc, "userptr", (void*)new ProceduralWrapper( this, true ) );

         AtPoint minParentBBox = AiNodeGetPnt( m_node, "min" );
         AtPoint maxParentBBox = AiNodeGetPnt( m_node, "max" );

         AiNodeSetPnt( nodeCleanupProc, "min", minParentBBox.x, minParentBBox.y, minParentBBox.z );
         AiNodeSetPnt( nodeCleanupProc, "max", maxParentBBox.x, maxParentBBox.y, maxParentBBox.z );

         m_nodes.push_back( nodeCleanupProc );
      }*/
    }

    // Face Init
    if( m_faces.size()!=0 )
    {
       render();
    }

    if( m_merged_data )
    {
       m_merged_data->merge_into_node(m_nodes[1]);
       delete m_merged_data;
       m_merged_data = NULL;
    }

   return 1;
}

int Procedural::Cleanup()
{
   m_nodes.clear();
   m_node = m_node_face = m_options = m_sphere = NULL; // Don't delete.

   if( m_faces.size()!=0 )
   {
      for (std::vector<FaceRenderer*>::iterator it = m_faces.begin() ; it != m_faces.end(); ++it)
         delete *it;
      m_faces.clear();
   }
   return 1;
}

int Procedural::NumNodes()
{
   return (int)m_nodes.size();
}

AtNode* Procedural::GetNode(int i)
{
   return m_nodes[i];
}

bool Procedural::getFloat( AtNode* in_node, const char* in_name, float& out_value, bool in_user  ) const
{
   if( in_user )
   {
      const AtUserParamEntry* upe = AiNodeLookUpUserParameter( in_node, in_name );
      if( upe && AiUserParamGetType(upe)==AI_TYPE_FLOAT )
      {
         out_value = AiNodeGetFlt( in_node, in_name );
         return true;
      }
   }
   else
   {
      // We are assuming the parameter exists
      out_value = AiNodeGetFlt( in_node, in_name );
      return true;
   }
   return false;
}

bool Procedural::getString( AtNode* in_node, const char* in_name, const char*& out_value, bool in_user  ) const
{
   if( in_user )
   {
      const AtUserParamEntry* upe = AiNodeLookUpUserParameter( in_node, in_name );
      if( upe && AiUserParamGetType(upe)==AI_TYPE_STRING )
      {
         out_value = AiNodeGetStr( in_node, in_name );
         return true;
      }
   }
   else
   {
      // We are assuming the parameter exists
      out_value = AiNodeGetStr( in_node, in_name );
      return true;
   }
   return false;
}

bool Procedural::getFloatArray( AtNode* in_node, const char* in_name, const float*& out_value, bool in_user  ) const
{
   bool bExists = !in_user;
   if( in_user )
   {
      const AtUserParamEntry* upe = AiNodeLookUpUserParameter(in_node, in_name );
      bExists = ( upe && AiUserParamGetType(upe)==AI_TYPE_ARRAY && AiUserParamGetArrayType(upe)==AI_TYPE_FLOAT );
   }

   if( bExists )
   {
      AtArray* a = AiNodeGetArray( in_node, in_name );
      if( a )
      {
        // FIXME Arnold5 this is not the right thing to do, we need to unpmap this after the whole function was called
         out_value = ((float*)AiArrayMap(a));//
         return true;
      }
   }

   return false;
}

bool Procedural::getMatrixArray( AtNode* in_node, const char* in_name, const AtMatrix*& out_value, bool in_user  ) const
{
   bool bExists = !in_user;
   if( in_user )
   {
      const AtUserParamEntry* upe = AiNodeLookUpUserParameter(in_node, in_name );
      bExists = ( upe && AiUserParamGetType(upe)==AI_TYPE_ARRAY && AiUserParamGetArrayType(upe)==AI_TYPE_MATRIX );
   }

   if( bExists )
   {
      AtArray* a = AiNodeGetArray( in_node, in_name );
      if( a )
      {
         // FIXME Arnold5 this is not the right thing to do, we need to unpmap this after the whole function was called
         out_value = (const AtMatrix*)AiArrayMap(a);
         return true;
      }
   }

   return false;
}

unsigned int Procedural::getArraySize( AtNode* in_node, const char* in_name, int in_eType, bool in_user  ) const
{
   if( in_user )
   {
      const AtUserParamEntry* upe = AiNodeLookUpUserParameter(in_node, in_name );
      if( upe && AiUserParamGetType(upe)==AI_TYPE_ARRAY && AiUserParamGetArrayType(upe)==in_eType )
      {
         AtArray* a = AiNodeGetArray( in_node, in_name );
         if( a )
            return AiArrayGetNumElements(a);
      }
   }
   else
   {
      AtArray* a = AiNodeGetArray( in_node, in_name );
      if( a )
         return AiArrayGetNumElements(a);
   }

   return 0;
}

const char* Procedural::get( EStringAttribute in_attr ) const
{
   //static string result;
   const char* cstr = NULL;
   if( in_attr == BypassFXModulesAfterBGM )
   {
       if( getString( m_node, "xgen_bypassFXModulesAfterBGM", cstr, true  ) )
       {
          return cstr;
       }
   }
   else if( in_attr == CacheDir )
   {
      string result = "xgenCache/";

      if( getString( m_node, "xgenCache", cstr, true  ) )
      {
         string tmp = cstr;
         if( tmp.size() )
         {
            result = tmp;
         }
      }
      return result.c_str();
   }
   else if( in_attr == Off )
   {
      if( getString( m_node, "xgen_OFF", cstr, true  ) )
      {
         if( stob( cstr ) )
         {
            //XGRenderAPIDebug( /*msg::C|msg::RENDERER|2,*/ "Ribbon disabled XGen patch " + _patch->name() + " from rendering." );
            return "xgen_OFF";
         }
      }
   }
   else if( in_attr == Generator )
   {
      if( getString( m_options, "generator", cstr, true  ) )
         return cstr;
   }
   else if( in_attr == RenderCam )
   {
      if( getString( m_node, "irRenderCam", cstr, true  ) )
         return cstr;
   }
   else if( in_attr == RenderCamFOV )
   {
      if( getString( m_node, "irRenderCamFOV", cstr, true  ) )
         return cstr;
   }
   else if( in_attr == RenderCamXform )
   {
      if( getString( m_node, "irRenderCamXform", cstr, true  ) )
         return cstr;
   }
   else if( in_attr == RenderCamRatio )
   {
      if( getString( m_node, "irRenderCamRatio", cstr, true  ) )
         return cstr;
   }
   else if( in_attr == RenderMethod )
   {
      if( getString( m_node, "xgen_renderMethod", cstr, true  ) )
         return cstr;
   }
   else if( in_attr == Phase )
   {
      if( getString( m_options, "phase", cstr, true  ) )
         return cstr;
   }
   return "";
}

bool Procedural::get( EBoolAttribute in_attr ) const
{
    if( in_attr == ClearDescriptionCache )
    {
       bool ret = s_bCleanDescriptionCache;
       s_bCleanDescriptionCache = false;
       return ret;
    }

    return false;
}

float Procedural::get( EFloatAttribute in_attr ) const
{
    float result=0.f;

    if( in_attr == ShadowMotionBlur )
    {
       if( getFloat( m_node, "shadowMotionBlur", result, true ) )
         return result;
    }
    else if( in_attr == ShutterOffset )
    {
       float shutter_start = 0.0f;
       float shutter_end = 0.f;

       getFloat( m_camera, "shutter_start", shutter_start, false );
       getFloat( m_camera, "shutter_end", shutter_end, false );

       float shutter_offset = shutter_start + 0.5f*(shutter_end-shutter_start);
       return shutter_offset;
    }

   return 0.f;
}

const float* Procedural::get( EFloatArrayAttribute in_attr) const
{
    const float* resultPtr=NULL;
    unsigned int uiArraySize = 0;

    if( in_attr==DensityFalloff )
    {
       uiArraySize = getArraySize( m_node, "xgen_densityFalloff", AI_TYPE_FLOAT, true );
       if( uiArraySize == 7 )
       {
          if( getFloatArray( m_node, "xgen_densityFalloff", resultPtr, true ) )
             return resultPtr;
       }
    }
    else if( in_attr==LodHi )
    {
       uiArraySize = getArraySize( m_node, "xgen_lodHi", AI_TYPE_FLOAT, true );
       if( uiArraySize == 2 )
       {
          if( getFloatArray( m_node, "xgen_lodHi", resultPtr, true ) )
             return resultPtr;
       }
    }
    else if( in_attr==LodLow )
    {
       uiArraySize = getArraySize( m_node, "xgen_lodLo", AI_TYPE_FLOAT, true );
       if( uiArraySize == 2 )
       {
          if( getFloatArray( m_node, "xgen_lodLo", resultPtr, true ) )
             return resultPtr;
       }
    }
    else if( in_attr==LodMed )
    {
       uiArraySize = getArraySize( m_node, "xgen_lodMed", AI_TYPE_FLOAT, true );
       if( uiArraySize == 2 )
       {
          if( getFloatArray( m_node, "xgen_lodMed", resultPtr, true ) )
             return resultPtr;
       }
    }
    else if( in_attr==Shutter )
    {
       uiArraySize = getArraySize( m_node, "time_samples", AI_TYPE_FLOAT, false );
       if( getFloatArray( m_node, "time_samples", resultPtr, false ) )
          return resultPtr;
    }

   return NULL;
}

unsigned int Procedural::getSize( EFloatArrayAttribute in_attr )const
{
    if( in_attr==DensityFalloff )
    {
       return getArraySize( m_node, "xgen_densityFalloff", AI_TYPE_FLOAT, true );
    }
    else if( in_attr==LodHi )
    {
       return getArraySize( m_node, "xgen_lodHi", AI_TYPE_FLOAT, true );
    }
    else if( in_attr==LodLow )
    {
       return getArraySize( m_node, "xgen_lodLo", AI_TYPE_FLOAT, true );
    }
    else if( in_attr==LodMed )
    {
       return getArraySize( m_node, "xgen_lodMed", AI_TYPE_FLOAT, true );
    }
    else if( in_attr==Shutter )
    {
       return getArraySize( m_node, "time_samples", AI_TYPE_FLOAT, false );
    }

    return 0;
}

const char* Procedural::getOverride( const char* in_name )const
{
   const char* cstr = NULL;
   if( getString( m_node, in_name, cstr, true  ) && cstr!=NULL )
   {
      return cstr;
   }

   return "";
}

// Auto close the file descriptor.
class auto_fclose
{
public:
   auto_fclose( FILE* fd )
   {
      m_fd = fd;
   }

   ~auto_fclose()
   {
      if(m_fd)
         fclose(m_fd);
   }
   FILE* m_fd;
};

bool Procedural::getArchiveBoundingBox( const char* in_filename, bbox& out_bbox )const
{
   std::string fname( in_filename );

   std::string fileBase = "";
   int extPos = 0;
   if ((extPos = (int)fname.find(".ass")) == ((int)fname.length()-4))
   {
      fileBase = fname.substr(0, extPos);
   }
   else if ((extPos = (int)fname.find(".ass.gz")) == ((int)fname.length()-7))
   {
      fileBase = fname.substr(0, extPos);
   }
   else
      return false;

   std::string asstocfile = fileBase + ".asstoc";

   if(m_bboxes.find(asstocfile) == m_bboxes.end())
   {
      std::ifstream file(asstocfile.c_str());
      if (!file.is_open())
      {
         if (XGDebugLevel >= 2)
            XGRenderAPIDebug(/*msg::C|msg::PRIMITIVE|2,*/ "Could not open "+ asstocfile);
         return false;
      }

      std::string line;
      std::getline(file, line);

      char *str = new char[line.length() + 1];
      strcpy(str, line.c_str());

      strtok(str, " ");
      out_bbox.xmin = atof(strtok(NULL, " "));
      out_bbox.ymin = atof(strtok(NULL, " "));
      out_bbox.zmin = atof(strtok(NULL, " "));
      out_bbox.xmax = atof(strtok(NULL, " "));
      out_bbox.ymax = atof(strtok(NULL, " "));
      out_bbox.zmax = atof(strtok(NULL, " "));

      m_bboxes.insert(std::make_pair(asstocfile,out_bbox));

      file.close();
      delete[] str;

   }
   else
   {
      out_bbox = m_bboxes[asstocfile];
   }
   return true;

   // Use an auto_fclose since we are returning from the function all over the place.

 //   // Do not attempt to read non-RIB archives (e.g. .caf)
 //   if (XGDebugLevel >= 2)
 //       XGRenderAPIDebug(/*msg::C|msg::PRIMITIVE|2,*/ "Reading "+ fname);

 //   if (fname.find(".abc") == (fname.length()-4))
 //   {
 //      out_bbox.xmin = -1.0;
 //      out_bbox.ymin = -1.0;
 //      out_bbox.zmin = -1.0;

 //      out_bbox.xmax = 1.0;
 //      out_bbox.ymax = 1.0;
 //      out_bbox.zmax = 1.0;

 //       return true;
 //   }

 //   if (fname.find(".ass") == (fname.length()-4))
 //   {
   //   FILE *fd = fopen(in_filename, "rb");
   //   if (!fd) {
   //      if (XGDebugLevel >= 2)
   //         XGRenderAPIDebug(/*msg::C|msg::PRIMITIVE|2,*/ "Could not open "+ fname);
   //      return false;
   //   }

   //   // Use an auto_fclose since we are returning from the function all over the place.
   //   auto_fclose afd( fd );

   //   // Scan the first N lines searching for "## BBOX ...."
   //   const int limit = 13;
   //   const int inner_limit = 192;
   //   int matched;
   //   int count = 0;
   //   int inner_count = 0;

   //   while (count < limit) {
   //      count++;
   //      inner_count = 0;
   //      matched = fscanf(fd, "## BBOX %lf %lf %lf %lf %lf %lf",
   //                   &out_bbox.xmin, &out_bbox.xmax, &out_bbox.ymin, &out_bbox.ymax, &out_bbox.zmin, &out_bbox.zmax);

   //      if (matched == 0) {
   //         // Skip this line
   //         char c = fgetc(fd);
   //         if (/*EOF == c ||*/ feof(fd))
   //            return false;

   //         while (c != '\n') {
   //            c = fgetc(fd);
   //            // Guard against really long lines
   //            if (inner_limit <= inner_count++)
   //               break;
   //            if (/*EOF == c ||*/ feof(fd)) {
   //               if (XGDebugLevel >= 2)
   //                  XGRenderAPIDebug(/*msg::C|msg::PRIMITIVE|2,*/ "EOF");
   //               return false;
   //            }
   //         }
   //         continue;
   //      }

   //      if (matched == 6) {
   //         if (XGDebugLevel >= 2)
   //            XGRenderAPIDebug(/*msg::C|msg::PRIMITIVE|2,*/
   //                  "DRA BBOX" +
   //                  std::string(" ") + std::to_string((long double)out_bbox.xmin) +
   //                  std::string(" ") + std::to_string((long double)out_bbox.xmax) +
   //                  std::string(" ") + std::to_string((long double)out_bbox.ymin) +
   //                  std::string(" ") + std::to_string((long double)out_bbox.ymax) +
   //                  std::string(" ") + std::to_string((long double)out_bbox.zmin) +
   //                  std::string(" ") + std::to_string((long double)out_bbox.zmax));

   //         return true;
   //      }
   //      if (EOF == matched || feof(fd)) {
   //         if (XGDebugLevel >= 2)
   //            XGRenderAPIDebug(/*msg::C|msg::PRIMITIVE|2,*/ "EOF");
   //         break;
   //      }
   //   }
 //   }

}

void Procedural::convertMatrix( const AtMatrix in_mat, mat44& out_mat )
{
   // FIXME Arnold5 is it correct ?
   memcpy( &out_mat, &in_mat[0][0], sizeof(float)*16 );
}

void Procedural::getTransform( float in_time, mat44& out_mat )const
{
   AtMatrix result = AiM4Identity();
   //AiArrayInterpolateMtx( AiNodeGetArray( m_node, "matrix" ), in_time, 0, result );

   convertMatrix( result, out_mat );
}

// Primitive cache get macro.
// To avoid writing PrimitiveCache:: for every get.
#define PC( x ) ( XGenRenderAPI::PrimitiveCache:: x )


/**
 * Emit the Arnold nodes for the cached primitives. This might be called
 * as primitives are emited to the renderer (to keep the size of the cache
 * down) or can be called from endPatch to flush the remaining cache.
 */
void Procedural::flush(  const char* geomName, PrimitiveCache* pc )
{
   bool bIsSpline = pc->get( PC(PrimIsSpline) );
   const char* strPrimType = pc->get( PC(PrimitiveType) );

    if( bIsSpline )
       flushSplines( geomName, pc );
    else if ( strcmp( strPrimType, "CardPrimitive" )==0 )
        flushCards( geomName, pc );
    else if ( strcmp( strPrimType, "SpherePrimitive" )==0 )
        flushSpheres( geomName, pc );
    else if ( strcmp( strPrimType, "ArchivePrimitive" )==0 )
        flushArchives( geomName, pc );
}

/**
 * Emit the Arnold nodes for the cached primitives. This might be called
 * as primitives are emited to the renderer (to keep the size of the cache
 * down) or can be called from endPatch to flush the remaining cache.
 */
void Procedural::flushSplines( const char *geomName, PrimitiveCache* pc )
{
    bool bFaceCamera = pc->get( PC(FaceCamera) );
    int mode = AiNodeGetInt( m_node, "ai_mode" );

    unsigned int numSamples = pc->get( PC(NumMotionSamples) );
    //unsigned int shutterSize = pc->getSize( PC(Shutter) );
    //unsigned int cacheCount = pc->get( PC(CacheCount) );

    unsigned int pointsTotal = pc->getSize2( PC(Points), 0 );
    unsigned int numPointsTotal = pc->getSize2( PC(NumVertices), 0 );
    //unsigned int orientationsTotal = pc->getSize2( PC(Norms), 0 );
    unsigned int widthsSize = pc->getSize( PC(Widths) );

    AtArray* num_points = AiArrayAllocate( numPointsTotal, numSamples, AI_TYPE_UINT );
    AtArray* points = AiArrayAllocate( pointsTotal, numSamples, AI_TYPE_VECTOR );
    AtArray* radius = AiArrayAllocate( widthsSize>0 ? widthsSize : 1, 1, AI_TYPE_FLOAT );
    AtArray* orientations = (bFaceCamera || (mode == 1)) ? NULL : AiArrayAllocate( pointsTotal, numSamples, AI_TYPE_VECTOR );

    // FIXME Arnold5
    unsigned int* curNumPoints = (unsigned int*)AiArrayMap(num_points);
    AtVector* curPoints = (AtVector*)AiArrayMap(points);//->data;
    AtVector* curOrientations = orientations ? (AtVector*)AiArrayMap(orientations)/*->data*/ : NULL;
    float* curRadius = (float*)AiArrayMap(radius);//->data;

    // Add NumPoints
    if (curNumPoints != NULL && curPoints != NULL)
    {
       for ( int i=0; i < (int)numSamples; i++ )
       {
           // Add the points.
           XGRenderAPIDebug( "Adding points." );
           memcpy( curPoints, pc->get( PC(Points), i ), sizeof( AtVector )*pointsTotal );
           curPoints+=pointsTotal;

           const vec3* pNorms = pc->get( PC(Norms), i );

           int* numVertsPtr = (int*)pc->get( PC(NumVertices), i );
           for( unsigned int j=0; j<pc->getSize2( PC(NumVertices), i ); ++j )
           {
              *curNumPoints = (unsigned int)numVertsPtr[j];

              // Add the normals if necessary.
            if( curOrientations )
            {
               XGRenderAPIDebug( "Adding normals." );

               unsigned int numVarying = *curNumPoints - 2;

               memcpy( curOrientations, &pNorms[0], sizeof(AtVector) );
               curOrientations++;

               memcpy( curOrientations, pNorms, sizeof(AtVector)*numVarying );
               curOrientations+=numVarying;

               memcpy( curOrientations, &pNorms[numVarying-1], sizeof(AtVector) );
               curOrientations++;

               pNorms += numVarying;
            }

              curNumPoints++;
           }
        }
    }

    if (curRadius)
    {
       // Add the constant widths.
       if( widthsSize==0 )
       {
         float constantWidth = pc->get( PC(ConstantWidth) );

         XGRenderAPIDebug( "Constant width: " + ftoa(constantWidth));
         *curRadius = constantWidth * 0.5f;
       }
       // Add Varying Widths
       else
       {
         const float* pWidths = pc->get( PC(Widths) );

         XGRenderAPIDebug( "Non-constant width.");
         for( unsigned int w=0; w<widthsSize; ++w )
         {
            curRadius[w] = pWidths[w] * 0.5f;
         }
       }
    }
    
   // Create only one node, all arrays get merged into it at the end
   if ( !m_merged_data )
   {
      m_merged_data = new XgMergedData();

      AtNode* nodeCurves = AiNode("curves");
      string strParentName = AiNodeGetName( m_node_face );
      string strID = itoa( (int)m_nodes.size() );
      string nodeName = strParentName + string("_curves_") + strID;
      AiNodeSetStr( nodeCurves, "name", nodeName.c_str()) ;

      AiNodeSetUInt(nodeCurves, "id", getHash(nodeCurves));
      AiNodeSetStr( nodeCurves, "mode", (mode == 1? "thick" :( bFaceCamera ? "ribbon" : "oriented")));
      AiNodeSetStr( nodeCurves, "basis", "b-spline" );
      AiNodeSetArray( nodeCurves, "shader", m_shaders ? AiArrayCopy(m_shaders) : NULL );

      float min_pixel_width = AiNodeGetFlt( m_node, "ai_min_pixel_width" );
      AiNodeSetFlt( nodeCurves, "min_pixel_width", min_pixel_width );

      // Transmitting parent node parameters to child nodes (#2752)
      AiNodeSetBool(nodeCurves, "opaque", AiNodeGetBool(m_node, "opaque"));
      AiNodeSetByte(nodeCurves, "visibility", AiNodeGetByte(m_node, "visibility"));
      AiNodeSetBool(nodeCurves, "self_shadows", AiNodeGetBool(m_node, "self_shadows"));
      AiNodeSetBool(nodeCurves, "receive_shadows", AiNodeGetBool(m_node, "receive_shadows"));
      AiNodeSetBool(nodeCurves, "matte", AiNodeGetBool(m_node, "matte"));
      AiNodeSetFlt(nodeCurves, "motion_start", AiNodeGetFlt(m_node, "motion_start"));
      AiNodeSetFlt(nodeCurves, "motion_end", AiNodeGetFlt(m_node, "motion_end"));
      

      // Add custom renderer parameters.
      pushCustomParams( nodeCurves, pc );

      // Keep our new nodes.
      m_nodes.push_back( nodeCurves );
   }
   else
   {
      // Add custom renderer uniform parameters to m_merged_data
      pushCustomParams( NULL, pc );
   }

   m_merged_data->add_array( "num_points", num_points );
   m_merged_data->add_array( "points", points );
   m_merged_data->add_array( "radius", radius );
   if( orientations ) m_merged_data->add_array( "orientations", orientations );

   if (num_points)
      AiArrayUnmap(num_points);
   if (points)
      AiArrayUnmap(points);
   if (orientations)
      AiArrayUnmap(orientations);
   if (radius)
      AiArrayUnmap(radius);

}

/**
 * Emit the Arnold Sphere nodes for the cached primitives. This might be called
 * as primitives are emited to the renderer (to keep the size of the cache
 * down) or can be called from endPatch to flush the remaining cache.
 */
void Procedural::flushSpheres( const char *geomName, PrimitiveCache* pc )
{
   string strParentName = AiNodeGetName( m_node_face );


   // Build up the token and parameter lists to output for all
   // passes of motionBlur.
   double length_;
   double width;
   double depth;
   vec3 P;
   vec3 lengthVec;
   vec3 axis1;
   double angle1;
   vec3 axis2;
   double angle2[2];
   vec3 zeroAxis = { 0.f, 0.f, 0.f };

   mat44 xP0, xP, xN, tmp;


   unsigned int cacheCount = pc->get( PC(CacheCount) );
   unsigned int numSamples = pc->get( PC(NumMotionSamples) );
   //unsigned int shutterSize = pc->getSize( PC(Shutter) );
   //float* shutter = (float*)pc->get( PC(Shutter) );

   bool normalParam = pc->get( PC(NormalParam) );
   bool flipParam = pc->get( PC(FlipParam) );

    for ( unsigned int j=0; j<cacheCount; j++ )
    {
       AtArray* matrix = AiArrayAllocate( 1, numSamples, AI_TYPE_MATRIX );

        for ( unsigned int i=0; i < numSamples; i++ )
        {
            // Determine scaling values.
            int p0 = j*4; // Start of first point
            int p1 = j*4 + 1; // Start of second point
            int p2 = j*4 + 2; // Start of third point
            int p3 = j*4 + 3; // Start of fourth point

            const vec3* points_i = pc->get( PC(Points), i );

            P = points_i[p0];
            vec3 lengthP( points_i[p1] );
            vec3 midP(( P + lengthP )/2.0 );
            vec3 widthP( points_i[p2] );
            vec3 depthP( points_i[p3] );
            lengthVec = lengthP - P;
            vec3 widthVec = widthP - midP;
            length_ = length(lengthVec);
            width = length(widthVec) * 2.0;
            depth = length((depthP - midP)) * 2.0;

            // Determine axis and angle of rotation.
            vec3 yAxis = { 0.f, 1.f, 0.f };
            vec3 xAxis = { 1.f, 0.f, 0.f };
            vec3 xChange;

            axis1 = yAxis * lengthVec;
            if( normalize( axis1 ) > 0.0 ) {
                angle1 = angle( yAxis, lengthVec );
                xChange = rotateBy( xAxis, axis1, angle1 );
            } else {
                angle1 = 0.0;
                axis1 = xAxis;
                xChange = xAxis;
            }
            axis2 = xChange * widthVec;
            if ( normalize( axis2 ) > 0.0 ) {
                angle2[i%2] = angle( xChange, widthVec );
                if ( dot( axis2, lengthVec ) < 0.0 )
                    angle2[i%2] *= -1.0;
            } else {
                angle2[i%2] = 0.0;
            }
            axis2 = yAxis;

            // We want to make sure motion frames take the shortest
            // distance from an angular position.
            if ( i > 0 ) {
                if ( angle2[i%2] - angle2[(i-1)%2] > 3.14159 ) {
                    angle2[i%2] -= 6.28319;
                } else if ( angle2[i%2] - angle2[(i-1)%2] < -3.14159 ) {
                    angle2[i%2] += 6.28319;
                }
            }

            // Now use these values to create the transforms for each motion
            // sample and put in a motion block

            // Translation
            translation( tmp, P + lengthVec / 2.0 );
            xP = tmp;
            
            // Rotation 1
            if ( axis1 != zeroAxis ) {
                rotation( tmp, axis1, (float)angle1 );
                multiply( xP, xP, tmp );
                if ( normalParam && (i==0) )
                    xN = tmp;
            }
            
            // Rotation 2
            if ( axis2 != zeroAxis ) {
                rotation( tmp, axis2, (float)angle2[i%2] );
                multiply( xP, xP, tmp );
                if ( normalParam && (i==0) )
                    multiply( xN, xN, tmp );
            }
            
            // Scale
            vec3 scaleV;
            scaleV.x = (float)width;
            scaleV.y = (float)length_;
            scaleV.z = (float)depth;
            scale( tmp, scaleV );

            multiply( xP, xP, tmp );
            if ( flipParam ) {
                rotationX( tmp, (float)degtorad(-90.0) );
            } else {
                rotationX( tmp, (float)degtorad(90) );
            }
            multiply( xP, xP, tmp );
            if ( normalParam && (i==0) )
                multiply( xN, xN, tmp );

            if(i == 0)
               xP0 = xP;

            const float* xPi = &xP._00;
            AtMatrix tmp = {{{float(xPi[0]),float(xPi[1]),float(xPi[2]),float(xPi[3])},
                            {float(xPi[4]),float(xPi[5]),float(xPi[6]),float(xPi[7])},
                            {float(xPi[8]),float(xPi[9]),float(xPi[10]),float(xPi[11])},
                            {float(xPi[12]),float(xPi[13]),float(xPi[14]),float(xPi[15])}}};

            AiArraySetMtx( matrix, i, tmp );
        }

        // Add custom parameters and call sphere.
        pc->inverseXformParams( j, xP0, xN );

        string strID = itoa( (int)m_nodes.size() );

        
        // and a geometry instance node.
        AtNode* nodeInstance = AiNode("ginstance");
        string nodeName = strParentName + string("_ginstance_") + strID;
        AiNodeSetStr( nodeInstance, "name", nodeName.c_str());
        AiNodeSetUInt(nodeInstance, "id", getHash(nodeInstance) );
        AiNodeSetArray( nodeInstance, "matrix", matrix );
        AiNodeSetPtr( nodeInstance, "node", (void*)m_sphere );
        AiNodeSetArray( nodeInstance, "shader", m_shaders ? AiArrayCopy(m_shaders) : NULL );
        AiNodeSetByte( nodeInstance, "visibility", AI_RAY_ALL );

        // Transmitting parent node parameters to child nodes (#2752)
        AiNodeSetBool(nodeInstance, "opaque", AiNodeGetBool(m_node, "opaque"));
        AiNodeSetBool(nodeInstance, "self_shadows", AiNodeGetBool(m_node, "self_shadows"));
        AiNodeSetBool(nodeInstance, "receive_shadows", AiNodeGetBool(m_node, "receive_shadows"));
        AiNodeSetBool(nodeInstance, "matte", AiNodeGetBool(m_node, "matte"));

        AiNodeSetFlt(nodeInstance, "motion_start", AiNodeGetFlt(m_node, "motion_start"));
        AiNodeSetFlt(nodeInstance, "motion_end", AiNodeGetFlt(m_node, "motion_end"));
      
        // Add custom renderer parameters.
        pushCustomParams( nodeInstance, pc, j);

        // Keep our new nodes.
        m_nodes.push_back( nodeInstance );
    }
}

/**
 * Emit the Arnold nodes for the cached primitives. This might be called
 * as primitives are emitted to the renderer (to keep the size of the cache
 * down) or can be called from endPatch to flush the remaining cache.
 */
void Procedural::flushCards( const char *geomName, PrimitiveCache* pc )
{
   string strParentName = AiNodeGetName( m_node_face );

   // Create a NURBS surface node for each card primitive. Properties are:
   // #span = 1,1, #degree = 3,3, #CVs = 4x4, #knots = 8,8
   // 

   unsigned int cacheCount = pc->get( PC(CacheCount) );
   unsigned int numSamples = pc->get( PC(NumMotionSamples) );

   for (unsigned int j=0; j < cacheCount; j++)
   {
      // Add the points.
      XGRenderAPIDebug(/*msg::C|msg::RENDERER|4,*/ "Adding points.");

      // NURBS CVs : 16 * (x,y,z,w) in Arnold
      float *cvsData = new float[16*4*numSamples]; 

      for (unsigned int i=0; i<numSamples; i++)
      {
         // Offset to the i-th motion sample of the j-th primitive
         const vec3* pointPtr = pc->get( PC(Points), i ) + j*16;
         
         float* cvsPtr = cvsData + i*16*4;

         // Copy points to Arnold array for i-th motion sample
         for ( int p = 0; p < 16; p++, pointPtr++ ) 
         {
             *cvsPtr++ = pointPtr->x;
             *cvsPtr++ = pointPtr->y;
             *cvsPtr++ = pointPtr->z;
             *cvsPtr++ = 1.0f;
         }      
      }
      AtArray *cvs = AiArrayConvert(16*4, numSamples, AI_TYPE_FLOAT, cvsData);

      // NURBS knots : 1 (span) + 2 * 3 (degree) + 1 = 8 floats
      AtArray* knotsU = AiArray( 8, 1, AI_TYPE_FLOAT,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f );
      AtArray* knotsV = AiArray( 8, 1, AI_TYPE_FLOAT,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f );
 
      string strID = itoa( (int)m_nodes.size() );

      // and a geometry instance node.
      AtNode* nodeCard = AiNode( "nurbs" );
      string nodeName = strParentName + string("_nurbs_") + strID;
      AiNodeSetStr( nodeCard, "name", nodeName.c_str());
      AiNodeSetInt(nodeCard, "id", getHash(nodeCard ));
      AiNodeSetArray( nodeCard, "shader", m_shaders ? AiArrayCopy(m_shaders) : NULL );

      AiNodeSetInt( nodeCard, "degree_u", 3 );
      AiNodeSetInt( nodeCard, "degree_v", 3 );
      AiNodeSetArray( nodeCard, "knots_u", knotsU );
      AiNodeSetArray( nodeCard, "knots_v", knotsV );
      AiNodeSetArray( nodeCard, "cvs", cvs );

      // Transmitting parent node parameters to child nodes (#2752)
      AiNodeSetByte(nodeCard, "visibility", AiNodeGetByte(m_node, "visibility"));
      AiNodeSetBool(nodeCard, "opaque", AiNodeGetBool(m_node, "opaque"));
      AiNodeSetBool(nodeCard, "self_shadows", AiNodeGetBool(m_node, "self_shadows"));
      AiNodeSetBool(nodeCard, "receive_shadows", AiNodeGetBool(m_node, "receive_shadows"));
      AiNodeSetBool(nodeCard, "matte", AiNodeGetBool(m_node, "matte"));
      
      AiNodeSetFlt(nodeCard, "motion_start", AiNodeGetFlt(m_node, "motion_start"));
      AiNodeSetFlt(nodeCard, "motion_end", AiNodeGetFlt(m_node, "motion_end"));
      
      // Add custom renderer parameters.
       pushCustomParams( nodeCard, pc );

      // Keep our new nodes.
      m_nodes.push_back( nodeCard );
    }

}

struct CustomParamTypeEntry
{
   string m_xgen;
   string m_arnold;
   size_t m_sizeOf;
   size_t m_components;
   uint8_t m_type;
   bool   m_constant;
};

const static CustomParamTypeEntry g_mapCustomParamTypes[]=
{
   { "uniform float ",    "uniform FLOAT",     sizeof(float),       1, AI_TYPE_FLOAT,  false},
   { "constant float ",   "constant FLOAT",    sizeof(float),       1, AI_TYPE_FLOAT,  true },
   { "uniform color ",    "uniform RGB",       sizeof(AtRGB),       3, AI_TYPE_RGB,    false },
   { "constant color ",   "constant RGB",      sizeof(AtRGB),       3, AI_TYPE_RGB,    true },
   { "uniform vector ",   "uniform VECTOR",    sizeof(AtVector),    3, AI_TYPE_VECTOR, false },
   { "constant vector ",  "constant VECTOR",   sizeof(AtVector),    3, AI_TYPE_VECTOR, true },
   { "uniform normal ",   "uniform VECTOR",    sizeof(AtVector),    3, AI_TYPE_VECTOR, false },
   { "constant normal ",  "constant VECTOR",   sizeof(AtVector),    3, AI_TYPE_VECTOR, true },
};
const static size_t g_ulCustomParamTypesCount = sizeof(g_mapCustomParamTypes) / sizeof(CustomParamTypeEntry);

/**
 * Push the custom shader parameters into the tokens and params arrays. Shader
 * parameters are assumed to have ONE motion sample.
 *
 * @param i index into param array
 */
void Procedural::pushCustomParams( AtNode* in_node, PrimitiveCache* pc , unsigned int cacheCount)
{
   unsigned int customAttrCount = pc->getSize( PC( CustomAttrNames ) );
   // Push any user-defined custom attributes.
   for ( unsigned int j = 0; j<customAttrCount; j++ ) {
      string attrName = pc->get( PC( CustomAttrNames ), j );
      const float* attrValue = pc->get( PC( CustomAttrValues ), j );
      unsigned int attrCount = pc->getSize2( PC( CustomAttrValues ), j );

      // See if the entry is an array and if so the number of elements
      int count = arrayindex( attrName );
      if ( count<1 ) count = 1;

      for( size_t i=0; i<g_ulCustomParamTypesCount; ++i )
      {
         const CustomParamTypeEntry& e = g_mapCustomParamTypes[i];
         if ( attrName.find( e.m_xgen ) != string::npos)
         {
            string fixedAttrName = attrName.substr( e.m_xgen.size() );
            unsigned int fixAttrCount = (unsigned int)(attrCount/e.m_components);

            // if in_node is NULL it means we are merging multiple nodes into
            // one and we do not need to declare the same attribute again
            if ( in_node )
               AiNodeDeclare( in_node, fixedAttrName.c_str(), e.m_arnold.c_str() );

            if(e.m_constant) //constant attribute
            {
               if ( in_node )
               {
                  unsigned int offset = (unsigned int)(cacheCount*e.m_components);
                  switch(e.m_type)
                  {
                     case AI_TYPE_FLOAT:
                        AiNodeSetFlt( in_node, fixedAttrName.c_str(), attrValue[offset]);
                        break;
                     case AI_TYPE_RGB:
                        AiNodeSetRGB( in_node, fixedAttrName.c_str(), attrValue[offset+ 0], attrValue[offset+1], attrValue[offset+2] );
                        break;
                     case AI_TYPE_VECTOR:
                        AiNodeSetVec( in_node, fixedAttrName.c_str(), attrValue[offset+ 0], attrValue[offset+1], attrValue[offset+2] );
                        break;
                  }
               }
            }
            else // uniform attribute
            {
               AtArray* a = AiArrayAllocate( fixAttrCount, 1, e.m_type );
               void *aData = AiArrayMap(a);
               if (aData)
                  memcpy( aData, attrValue, e.m_sizeOf*fixAttrCount );
               if ( m_merged_data )
                  m_merged_data->add_array( fixedAttrName.c_str(), a );
               else
                  AiNodeSetArray( in_node, fixedAttrName.c_str(), a );

               AiArrayUnmap(a);
            }
            break;
         }
      }

    }
}

void Procedural::flushArchives( const char *geomName, PrimitiveCache* pc )
{
/*
    // DEBUG INFO
    {
        // Get num of archives
        unsigned int cacheCount = pc->get( PC(CacheCount) );
        unsigned int numSamples = pc->get( PC(NumMotionSamples) );
        double archiveSize = pc->get( PC(ArchiveSize) );
        unsigned int archivesCount = pc->getSize( PC(Archives) );
        unsigned int archivesAbsCount = pc->getSize( PC(ArchivesAbsolute) );

        char buf[1024];
        sprintf( buf, "Procedural::flushArchives: Patch(%s), \
            CacheCount(%d), MotionSamples(%d), \
            ArchiveSize(%f), ArchivesCount(%d), \
            AbsArchivesCount(%d)\n",
            geomName, cacheCount, numSamples,
            archiveSize, archivesCount, archivesAbsCount );
        std::cout << buf;

        // Get archive
        const char* archivesAbs = pc->get( PC(ArchivesAbsolute), 0 );
        std::cout << "Procedural::flushArchives: Archive: " << archivesAbs << "\n";
    }
*/

   string strParentName = AiNodeGetName( m_node_face );

   // Default to 1.0 so that it has no effect for archive files that
   // do not contain BBOX information
   double bbox_scale = 1.0 / pc->get( PC(ArchiveSize) );

   unsigned int cacheCount = pc->get( PC(CacheCount) );
   unsigned int numSamples = pc->get( PC(NumMotionSamples) );

//   unsigned int shutterSize = pc->getSize( PC(Shutter) );
//   float* shutter = (float*)pc->get( PC(Shutter) );

   bool normalParam = pc->get( PC(NormalParam) );

   int lodLevels = pc->get( PC(LodLevels) );
//   const bool* useLevel = pc->get( PC(ArchiveUseLevel) );
//   const float* minVis = (const float*)&pc->get( PC(MinVis) ).x;
//   const float* maxVis = (const float*)&pc->get( PC(MaxVis) ).x;
//   const float* loTrans = (const float*)&pc->get( PC(LoTrans) ).x;
//   const float* upTrans = (const float*)&pc->get( PC(UpTrans) ).x;

   // Get archive name string pointers
   // double archiveScale = pc->get( PC(ArchiveSize) ); // unused variable
   unsigned int archivesSize = pc->getSize( PC(Archives) );
   const char** archives = new const char*[archivesSize];
   const char** archivesAbsolute = new const char*[archivesSize];
   //const char** archivesObjects = new const char*[archivesSize];
   const char** archivesMaterial = new const char*[archivesSize];
   for ( unsigned int a = 0; a < archivesSize; a++ )
   {
      archives[a] = pc->get( PC(Archives), a );
      archivesAbsolute[a] = pc->get( PC(ArchivesAbsolute), a );
      //archivesObjects[a] = pc->get(PC(ArchivesObjects), a);
      archivesMaterial[a] = pc->get(PC(ArchivesMaterial), a);
   }
   const double* archivesFrame = pc->get( PC(ArchivesFrame_XP) );

   double length_;
   double width;
   double depth;
   vec3 P;
   vec3 lengthVec;
   vec3 axis1;
   double angle1;
   vec3 axis2;
   double angle2[2];
   vec3 zeroAxis = { 0.f, 0.f, 0.f };

   mat44 xP, xP0, xN, tmp;


    for ( unsigned int j = 0; j < cacheCount; j++ ) {

        AtArray* matrix = AiArrayAllocate( 1, numSamples, AI_TYPE_MATRIX );

        // Build up the token and parameter lists to output for all
        // passes of motionBlur.
        
        int jj=j*lodLevels*numSamples;

        string strID = itoa((int)m_nodes.size());

        string instance_name = strParentName + string("_archive_") + strID;
        
        for ( unsigned int i=0; i <numSamples; i++ )
        {
            // Determine scaling values.
            int p0 = j*4; // Start of first point
            int p1 = j*4 + 1; // Start of second point
            int p2 = j*4 + 2; // Start of third point
            int p3 = j*4 + 3; // Start of fourth point

            const vec3* points_i = pc->get( PC(Points), i );

            P = points_i[p0];
            vec3 lengthP( points_i[p1] );
            vec3 midP(( P + lengthP )/2.0 );
            vec3 widthP( points_i[p2] );
            vec3 depthP( points_i[p3] );
            lengthVec = lengthP - P;
            vec3 widthVec = widthP - midP;
            length_ = length(lengthVec);
            width = length( widthVec ) * 2.0;
            depth = length(depthP - midP)* 2.0;

            // Determine axis and angle of rotation.
            vec3 yAxis={ 0.0, 1.0, 0.0 };
            vec3 xAxis={ 1.0, 0.0, 0.0 };
            vec3 xChange;

            axis1 = yAxis * lengthVec;
            if ( normalize(axis1) > 0.0 ) {
                angle1 = angle(yAxis, lengthVec );
                xChange = rotateBy(xAxis, axis1, angle1 );
            } else {
                angle1 = 0.0;
                axis1 = xAxis;
                xChange = xAxis;
            }
            axis2 = xChange * widthVec;
            if ( normalize(axis2) > 0.0 ) {
                angle2[i%2] = angle( xChange, widthVec );
                if ( dot( axis2, lengthVec ) < 0.0 )
                    angle2[i%2] *= -1.0;
            } else {
                angle2[i%2] = 0.0;
            }
            axis2 = yAxis;


            // We want to make sure motion frames take the shortest
            // distance from an angular position.
            if ( i > 0 ) {
                if ( angle2[i%2] - angle2[(i-1)%2] > 3.14159 ) {
                    angle2[i%2] -= 6.28319;
                } else if ( angle2[i%2] - angle2[(i-1)%2] < -3.14159 ) {
                    angle2[i%2] += 6.28319;
                }
            }

            // Now use these values to create the transforms for each motion
            // sample and put in a motion block

            // Translation
            translation( tmp, P );
            xP = tmp;

            // Rotation 1
            if ( axis1 != zeroAxis ) {
                rotation( tmp, axis1, (float)angle1 );
                multiply( xP, xP, tmp );
                if ( normalParam && (i==0))
                    xN = tmp;
            }

            // Rotation 2
            if ( axis2 != zeroAxis ) {
                rotation( tmp, axis2, (float)angle2[i%2] );
                multiply( xP, xP, tmp );
                if ( normalParam && (i==0) )
                    multiply( xN, xN, tmp );
            }

            // Scale
            vec3 scaleV;
            scaleV.x = (float)(bbox_scale * width);
            scaleV.y = (float)(bbox_scale * length_);
            scaleV.z = (float)(bbox_scale * depth);
            scale( tmp, scaleV );
            multiply( xP, xP, tmp );


            if(i == 0)
               xP0 = xP;

            float* xPi = &xP._00;
            AtMatrix tmp = {{{float(xPi[0]),float(xPi[1]),float(xPi[2]),float(xPi[3])},
                            {float(xPi[4]),float(xPi[5]),float(xPi[6]),float(xPi[7])},
                            {float(xPi[8]),float(xPi[9]),float(xPi[10]),float(xPi[11])},
                            {float(xPi[12]),float(xPi[13]),float(xPi[14]),float(xPi[15])}}};
            AiArraySetMtx( matrix, i, tmp );
        }

        // Add custom parameters.
        pc->inverseXformParams( j, xP0, xN );

        // Get archive bbox

      std::vector < std::string > vecFilenames;
      std::vector < std::string > vecMaterials;
      std::string archivesAbsoluteJJ = archivesAbsolute[jj];
      std::string archivesMaterialJJ = archivesMaterial[jj];


      size_t currentF, currentM;
      size_t nextF = (size_t)-1;
      size_t nextM = (size_t)-1;
      do
      {
         currentF = nextF + 1;
         currentM = nextM + 1;
         nextF = archivesAbsoluteJJ.find_first_of("\n", currentF);
         nextM = archivesMaterialJJ.find_first_of("\n", currentM);
         vecFilenames.push_back(archivesAbsoluteJJ.substr(currentF, nextF - currentF));
         vecMaterials.push_back(archivesMaterialJJ.substr(currentM, nextM - currentM));
      }
      while (nextF != std::string::npos);

      for( size_t i=0; i<vecFilenames.size(); ++i )
      {
         std::string filename = vecFilenames[i];
         std::string materialName = vecMaterials[i];
         std::string uniqueName = instance_name + "_" + itoa((int)i);

         AtNode* materialNode = AiNodeLookUpByName(materialName.c_str());
         if (materialNode == NULL && m_initArnoldFunc != NULL)
         {
            materialNode = m_initArnoldFunc(materialName.c_str());            
         }
         
         if (materialNode == NULL)
         {
            // still haven't found the material.
            // In pre-2.1.0 versions, the material was the shading engine.
            // But since it no longer exists now, we cannot find it anymore.
            // In most frequent cases, the shading engine has the same name as the 
            // material, with a suffix 'SG'. Let's try to consider this case to minimize 
            // incompatibilities.
            size_t materialNameLength = materialName.length();
            if (materialNameLength > 2 && materialName[materialNameLength - 2] == 'S' && materialName[materialNameLength - 1] == 'G')
            {
               materialName = materialName.substr(0, materialNameLength - 2);
               materialNode = AiNodeLookUpByName(materialName.c_str());
               if (materialNode == NULL && m_initArnoldFunc != NULL)
                  materialNode = m_initArnoldFunc(materialName.c_str());            
            }
         }

         if (materialNode == NULL)
            AiMsgWarning("[xgen] Material %s not found in the arnold scene", materialName.c_str());

         std::string ext3 = filename.size() > 3 ? filename.substr(filename.size() - 3) : "";
         std::string ext6 = filename.size() > 7 ? filename.substr(filename.size() - 6) : "";
         if (!(ext3 == "ass") && !(ext6 == "ass.gz"))
            continue;

         // Expand the ${FRAME} token.
         {
            size_t pos = filename.find("${FRAME}");
            while ( pos != string::npos ) {
               filename.replace(pos,8, itoa((int)floor(archivesFrame[j]),"%4.4d") );
               pos = filename.find("${FRAME}",pos+1);
            }
         }
/*
         bbox arcbox;
         if( !getArchiveBoundingBox( filename.c_str(), arcbox ) )
         {
            std::cerr << "ERROR: XgArnoldProcedural: Unable to get asset information for " << archivesAbsolute[jj] << "\n";
            continue;
         }*/

         // Scale the bbox by the archive bbox
         /*
         arcbox.xmin *= archiveScale;
         arcbox.ymin *= archiveScale;
         arcbox.zmin *= archiveScale;
         arcbox.xmax *= archiveScale;
         arcbox.ymax *= archiveScale;
         arcbox.zmax *= archiveScale;
         */


         AtNode* archive_procedural = getArchiveProceduralNode( filename.c_str(), instance_name.c_str(), /*arcbox,*/ archivesFrame[j] );
         if ( archive_procedural )
         {
            AiNodeSetStr( archive_procedural, "name", uniqueName.c_str() );
            AiNodeSetUInt(archive_procedural, "id", getHash(archive_procedural ));
            AiNodeSetArray( archive_procedural, "matrix", AiArrayCopy(matrix) );
            if(materialNode != NULL)
               AiNodeSetPtr( archive_procedural, "shader", materialNode );
            else if(materialName.size() > 0 && m_shaders != NULL)
               AiNodeSetArray( archive_procedural, "shader", AiArrayCopy(m_shaders));
            

            // Add custom renderer parameters.
            pushCustomParams( archive_procedural, pc ,j);

            m_nodes.push_back( archive_procedural );
         }
      }

      AiArrayDestroy(matrix);
    }
    
    delete [] archives;
    delete [] archivesAbsolute;
}

// Get info from archive file and create procedural node
AtNode* Procedural::getArchiveProceduralNode( const char* file_name, const char* instance_name, /*const bbox& arcbox,*/ double frame )
{
   // Assuming the archive is exported at 24fps
   /*frame /= 24.0;
   char strFrame[256];
   snprintf( strFrame, 255, "%lf", frame );*/

   // Get arnold info
   //static string dso = string(getenv("MTOA_PATH")) + string("/procedurals/libAbcArnold.so");
   string dso = string(file_name);
   
   /*std::string dso_data;
   dso_data += string(" -filename ") + string(file_name);
   dso_data += string(" -nameprefix ") + string(instance_name);
   dso_data += string(" -frame ") + string(strFrame);
   dso_data += string(" -fps 24 ");
   dso_data += string(" -shutteropen 0.0 " );
   dso_data += string(" -shutterclose 0.0 " );
   dso_data += string(" -makeinstance ");*/

   // Return a procedural node
   AtNode* abcProc = AiNode("procedural");
   AiNodeSetStr( abcProc, "filename", dso.c_str() );
   //AiNodeSetStr( abcProc, "data", dso_data.c_str() );
   //AiNodeSetVec( abcProc, "min", (float)arcbox.xmin, (float)arcbox.ymin, (float)arcbox.zmin );
   //AiNodeSetVec( abcProc, "max", (float)arcbox.xmax, (float)arcbox.ymax, (float)arcbox.zmax );

   return abcProc;
}

}