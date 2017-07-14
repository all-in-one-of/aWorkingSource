// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANRENDER__H
#define PRMANRENDER__H

#include <FnRender/plugin/RenderBase.h>
#include <FnRender/plugin/CommandLineTools.h>

#include <PRManLiveRender.h>
#include <PRManRenderSettings.h>
#include <FnRender/plugin/CameraSettings.h>
#include <PRManGlobalSettings.h>
#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <PRManProcedural.h>
#include <slo.h>

#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif

#include <PRManPluginState.h>
#include <map>
#include <boost/thread.hpp>

namespace FnKatRender = Foundry::Katana::Render;

class PRManRender : public Foundry::Katana::Render::RenderBase
{
public:
    PRManRender( FnKat::FnScenegraphIterator rootIterator, FnAttribute::GroupAttribute arguments );
    virtual int start();
    virtual int stop();
    virtual int stopLiveEditing();
    virtual int queueDataUpdates( FnAttribute::GroupAttribute updateAttribute );
    virtual bool hasPendingDataUpdates() const;
    virtual int applyPendingDataUpdates();

    virtual void configureDiskRenderOutputProcess(
            FnKatRender::DiskRenderOutputProcess& diskRenderOutputProcess,
            const std::string& outputName,
            const std::string& outputPath,
            const std::string& renderMethodName,
            const float& frameTime) const;

    static Foundry::Katana::Render::RenderBase* create( FnKat::FnScenegraphIterator rootIterator, FnAttribute::GroupAttribute args )
    {
        return new PRManRender( rootIterator, args );
    }
    static void flush() {};

private:

    typedef FnKat::Render::CommandLineArguments ProceduralArguments;

    // PRMan specific functions for setting up and rendering a scene for a particular frame
    int setupScene( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings, PRManGlobalSettings &globalSettings, std::string ribFile = "" );
    int setupRerender( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings, PRManGlobalSettings &globalSettings );
    void fillCommandLineArguments( std::vector<std::string> &commandLineArgs, FnKat::FnScenegraphIterator sgi,
                                   PRManRenderSettings &renderSettings, PRManGlobalSettings &globalSettings, bool useProgressLog );
    std::string getProceduralArguments( PRManRenderSettings &renderSettings );
    int launchPRMan( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings, PRManGlobalSettings &globalSettings, bool useProgressLog );
    int startRerender( FnKat::FnScenegraphIterator rootIterator, PRManRenderSettings renderSettings, PRManGlobalSettings globalSettings, boost::recursive_mutex::scoped_lock& lock );

    int addErrorHandler( PRManGlobalSettings &globalSettings );
    int addRibInclude( FnKat::FnScenegraphIterator sgi, PRManGlobalSettings &globalSettings, std::string currentLocation );
    int addShutter( PRManRenderSettings &renderSettings );
    int addClippingPlanes( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings );
    int addFormat( PRManRenderSettings &renderSettings );
    int addDisplayLines( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings );
    int addCamera( Foundry::Katana::Render::CameraSettings *cameraSettings, PRManRenderSettings &renderSettings, bool applyRenderSettings = true );
    int addAdditionalCameras( PRManRenderSettings &renderSettings );
    int addFrameCamera( FnKat::FnScenegraphIterator sgi, PRManGlobalSettings &globalSettings, PRManRenderSettings &renderSettings );
    int addQuantize( PRManGlobalSettings &globalSettings );
    int addDefaultAttributes();
    int buildSearchPaths();
    int buildProceduralSettings( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings );

    int renderScene( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings );

    struct RerenderBake
    {
        bool enabled;
        std::string name;
        std::string directory;
    };

    bool doesCachedRerenderBakeExist( RerenderBake rerenderBake );
    void configureRerenderBake( RerenderBake &rerenderBake, PRManGlobalSettings &globalSettings );

    PRManPluginState _sharedState;

    // Paths
    std::string _proceduralArguments;
    std::string _katanaProceduralPath;
    std::string _katanaBase;
    std::string _rmanDsoPath;
    std::string _rmanShaderPath;
    std::string _katanaRmanDsoPath;
    std::string _katanaRmanBinPath;

    bool _ribDump;
    bool _ribDumpExpandProcedural;
    std::string _renderMethodName;

    std::auto_ptr<PRManLiveRender> _liveRenderUpdate;
    mutable boost::recursive_mutex _liveRenderMutex;
    std::string _rerenderer;
    bool isRaytrace()
    {
        return _rerenderer == "raytrace";
    }
};

#endif
