// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef INCLUDED_RIPYTHON_H
#define INCLUDED_RIPYTHON_H

#include <ri.h>
#include <rx.h>

#include <FnAttribute/FnAttribute.h>
#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <FnAttribute/FnGroupBuilder.h>

#include <PRManPluginState.h>
#include <ptr/shared_ptr.h>
namespace ptr = SPI_ptr_v4;

//TODO: use normal openexr
#ifdef USE_SPINAMESPACED_EXR
#define SPI_GEOLIB_IMATH_NAMESPACE namespace Imath = SPI::OpenEXR::Imath;
#else
#define SPI_GEOLIB_IMATH_NAMESPACE
#endif

#include <Timer.h>

#include <string>
#include <vector>
#include <list>
#include <tr1/unordered_map>


#ifdef _WIN32
#define FN_FUNCTIONEXPORT __declspec(dllexport)
#else
#define FN_FUNCTIONEXPORT __attribute__ ((visibility("default")))
#endif

// Stub entry points which call the equivalent KatanaProcedural namespaced functions.
// This is to maintain support for launching a procedural from a stub RIB file.
extern "C" FN_FUNCTIONEXPORT RtPointer ConvertParameters(RtString paramstr);
extern "C" FN_FUNCTIONEXPORT RtVoid Subdivide(RtPointer data, RtFloat detail);
extern "C" FN_FUNCTIONEXPORT RtVoid Free(RtPointer data);

#include <ProducerPacket.h>
#include <WriteRI_Util.h>
#include <PRManPluginState.h>

namespace PRManProcedural
{
    typedef std::map<std::string, FnKat::FnScenegraphIterator> InstanceCache;

    void RegisterInstanceCache(std::string location, InstanceCache instanceCache, int numUniqueProducers, PRManPluginState* sharedState);

    void PushProducerIntoGeometricCache(FnKat::FnScenegraphIterator sgIterator, FnAttribute::DoubleAttribute parentBoundsAttr, PRManPluginState* sharedState);

    void RegisterRootSgIteratorInCache(FnKat::FnScenegraphIterator rootIterator);

    FnKat::FnScenegraphIterator GetCachedSgIterator(std::string scenegraphLocation, PRManPluginState* sharedState);

    struct CoShaderEntry
    {
        std::string shaderName;
        FnAttribute::GroupAttribute coshaderAttr;
        AttrList attrList;
        bool used;
        std::vector<std::string> dependencies;
    };
    typedef std::tr1::unordered_map<std::string, CoShaderEntry> CoShadersInfo;

    // Profiling timers.
    extern UTIL::Timer   globalTotalTime;
    extern UTIL::Counter globalScenegraphTraversalCounter;
    extern UTIL::Counter globalRIBCounter;

    // Retrieves the PRMan version and prints in the render log
    void PrintVersion();

    // Fills in the global sharedState
    void SetSharedState(const PRManPluginState& sharedStateIn);

    // These are the actual implementations of the entry point functions.
    RtPointer ConvertParameters(RtString paramstr);
    RtVoid Subdivide(RtPointer data, RtFloat detail);
    RtVoid Free(RtPointer data);

    // WriteRI.cpp
    void WriteRI(const ProducerPacket& producerPacket, PRManPluginState* sharedState);
    void WriteRI_Object(const ProducerPacket& producerPacket, PRManPluginState* sharedState);
    void WriteRI_Location(const ProducerPacket& producerPacket, PRManPluginState* sharedState, bool applyState = true);

    void WriteRI_CoordinateSystems(FnKat::FnScenegraphIterator worldSgIterator, PRManPluginState* sharedState);
    void WriteRI_Lights(FnKat::FnScenegraphIterator worldSgIterator, PRManPluginState* sharedState);

    void WriteRI_Transforms( const std::vector<FnKat::RenderOutputUtils::Transform>& transforms );

    // WriteRI_Shapes.cpp
    void WriteRI_Group(const ProducerPacket& producerPacketsharedState, PRManPluginState* sharedState);
    void WriteRI_RIBArchive(const ProducerPacket& producerPacket);
    void WriteRI_LevelOfDetailGroup(const ProducerPacket& producerPacket, PRManPluginState* sharedState);

    void WriteShader( FnKat::FnScenegraphIterator sgIterator,
                      FnAttribute::GroupAttribute material,
                      const std::string &type,
                      const std::string &attributeType,
                      FnAttribute::GroupAttribute info_attr,
                      PRManPluginState* sharedState,
                      std::string location = "");

    void WriteShader( const std::string & identifier,
                      bool flipZ,
                      FnAttribute::GroupAttribute material,
                      const std::string &type,
                      const std::string &attributeType,
                      FnAttribute::GroupAttribute info_attr,
                      PRManPluginState* sharedState,
                      std::string location = "" );

    FnAttribute::GroupAttribute ConvertNetworkMaterialToCoshaders( FnAttribute::GroupAttribute materialAttr );
    void WriteCoshaders( CoShadersInfo &coshaders, int unusedCoshaders );
    void PrepareCoshaders( const std::string & identifier, CoShadersInfo &coshaders,
                           PRManPluginState* sharedState, FnAttribute::GroupAttribute info_attr = FnAttribute::GroupAttribute() );
    void GetCoshaders( FnAttribute::GroupAttribute materialAttr, CoShadersInfo &coshaders );

    RtLightHandle GetLightHandle( std::string location, PRManPluginState* sharedState );

    void GetGlobalMembershipAndBuildVector(FnKat::FnScenegraphIterator & sgIterator, std::vector<std::string> & stringArray);
}

#endif
