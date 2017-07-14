// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#if _WIN32
#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif
#endif //_WIN32

#include <PRManRender.h>
#include <Ric.h>
#include <ri.h>
#include <rx.h>

#include <FnRendererInfo/plugin/RenderMethod.h>

#include <FnAttribute/FnConstVector.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>

#include "boost/filesystem.hpp"
#include <pystring/pystring.h>

#include <dirent.h>
#include <regex.h>
#include <ErrorReporting.h>
#include <ProducerPacket.h>
#include <WriteRI_Statements.h>
#include <WriteRI_Attributes.h>
#include <boost/thread.hpp>

#include <dlfcn.h>

using namespace FnKat;

const RtInt frameNumber = 1;


namespace // anonymous
{
    // Functions to deal with parsing of RiFilter arguments, to allow quoted
    // arguments in in 'single quotes' or "double quotes" to be treated as
    // single arguments, and to correctly handle escaped characters.

    char handleEscapedChar(char charIn)
    {
        char charOut = charIn;
        switch (charIn)
        {
            case 't': charOut = '\t'; break;
            case 'b': charOut = '\b'; break;
            case 'n': charOut = '\n'; break;
            case 'r': charOut = '\r'; break;
            case 'f': charOut = '\f'; break;
        }
        return charOut;
    }

    void splitRiFilterArgs( const std::string & args, std::vector<std::string> & tokens )
    {
        tokens.clear();
        const int argsLength = args.length();
        bool unterminatedStringArg = false;
        std::string token;

        int j = 0;
        while ( j < argsLength )
        {
            // Eat up leading whitespace
            while ( j < argsLength && isspace(args[j]) )
                j++;

            // End of the string?
            if ( j >= argsLength )
                break;

            if ( (args[j] == '"') || (args[j] == '\'') )
            {
                // Enter quote - look for the corresponding end quote character
                const char quoteChar = args[j++];
                token = "";

                while ( (j < argsLength) && (args[j] != quoteChar) )
                {
                    char nextChar = args[j];

                    // Handle escaped characters
                    if ( (nextChar == '\\') && (j < argsLength - 1) )
                        nextChar = handleEscapedChar(args[++j]);

                    token.push_back( nextChar );
                    j++;
                }

                // Was the string terminated?
                if ( j >= argsLength )
                {
                    unterminatedStringArg = true;
                }
                else
                {
                    tokens.push_back( token );
                    // Don't include the end quote character
                    j++;
                }
            }
            else
            {
                // Not a quoted argument - look for the next space
                token.clear();

                while ( (j < argsLength) && (!isspace(args[j])) )
                {
                    char nextChar = args[j];

                    // Handle escaped characters
                    if ( (nextChar == '\\') && (j < argsLength - 1) )
                        nextChar = handleEscapedChar( args[++j] );

                    token.push_back( nextChar );
                    j++;
                }

                tokens.push_back( token );
            }

            if ( unterminatedStringArg )
            {
                std::ostringstream warningStream;
                warningStream << "Invalid RiFilter argument string : " << args << std::endl;
                PRManProcedural::Report_Warning( warningStream.str(), "" );
                break;
            }
        }
    }
} // end of anonymous namespace


PRManRender::PRManRender( FnKat::FnScenegraphIterator rootIterator, FnAttribute::GroupAttribute arguments ) :
    RenderBase( rootIterator, arguments ),
    _ribDump( false ),
    _ribDumpExpandProcedural( false )
{
}

int PRManRender::start()
{
    boost::recursive_mutex::scoped_lock lock(_liveRenderMutex);

    // This is a workaround for an unresolved PRMan issue.  PRMan's RI Filters are not currently
    // shipped with a link-time dependency on libprman.so, but need to access its symbols.  We
    // will load the PRMan symbols into the process's global symbol space to allow filters access 
    // to them:
    const char* dsoName = "libprman.so";
    void* handle = dlopen(dsoName, RTLD_NOW | RTLD_GLOBAL);
    if (!handle)
    {
        std::cerr << "Failed to load libprman.so (" << dlerror() << ")" << std::endl;
        return -1;
    }
    dlerror();
    // End Workaround.

    FnScenegraphIterator rootIterator = getRootIterator();

    // Store the root iterator so we can handle cache requests later.
    PRManProcedural::RegisterRootSgIteratorInCache(rootIterator);

    buildSearchPaths();
    PRManRenderSettings renderSettings( rootIterator );
    PRManGlobalSettings globalSettings( rootIterator, _rmanDsoPath, _rmanShaderPath );

    _renderMethodName = getRenderMethodName();

    // Treat batch renders as disk renders
    if( _renderMethodName == FnKat::RendererInfo::DiskRenderMethod::kBatchName )
    {
        _renderMethodName = FnKat::RendererInfo::DiskRenderMethod::kDefaultName;
    }

    if( _renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName )
    {
        startRerender( rootIterator, renderSettings, globalSettings, lock );
    }
    else
    {
        std::string ribOut = getRenderOutputFile();

        setupScene( rootIterator, renderSettings, globalSettings, ribOut );
        renderScene( rootIterator, renderSettings );
        RiFrameEnd();
    }

    return 0;
}

int PRManRender::startRerender( FnKat::FnScenegraphIterator rootIterator,
                                PRManRenderSettings renderSettings,
                                PRManGlobalSettings globalSettings,
                                boost::recursive_mutex::scoped_lock& lock )
{
    std::string rerenderSource;

    FnKat::StringAttribute rerendererAttr =
            rootIterator.getAttribute("liveRenderSettings.prman.rerenderer");
    _rerenderer = rerendererAttr.getValue("raytrace", false);

    bool renderFromCache = false;
    std::string katTempDir = getKatanaTempDirectory();
    std::string cacheSubDir = "/prmanCache";

    RerenderBake cache;
    configureRerenderBake( cache, globalSettings );
    std::string cacheDir = cache.directory;

    // We fall back on the Katana temp directory if the bake directory has not been set.
    if( cacheDir != "" )
        renderFromCache = true;
    else
        cacheDir = katTempDir;

    bool cacheExists = doesCachedRerenderBakeExist( cache );

    if( renderFromCache && cacheExists )
    {
        rerenderSource = cacheDir + cacheSubDir;
    }
    else
    {
        // Re-rendering is processed in two stages. The first stage involves rendering a first pass which
        // is stored either as a render cache or a RIB file (no cache, only supported in raytrace mode).
        // The second stage launches another render but this time by editing the cache/RIB.

        if( cache.enabled && !cacheExists )
        {
            boost::filesystem::create_directories( cacheDir.c_str() );
        }

        FnKat::StringAttribute raytraceStartModeAttr =
                rootIterator.getAttribute("liveRenderSettings.prman.raytraceStartMode");
        std::string raytraceStartMode = raytraceStartModeAttr.getValue("cacheless", false);

        // The first render stage.
        if( isRaytrace() && raytraceStartMode == "cacheless" )
        {
            // Dump out a RIB file which is used to perform re-render edits.
            std::string ribFile = cacheDir + std::string( "/rerender.rib" );
            rerenderSource = ribFile;

            // Use RIB binary format for efficiency
            RtString format[1] = { "binary" };
            RiOption( "rib", "format", (RtPointer) format, RI_NULL );

            // The setup involves calling RiBegin with a RIB target (flags ribDump)
            // and RiFrameBegin
            setupScene( rootIterator, renderSettings, globalSettings, ribFile );
        }
        else
        {
            // Set up the scene without giving a RIB filename
            setupScene( rootIterator, renderSettings, globalSettings, "" );

            // Set options to turn on baking for re-rendering, so that prman will create
            // the directory specified by rerenderbakedbdir and write cached data into it
            int rerenderbake = 1;
            rerenderSource = cacheDir + cacheSubDir;
            char* rerenderbakedbdir = (char*) rerenderSource.c_str();

            RiOption( "render", "int rerenderbake", &rerenderbake, RI_NULL );
            RiOption( "render", "string rerenderbakedbdir", &rerenderbakedbdir, RI_NULL );
        }
    }

    if( !renderFromCache || ( renderFromCache && !cacheExists ) )
    {
        renderScene( rootIterator, renderSettings );

        // End the first stage render process. This is normally done in
        // finished() when we are not re-rendering.
        RiFrameEnd();
        RiEnd();

        // The rib dump is the only render process that is in-process
        // (PRManBegin/PRManEnd) so ending it is omitted if that is
        // the case.
        if( !_ribDump )
            PRManEnd();
    }

    // The second render stage.
    setupRerender( rootIterator, renderSettings, globalSettings );

    // Find edit parameters and start re-render edits
    char* rerenderer = (char*) _rerenderer.c_str();

    FnKat::StringAttribute screenOrderArgAttr =
            rootIterator.getAttribute("liveRenderSettings.prman.screenorder");
    std::string screenOrderArg = screenOrderArgAttr.getValue("variance", false);
    char* screenOrder = (char*) screenOrderArg.c_str();

    FnKat::IntAttribute progressiveAttr =
            rootIterator.getAttribute("liveRenderSettings.prman.progressive");
    int progressive = progressiveAttr.getValue(1, false);

    _liveRenderUpdate.reset( new PRManLiveRender( isRaytrace(), &_sharedState ) );
    // Now that the PRManLiveRender instance exists allow updates to be queued
    lock.unlock();

    if( cache.enabled && cache.name != "world" )
    {
        char* rerenderWorld = (char*) cache.name.c_str();

        RiEditWorldBegin( (char*) rerenderSource.c_str(),
                          "string rerenderer", &rerenderer,
                          "int progressive", &progressive,
                          "string screenorder", &screenOrder,
                          "string bakedbname", &rerenderWorld,
                          RI_NULL );
    }
    else
    {
        RiEditWorldBegin( (char*) rerenderSource.c_str(),
                          "string rerenderer", &rerenderer,
                          "int progressive", &progressive,
                          "string screenorder", &screenOrder,
                          RI_NULL );
    }

    return 0;
}

bool PRManRender::doesCachedRerenderBakeExist( RerenderBake rerenderBake )
{
    regmatch_t *match = 0;
    std::string rerenderSource = rerenderBake.directory + std::string( "/prmanCache" );

    struct dirent *entry;
    DIR *cacheDir = opendir( rerenderSource.c_str() );

    if( cacheDir )
    {
        regex_t regex;
        std::string nameWildcard = rerenderBake.name + ".*";
        bool valid = ( ::regcomp( &regex, nameWildcard.c_str(), REG_EXTENDED ) == 0 );

        if( valid )
        {
            while( ( entry = readdir( cacheDir ) ) )
            {
                std::string filename = entry->d_name;
                if( ::regexec( &regex, filename.c_str(), 0, match, 0 ) == 0 )
                {
                    closedir( cacheDir );
                    return true;
                }
            }
        }
        closedir(cacheDir);
        cacheDir = NULL;
    }

    return false;
}

void PRManRender::configureRerenderBake( RerenderBake &rerenderBake, PRManGlobalSettings &globalSettings )
{
    // Get the cache directory based on whether it has been set through global settings.
    // If it set using the (int) 'options.render.rerenderbake' then extract the (string)
    // 'options.render.rerenderbakedbdir' which contains the set directory.
    FnAttribute::GroupAttribute globalSettingsAttr = globalSettings.getGlobalSettingsAttr();
    FnAttribute::GroupAttribute optionsAttr = globalSettingsAttr.getChildByName( "options" );

    rerenderBake.name = "world";

    if ( optionsAttr.isValid() )
    {
        FnAttribute::GroupAttribute renderOptionsAttr = optionsAttr.getChildByName( "render" );
        if ( renderOptionsAttr.isValid() )
        {
            // (int) options.render.rerenderbake
            FnAttribute::IntAttribute rerenderBakeAttr = renderOptionsAttr.getChildByName( "rerenderbake" );
            if ( rerenderBakeAttr.isValid() )
            {
                FnAttribute::IntConstVector rerenderBakeVector = rerenderBakeAttr.getNearestSample( 0.f );
                int rerenderBakeValue = 0;

                if( rerenderBakeVector.size() > 0 )
                    rerenderBakeValue = rerenderBakeVector[0];

                if( rerenderBakeValue )
                    rerenderBake.enabled = true;
                else
                    return;
            }
            else
            {
                return;
            }

            // (string) options.render.rerenderbakedbdir
            FnAttribute::StringAttribute bakeDirAttr = renderOptionsAttr.getChildByName( "rerenderbakedbdir" );
            if ( bakeDirAttr.isValid() )
            {
                FnAttribute::StringConstVector bakeDirVector = bakeDirAttr.getNearestSample( 0.f );

                if( bakeDirVector.size() == 0 )
                    return;

                rerenderBake.directory = bakeDirVector[0];
            }
            else
            {
                return;
            }

            // (string) options.render.rerenderbakedbname (default is world)
            FnAttribute::StringAttribute bakeNameAttr = renderOptionsAttr.getChildByName( "rerenderbakedbname" );
            if ( bakeNameAttr.isValid() )
            {
                FnAttribute::StringConstVector bakeNameVector = bakeNameAttr.getNearestSample( 0.f );

                if( bakeNameVector.size() == 0 )
                    return;

                rerenderBake.name = bakeNameVector[0];
            }
            else
            {
                return;
            }
        }
    }
}

int PRManRender::setupScene( FnScenegraphIterator rootIterator, PRManRenderSettings &renderSettings,
                             PRManGlobalSettings &globalSettings, std::string ribFile )
{
    if ( ribFile != "" )
    {
        _ribDump = true;
        _ribDumpExpandProcedural = isExpandProceduralActive();

        RiBegin( (char*) ribFile.c_str() );
    }
    else
    {
        launchPRMan( rootIterator, renderSettings, globalSettings, true );
    }

#ifndef _WIN32
    RiSystem( "/bin/echo Render process started." );
#endif

    // Set error handling before any other PRMan command
    addErrorHandler( globalSettings );

    addFrameCamera( rootIterator, globalSettings, renderSettings );

    int frameNumber = (int) getRenderTime();
    RiFrameBegin( (RtInt) frameNumber );

    addRibInclude( rootIterator, globalSettings, "After FrameBegin; Before Display" );

    addShutter( renderSettings );
    globalSettings.applyGlobalSettings();
    _rmanShaderPath = globalSettings.getShaderPath();   // Adopt changes from PrmanGlobalSettings

    if( renderSettings.isValid() )
    {
        addFormat( renderSettings );
        addAdditionalCameras( renderSettings );
        addDisplayLines( rootIterator, renderSettings );
    }
    addQuantize( globalSettings );

    addRibInclude( rootIterator, globalSettings, "After Display; Before Camera" );
    addCamera( renderSettings.getCameraSettings(), renderSettings );
    addRibInclude( rootIterator, globalSettings, "After Camera; Before WorldBegin" );

    addClippingPlanes( rootIterator, renderSettings );

    return 0;
}

int PRManRender::setupRerender( FnScenegraphIterator rootIterator, PRManRenderSettings &renderSettings, PRManGlobalSettings &globalSettings )
{
    launchPRMan( rootIterator, renderSettings, globalSettings, false );
    addDisplayLines( rootIterator, renderSettings );

    return 0;
}

void PRManRender::fillCommandLineArguments( std::vector<std::string> &commandLineArgs, FnScenegraphIterator sgi, PRManRenderSettings &renderSettings,
                                            PRManGlobalSettings &globalSettings, bool useProgressLog )
{
    commandLineArgs.push_back( "" );

    // Render threads
    int numberOfRenderThreads = 1;
    char renderThreadsBuffer[1024];
    FnAttribute::GroupAttribute commandArgsSettings = globalSettings.getAttribute( "commandArguments" );

    // Check render settings for a render threads override
    renderSettings.applyRenderThreads( numberOfRenderThreads );

    // Check global settings for a render threads override
    if( commandArgsSettings.isValid() )
    {
        FnAttribute::IntAttribute threadsGlobalAttr = commandArgsSettings.getChildByName( "numThreadModeProcs" );
        if( threadsGlobalAttr.isValid() )
            numberOfRenderThreads = threadsGlobalAttr.getValue();
    }

    // Check if the UI preferences or the batch command line contains a render thread override
    applyRenderThreadsOverride( numberOfRenderThreads );

    sprintf( renderThreadsBuffer, "%d", numberOfRenderThreads );

    // Print a message in the render log (used by the test harness)
    std::cout << "Using " << renderThreadsBuffer << " threads." << std::endl;

    std::string renderThreadDeclaration = std::string( "-t:" ) + std::string( renderThreadsBuffer );
    commandLineArgs.push_back( renderThreadDeclaration );

    // RIB capture path
    FnAttribute::StringAttribute capturePathAttr = commandArgsSettings.getChildByName( "capturePath" );
    if( capturePathAttr.isValid() )
    {
        std::string capturePath = capturePathAttr.getValue();
        commandLineArgs.push_back( "-capture" );
        commandLineArgs.push_back( capturePath );
    }

    // RI Filters
    FnAttribute::StringAttribute riFiltersAttr = commandArgsSettings.getChildByName( "riFilters" );
    if( riFiltersAttr.isValid() )
    {
        // We can have a number of ri filters defined in a single string, separated by |
        //   e.g.: 'myFilter1|myFilter2 arg1 arg2'
        // where the first token in each string is the name of the ri filter, and the
        // remaining tokens are the filter arguments.
        std::vector<std::string> riFilters;
        pystring::split( riFiltersAttr.getValue(), riFilters, "|" );

        std::vector<std::string>::iterator filtersIter;
        std::vector<std::string>::iterator filterArgsIt;

        for( filtersIter = riFilters.begin(); filtersIter != riFilters.end(); filtersIter++ )
        {
            std::vector<std::string> filterTokens;
            splitRiFilterArgs( *filtersIter, filterTokens );

            if( filterTokens.size() == 0 )
                continue;

            commandLineArgs.push_back( "-rif" );
            commandLineArgs.push_back( filterTokens[0] );

            // Leave loop if the filter has no arguments
            if( filterTokens.size() == 1 )
                continue;

            commandLineArgs.push_back( "-rifargs" );
            for( filterArgsIt = filterTokens.begin() + 1; filterArgsIt != filterTokens.end(); filterArgsIt++ )
            {
                commandLineArgs.push_back( (*filterArgsIt) );
            }
            commandLineArgs.push_back( "-rifend" );
        }
    }

    // Progress messages in the render log
    if( useProgressLog )
        commandLineArgs.push_back( "-Progress" );

    // Warning filters
    StringAttribute woffAttr = commandArgsSettings.getChildByName( "woff" );
    if( woffAttr.isValid() )
    {
        std::vector<std::string> woff;
        pystring::split( woffAttr.getValue(), woff, "|" );
        std::vector<std::string>::iterator woffIter;

        for( woffIter = woff.begin(); woffIter != woff.end(); ++woffIter )
        {
            std::string woffToken = (*woffIter);

            if( !woffToken.empty() )
            {
                commandLineArgs.push_back( "-woff" );
                commandLineArgs.push_back( woffToken );
            }
        }
    }
}

static char* convert( const std::string &str )
{
    return (char*) str.c_str();
}

int PRManRender::launchPRMan( FnScenegraphIterator sgi, PRManRenderSettings &renderSettings,
                              PRManGlobalSettings &globalSettings, bool useProgressLog )
{
    std::vector<std::string> commandLineArgs;
    fillCommandLineArguments( commandLineArgs, sgi, renderSettings, globalSettings, useProgressLog );

    std::vector<char*> argsv;
    std::transform( commandLineArgs.begin(), commandLineArgs.end(),
                    std::back_inserter( argsv ), convert );

    // Start PRMan
    int status = PRManBegin( commandLineArgs.size(), &argsv[0] );
    if( status )
        return status;

    RiBegin( RI_RENDER );

    return 0;
}

int PRManRender::stopLiveEditing()
{
    RiEditWorldEnd();
    return 0;
}

int PRManRender::stop()
{
    RiEnd();

    // The rib dump is the only render process that is in-process
    // (PRManBegin/PRManEnd) so ending it is omitted if that is
    // the case.
    if( !_ribDump )
        return PRManEnd();

    return 0;
}

int PRManRender::queueDataUpdates( FnAttribute::GroupAttribute updateAttribute )
{
    boost::recursive_mutex::scoped_lock lock(_liveRenderMutex);
    if( _liveRenderUpdate.get() )
        _liveRenderUpdate->processUpdates( updateAttribute );

    return 0;
}

bool PRManRender::hasPendingDataUpdates() const
{
    if( _liveRenderUpdate.get() )
    {
        if( _liveRenderUpdate->getNumberOfUpdates() > 0 )
            return true;
    }

    return false;
}

int PRManRender::applyPendingDataUpdates()
{
    if( _liveRenderUpdate.get() )
    {
        _liveRenderUpdate->applyUpdates( getRootIterator() );
    }

    return 0;
}

int PRManRender::buildSearchPaths()
{
    _katanaBase = getKatanaBasePath();
    _rmanDsoPath = getEnvironmentVariable( "RMAN_DSOPATH" );
    _rmanShaderPath = getEnvironmentVariable( "RMAN_SHADERPATH" );
    _katanaRmanDsoPath = getRenderPluginDirectory();

    // We assume some modules such as the display drivers are relative to the
    // plug-in path (../RenderBin)
    boost::filesystem::path dsoPath( _katanaRmanDsoPath );
    dsoPath /= "../RenderBin";
    _katanaRmanBinPath = dsoPath.string();

    _rmanDsoPath = _katanaRmanDsoPath + ":" + _katanaRmanBinPath + ":" + _rmanDsoPath;

    return 0;
}

int PRManRender::addErrorHandler( PRManGlobalSettings &globalSettings )
{
    FnAttribute::StringAttribute errorHandlerAttr = globalSettings.getAttribute( "errorHandler" );

    if( errorHandlerAttr.isValid() )
    {
        const std::string errorHandler = errorHandlerAttr.getValue("print", false);

        if( errorHandler == "ignore" )
        {
            RiErrorHandler( RiErrorIgnore );
        }
        else if( errorHandler == "print" )
        {
            RiErrorHandler( RiErrorPrint );
        }
        else if ( errorHandler == "abort" )
        {
            RiErrorHandler( RiErrorAbort );
        }
        else
        {
            PRManProcedural::Report_Warning("Invalid error handler specified.", "");
        }
    }

    return 0;
}

int PRManRender::addRibInclude( FnScenegraphIterator sgi, PRManGlobalSettings &globalSettings, std::string currentLocation )
{
    FnAttribute::GroupAttribute globalSettingsAttr = globalSettings.getGlobalSettingsAttr();

    FnAttribute::StringAttribute ribIncludeAttr = globalSettingsAttr.getChildByName( "ribInclude" );
    if( ribIncludeAttr.isValid() )
    {
        std::string data = ribIncludeAttr.getValue();

        std::string location;
        FnAttribute::StringAttribute ribIncludeLocationAttr = globalSettingsAttr.getChildByName( "ribIncludeLocation" );
        if( ribIncludeLocationAttr.isValid() )
            location = ribIncludeLocationAttr.getValue();
        else
            location = "After FrameBegin; Before Display";

        if( location != currentLocation )
        {
            return 0;
        }

        RiArchiveRecord( (char*) "comment", (char*) "begin user-defined rib include" );

        // Resolve asset ID
        if (DefaultAssetPlugin::isAssetId(data))
        {
            data = DefaultAssetPlugin::resolvePath(data, 0);
        }

        if( pystring::endswith( data, ".rib" ) )
        {
            RiReadArchive( const_cast<RtToken>( data.c_str() ) , NULL, RI_NULL );
        }
        else
        {
            RtArchiveHandle arHandle = RiArchiveBegin( (char*) "inline", RI_NULL );
            RiArchiveRecord( (char*) "verbatim", (char*) "%s\n", ribIncludeAttr.getValue().c_str() );
            RiArchiveEnd();
            RiReadArchive( arHandle, NULL, RI_NULL );
        }

        RiArchiveRecord( (char*) "comment", (char*) "end user-defined rib include" );
    }
    else
    {
        return 1;
    }

    return 0;
}

int PRManRender::addClippingPlanes( FnScenegraphIterator sgi, PRManRenderSettings &renderSettings )
{
    FnScenegraphIterator worldIterator = sgi.getByPath( "/root/world" );
    if( !worldIterator.isValid() )
        return 1;

    FnAttribute::GroupAttribute globalsAttr = worldIterator.getAttribute( "globals" );
    if( globalsAttr.isValid() )
    {
        FnAttribute::GroupAttribute clippingPlanesAttr = globalsAttr.getChildByName( "clippingPlanes" );
        if( clippingPlanesAttr.isValid() )
        {
            int noClippingPlanes = clippingPlanesAttr.getNumberOfChildren();
            for( int i = 0; i < noClippingPlanes; ++i )
            {
                FnAttribute::StringAttribute planeAttr = clippingPlanesAttr.getChildByIndex( i );
                if( planeAttr.isValid() )
                {
                    std::string clippingPlaneName = planeAttr.getValue();
                    FnScenegraphIterator planeProducer = sgi.getByPath( clippingPlaneName );
                    if( !planeProducer.isValid() )
                        continue;

                    FnAttribute::IntAttribute enableAttr = planeProducer.getAttribute( "enableClippingPlane" );
                    if( !enableAttr.isValid() )
                        continue;
                    if( enableAttr.getValue() == 0 )
                        continue;

                    bool reversePlane = false;
                    FnAttribute::IntAttribute reverseAttr = planeProducer.getAttribute( "reverseClippingDirection" );
                    if( reverseAttr.isValid() && reverseAttr.getValue() == 1 )
                        reversePlane = true;

                    RiTransformBegin();
                    PRManProcedural::WriteRI_Object_GlobalTransform( planeProducer, &_sharedState );

                    if( reversePlane )
                        RiClippingPlane( 0, -1, 0, 0, 0, 0 );
                    else
                        RiClippingPlane( 0, 1, 0, 0, 0, 0 );

                    RiTransformEnd();
                }
            }
        }
    }

    return 0;
}

int PRManRender::addShutter( PRManRenderSettings &renderSettings )
{
    if( renderSettings.getMaxTimeSamples() > 1 )
    {
        RiShutter( renderSettings.getShutterOpen(), renderSettings.getShutterClose() );
    }

    return 0;
}

int PRManRender::addFormat( PRManRenderSettings &renderSettings )
{
    int dataWindowSize[2];
    renderSettings.getDataWindowSize( dataWindowSize );
    float pixelaspectratio = 1.f;

    RiIdentity();
    RiOrientation( RI_RH );
    RiFormat( dataWindowSize[0], dataWindowSize[1], pixelaspectratio );

    return 0;
}

void appendChildAttrsToAttrList( PRManProcedural::AttrList& attrList, FnAttribute::GroupAttribute groupAttr,
                                 const std::string& namePrefix = "" )
{
    for ( int i = 0; i < groupAttr.getNumberOfChildren(); ++i )
    {
        const std::string childName = namePrefix + groupAttr.getChildName( i );
        std::string declaration = PRManProcedural::BuildDeclarationFromAttribute( childName,
                                                               groupAttr.getChildByIndex( i ) );
        if ( !declaration.empty() )
        {
            attrList.push_back(PRManProcedural::AttrListEntry( declaration,
                                                               childName,
                                                               groupAttr.getChildByIndex( i ) ) );
        }
    }

}

int PRManRender::addDisplayLines( FnScenegraphIterator sgi, PRManRenderSettings &renderSettings )
{
    if( _renderMethodName == FnKat::RendererInfo::PreviewRenderMethod::kDefaultName ||
        _renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName )
    {
        // Interactive case
        FnKatRender::RenderSettings::ChannelBuffers interactiveBuffers;
        renderSettings.getChannelBuffers( interactiveBuffers );

        // Render outputs
        FnKatRender::RenderSettings::RenderOutputs outputs = renderSettings.getRenderOutputs();

        std::string underscores = "";
        int origin[2];
        int originalSize[2];
        float sampleRate[2];

        float frameTime = getRenderTime();
        renderSettings.getSampleRate( sampleRate );
        renderSettings.getWindowOrigin( origin );
        renderSettings.getDisplayWindowSize( originalSize );
        std::string host = getKatanaHost();

        int allowIDPass = 0;
        if( useRenderPassID() )
            allowIDPass = 1;

        int mainSequenceId = -1;

        std::string displayDriver = "SocketMultiresDisplayDriver";
        if( _renderMethodName == FnKat::RendererInfo::PreviewRenderMethod::kDefaultName )
            displayDriver = "SocketDisplayDriver";

        for( FnKatRender::RenderSettings::ChannelBuffers::const_iterator it = interactiveBuffers.begin(); it != interactiveBuffers.end(); ++it )
        {
            const std::string outputName = it->first;
            FnKatRender::RenderSettings::ChannelBuffer buffer = it->second;

            int frameId = atoi( buffer.bufferId.c_str() );
            RtString frameName[] = { (char*) outputName.c_str() };
            RtString channelName = (char*) buffer.channelName.c_str();
            std::string displayName = underscores + host;

            // Check if there is an alternative camera in the outputs
            std::string cameraName;
            FnKatRender::RenderSettings::RenderOutputs::iterator outputIt = outputs.find(outputName);
            if ( outputIt != outputs.end() )
            {
                cameraName = outputIt->second.cameraName;
            }

            if( !cameraName.empty() )
            {
                RtString cameraNameValue[] = { (char*) cameraName.c_str() };

                RiDisplay( (char*) displayName.c_str(),
                           (char*) displayDriver.c_str(), channelName,
                           "int frameID", &frameId,
                           "float frameTime", &frameTime,
                           "string frameName", (RtPointer) frameName,
                           "float[2] sampleRate", sampleRate,
                           "int[2] origin", origin,
                           "int[2] OriginalSize", originalSize,
                           "int allowIDPass", &allowIDPass,
                           "string camera", cameraNameValue,
                           RI_NULL );
            }
            else
            {
                RiDisplay( (char*) displayName.c_str(),
                           (char*) displayDriver.c_str(), channelName,
                           "int frameID", &frameId,
                           "float frameTime", &frameTime,
                           "string frameName", (RtPointer) frameName,
                           "float[2] sampleRate", sampleRate,
                           "int[2] origin", origin,
                           "int[2] OriginalSize", originalSize,
                           "int allowIDPass", &allowIDPass,
                           RI_NULL );
            }

            if( underscores == "" )
                underscores = "+";
            underscores += "_";

            if( mainSequenceId == -1 )
                mainSequenceId = frameId;
        }

        // We do not declare an ID pass for live renders until we get the PRMan bug fix
        if( _renderMethodName != FnKat::RendererInfo::LiveRenderMethod::kDefaultName )
        {
            if( allowIDPass && mainSequenceId != -1 )
            {
                std::string displayName = underscores + host;
                RtString frameName[] = { "__id" };
                RtString channelName = "uniform float id";

                int quantize[4] = { 0, 0, 0, 0 };

                RtString filter[] = { "zmin" };
                float filterWidth[2] = { 1, 1 };

                RiDisplay( (char*) displayName.c_str(),
                           (char*) displayDriver.c_str(), channelName,
                           "int frameID", &mainSequenceId,
                           "float frameTime", &frameTime,
                           "string frameName", (RtPointer) frameName,
                           "quantize", quantize,
                           "filter", (RtPointer) filter,
                           "filterwidth", filterWidth,
                           "float[2] sampleRate", sampleRate,
                           "int[2] origin", origin,
                           "int[2] OriginalSize", originalSize,
                           "int allowIDPass", &allowIDPass,
                           RI_NULL );

                RtString pythonHostName[] = { (char*) host.c_str() };
                RiAttribute( "user", "uniform string ProceduralHostName", (RtPointer) pythonHostName, RI_NULL );
                RiAttribute( "user", "uniform int ProceduralFrameID", &mainSequenceId, RI_NULL );
                RiAttribute( "user", "uniform int ProceduralGenerateID", &allowIDPass, RI_NULL );
            }
        }
    }
    else if( _renderMethodName == FnKat::RendererInfo::DiskRenderMethod::kDefaultName )
    {
        FnKatRender::RenderSettings::RenderOutputs outputs = renderSettings.getRenderOutputs();
        std::vector<std::string> renderOutputNames = renderSettings.getRenderOutputNames();
        std::string displayPrefix = "";

        for( std::vector<std::string>::const_iterator it = renderOutputNames.begin(); it != renderOutputNames.end(); ++it )
        {
            FnKatRender::RenderSettings::RenderOutput output = outputs[(*it)];

            PRManProcedural::AttrList attrList;
            std::string displayDriver;
            std::string displayMode;
            std::string displayName = displayPrefix + output.renderLocation;

            FnKatRender::RenderSettings::AttributeSettings settings = output.rendererSettings;

            if( output.type == "color" )
            {
                displayDriver = "openexr";
                displayMode = output.channel;
                attrList.push_back(PRManProcedural::AttrListEntry("string exrcompression",
                                                           "exrcompression", FnAttribute::StringAttribute("rle")));
            }
            else if( output.type == "deep" )
            {
                displayDriver = "deepshad";
                displayMode = output.channel;
                attrList.push_back(PRManProcedural::AttrListEntry("string subimage",
                                                           "subimage", settings["subimage"]));
            }
            else if( output.type == "shadow" )
            {
                std::string shadowType = getStringAttrValue( settings["shadowType"] );
                if( shadowType == "zfile" )
                {
                    displayDriver = "zfile";
                    displayMode = "z";
                }
                else if( shadowType == "deepshad" )
                {
                    displayDriver = "deepshad";
                    std::string deepshadMode = getStringAttrValue( settings["deepshadMode"], "deepopacity" );
                    displayMode = deepshadMode;
                    attrList.push_back(PRManProcedural::AttrListEntry("string volumeinterpretation",
                                                               "volumeinterpretation", settings["deepshadVolumeInterpretation"]));
                }
            }
            else if( output.type == "raw" )
            {
                displayDriver = getStringAttrValue( settings["displayDriver"], "openexr" );
                displayMode = output.channel;
            }
            else if( output.type == "ptc" )
            {
                continue;
            }

            if( displayPrefix != "" && displayMode == "rgba" )
            {
                displayMode = "Ci";    // An additional display cannot be rgba
            }

            // Always add an additional camera if it has been declared, regardless of type.
            if( output.cameraName != "" )
            {
                attrList.push_back(PRManProcedural::AttrListEntry("string camera",
                                                           "camera", FnAttribute::StringAttribute(output.cameraName)));
            }

            // Add custom options to the display line
            FnAttribute::GroupAttribute displayOptionsAttr = settings["displayOptions"];
            if( displayOptionsAttr.isValid() )
            {
                appendChildAttrsToAttrList( attrList, displayOptionsAttr );
            }

            // Starting with PRMan 17, the driver supports the injection of
            // arbitrary metadata. This is done by passing named parameters with
            // the form "exrheader_$key", where $key will be the name of the
            // attribute as it shows up in the OpenEXR header. For example,
            // Display "myfile.exr" "openexr" "rgba" "string exrheader_author" ["Bob"]
            //
            // Note that even though PRMan 17 currently does not support string
            // arrays or floats arrays beyond a size of 4, we still allow
            // passing this options in here since this is likely to be added.
            FnAttribute::GroupAttribute exrheadersAttr = settings["exrheaders"];
            if( exrheadersAttr.isValid() )
            {
                appendChildAttrsToAttrList( attrList, exrheadersAttr, "exrheader_" );
            }

            PRManProcedural::AttrList_Converter converter(attrList);
            RiDisplayV( (char*) displayName.c_str(), (char*) displayDriver.c_str(), (char*) displayMode.c_str(),
                    converter.getSize(0), converter.getDeclarationTokens(0), converter.getParameters(0) );

            if( displayPrefix == "" )
            {
                displayPrefix = "+";
            }
        }
    }

    return 0;
}

void PRManRender::configureDiskRenderOutputProcess(
        FnKatRender::DiskRenderOutputProcess& diskRenderOutputProcess,
        const std::string& outputName,
        const std::string& outputPath,
        const std::string& renderMethodName,
        const float& frameTime) const
{
    // Get the recipe and retrieve the render settings
    FnScenegraphIterator rootIterator = getRootIterator();
    PRManRenderSettings renderSettings( rootIterator );

    // Build temporary render and final target locations
    std::string tempRenderLocation = FnKat::RenderOutputUtils::buildTempRenderLocation( rootIterator, outputName, "render", "exr", frameTime );
    std::string targetRenderLocation = outputPath;

    if( renderSettings.isTileRender() )
        targetRenderLocation = FnKat::RenderOutputUtils::buildTileLocation( rootIterator, outputPath );

    // Get the attributes for the render output from the recipe
    FnKatRender::RenderSettings::RenderOutput output = renderSettings.getRenderOutputByName( outputName );

    // The render action used for this render output
    std::auto_ptr<FnKatRender::RenderAction> renderAction;

    // Determine the rendering behaviour based on the output type
    if( output.type == "color" )
    {
        if( renderSettings.isTileRender() )
        {
            renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        }
        else
        {
            renderAction.reset( new FnKatRender::CopyAndConvertRenderAction( targetRenderLocation,
                    tempRenderLocation, output.clampOutput, output.colorConvert, output.computeStats,
                    output.convertSettings ) );
        }
    }
    else if( output.type == "deep" )
    {
        renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        renderAction->setLoadOutputInMonitor( false );
    }
    else if( output.type == "shadow" )
    {
        if( renderSettings.isTileRender() )
        {
            renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        }
        else
        {
            std::string shadowType = getStringAttrValue( output.rendererSettings["shadowType"] );
            if( shadowType == "zfile" )
            {
                renderAction.reset( new FnKatRender::TemporaryRenderAction( tempRenderLocation ) );

                if( output.fileExtension == "tx" )
                    diskRenderOutputProcess.addPostCommand( "txmake -shadow " + tempRenderLocation + std::string( " " ) + targetRenderLocation );
                else if( output.fileExtension == "exr" )
                    diskRenderOutputProcess.addPostCommand( "txmake -shadow -format openexr " + tempRenderLocation + std::string( " " ) + targetRenderLocation );
                else if( output.fileExtension == "tif" || output.fileExtension == "tiff" )
                    diskRenderOutputProcess.addPostCommand( "txmake -shadow -format tiff " + tempRenderLocation + std::string( " " ) + targetRenderLocation );
            }
            else if( shadowType == "deepshad" )
            {
                renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
            }
        }

        renderAction->setLoadOutputInMonitor( false );
    }
    else if( output.type == "raw" )
    {
        int rawHasOutput = getAttrValue<int, FnAttribute::IntAttribute>( output.rendererSettings["rawHasOutput"], 0 );
        if( rawHasOutput )
        {
            renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        }
        else
        {
            renderAction.reset( new FnKatRender::TemporaryRenderAction( tempRenderLocation ) );
            renderAction->setLoadOutputInMonitor( false );
        }
    }
    else if( output.type == "ptc" )
    {
        tempRenderLocation = output.tempRenderLocation;

        bool inValidRange = false;
        std::string validFrames = getStringAttrValue( output.rendererSettings["validFrames"] );
        if( validFrames == "All" )
        {
            inValidRange = true;
        }
        else if( validFrames == "None" )
        {
            inValidRange = false;
        }
        else
        {
            std::string validRange = getStringAttrValue( output.rendererSettings["validFrameRange"] );
            std::string seqStr = std::string( "image.(" ) + validRange + std::string( ")%04d.exr" );
            inValidRange = DefaultFileSequencePlugin::isFrameInFileSequence( seqStr, (int) frameTime );
        }

        if( inValidRange )
        {
            renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        }
        else
        {
            renderAction.reset( new FnKatRender::NoOutputRenderAction() );
            std::cout << "Warning: Not copying '" << tempRenderLocation << "' as it is not in the valid range." << std::endl;
        }

        renderAction->setLoadOutputInMonitor( false );
    }
    else
    {
        renderAction.reset( new FnKatRender::NoOutputRenderAction() );
        std::cout << "Warning: Unknown render output type '" << output.type << "' for render output '" << outputName << "'." << std::endl;
    }

    diskRenderOutputProcess.setRenderAction( renderAction );
}

int PRManRender::addQuantize( PRManGlobalSettings &globalSettings )
{
    bool rgbaQuantizeUsed = false;
    FnAttribute::GroupAttribute globalSettingsAttr = globalSettings.getGlobalSettingsAttr();

    if( globalSettingsAttr.isValid() )
    {
        FnAttribute::FloatAttribute rgbaAttr = globalSettingsAttr.getChildByName( "rgba" );
        if( rgbaAttr.isValid() )
        {
            FnAttribute::FloatConstVector rgbaData = rgbaAttr.getNearestSample( 0.f );
            if( rgbaData.size() == 4 )
            {
                RiQuantize( RI_RGBA, (RtInt) rgbaData[0], (RtInt) rgbaData[1], (RtInt) rgbaData[2], (RtFloat) rgbaData[3] );
                rgbaQuantizeUsed = true;
            }
        }

        FnAttribute::FloatAttribute zAttr = globalSettingsAttr.getChildByName( "z" );
        if( zAttr.isValid() )
        {
            FnAttribute::FloatConstVector zData = zAttr.getNearestSample( 0.f );
            if( zData.size() == 4 )
            {
                RiQuantize( RI_Z, (RtInt) zData[0], (RtInt) zData[1], (RtInt) zData[2], (RtFloat) zData[3] );
            }
        }
    }

    if( !rgbaQuantizeUsed )
        RiQuantize( RI_RGBA, 0, 0, 0, 0 );

    return 0;
}


int PRManRender::addCamera( Foundry::Katana::Render::CameraSettings *cameraSettings,
                            PRManRenderSettings &renderSettings,
                            bool applyRenderSettings )
{
    if( !cameraSettings )
        return 1;

    float fov = cameraSettings->getFov();
    if( cameraSettings->getProjection() == "perspective" )
        RiProjection( RI_PERSPECTIVE, "fov", &fov, RI_NULL );
    else
        RiProjection( RI_ORTHOGRAPHIC, RI_NULL );

    float screenWindow[4];
    cameraSettings->getScreenWindow( screenWindow );

    RiScreenWindow( screenWindow[0], screenWindow[2], screenWindow[1], screenWindow[3] );

    float clipping[2];
    cameraSettings->getClipping( clipping );
    RiClipping( clipping[0], clipping[1] );

    if( applyRenderSettings && renderSettings.isValid() )
    {
        float cropWindow[4];
        renderSettings.getCropWindow( cropWindow );
        RiCropWindow( cropWindow[0], cropWindow[1], cropWindow[2], cropWindow[3] );
    }

    RiScale( 1.0, 1.0, -1.0 );

    RiArchiveRecord( "comment", "Camera: %s", cameraSettings->getName().c_str() );
    PRManProcedural::WriteRI_Transforms( cameraSettings->getTransforms() );

    return 0;
}

int PRManRender::addAdditionalCameras( PRManRenderSettings &renderSettings )
{
    FnKatRender::RenderSettings::CameraMap additionalCameras = renderSettings.getAdditionalCameras();
    for( FnKatRender::RenderSettings::CameraMap::const_iterator it = additionalCameras.begin(); it != additionalCameras.end(); ++it )
    {
        RiTransformBegin();

        Foundry::Katana::Render::CameraSettings *cameraSettings = (*it).second;
        addCamera( cameraSettings, renderSettings );

        std::string cameraName = cameraSettings->getName();
        RiCamera( const_cast<RtToken>( cameraName.c_str() ), RI_NULL );

        RiTransformEnd();
    }

    return 0;
}

int PRManRender::addFrameCamera( FnScenegraphIterator sgi,
                                 PRManGlobalSettings &globalSettings,
                                 PRManRenderSettings &renderSettings )
{
    const std::string cameraPath = globalSettings.getFrameBeginCameraPath();
    if( !cameraPath.empty() )
    {
        int overscan[4];
        renderSettings.getOverscan( overscan );

        int displayWindow[4];
        renderSettings.getDisplayWindow( displayWindow );

        Foundry::Katana::Render::CameraSettings frameBeginCamera( sgi, cameraPath );
        frameBeginCamera.initialise( displayWindow, overscan, 0 );

        RiArchiveRecord( "comment", "Frame Begin Camera: %s", cameraPath.c_str() );
        RiOrientation( RI_RH );
        addCamera( &frameBeginCamera, renderSettings, false );
    }

    return 0;
}

int PRManRender::addDefaultAttributes()
{
    PRManProcedural::AttrList visibilityAttrList;
    visibilityAttrList.push_back( PRManProcedural::AttrListEntry( "int camera", "camera", IntAttribute(1) ) );
    visibilityAttrList.push_back( PRManProcedural::AttrListEntry( "int diffuse", "diffuse", IntAttribute(1) ) );
    visibilityAttrList.push_back( PRManProcedural::AttrListEntry( "int specular", "specular", IntAttribute(1) ) );
    visibilityAttrList.push_back( PRManProcedural::AttrListEntry( "int transmission", "transmission", IntAttribute(1) ) );
    visibilityAttrList.push_back( PRManProcedural::AttrListEntry( "int photon", "photon", IntAttribute(1) ) );
    visibilityAttrList.push_back( PRManProcedural::AttrListEntry( "int midpoint", "midpoint", IntAttribute(1) ) );

    PRManProcedural::AttrList userAttrList;
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_visibilitySet", "katana_visibilitySet", IntAttribute(1) ) );
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_camera", "katana_camera", IntAttribute(1) ) );
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_diffuse", "katana_diffuse", IntAttribute(0) ) );
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_specular", "katana_specular", IntAttribute(0) ) );
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_transmission", "katana_transmission", IntAttribute(0) ) );
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_photon", "katana_photon", IntAttribute(0) ) );
    userAttrList.push_back( PRManProcedural::AttrListEntry( "int katana_midpoint", "katana_midpoint", IntAttribute(0) ) );

    WriteRI_Object_ConvertAttrListToStatements( visibilityAttrList, "visibility" );
    WriteRI_Object_ConvertAttrListToStatements( userAttrList, "user" );

    return 0;
}

std::string PRManRender::getProceduralArguments( PRManRenderSettings &renderSettings )
{
    ProceduralArguments arguments;

    FnKat::Render::addArgument( arguments, "geolib3OpTree", findArgument("geolib3OpTree"));
    FnKat::Render::addArgument( arguments, "rmanShaderPath", _rmanShaderPath );
    FnKat::Render::addArgument( arguments, "katanaBase", _katanaBase );

    if( _ribDump )
    {
        if (getRenderMethodName() != FnKat::RendererInfo::LiveRenderMethod::kDefaultName)
            FnKat::Render::addArgument( arguments, "ribDump", "", true );

        if( useRenderPassID() )
            FnKat::Render::addArgument( arguments, "useIDPass", "", true );
    }

    if( _ribDumpExpandProcedural )
        FnKat::Render::addArgument( arguments, "fullRecurse", "", true );

    FnKat::Render::addArgument( arguments, "frame", findArgument( "renderTime" ) );
    FnKat::Render::addArgument( arguments, "shutter", renderSettings.getShutterAsString() );
    FnKat::Render::addArgument( arguments, "cropwindow", renderSettings.getCropWindowAsString() );

    FnKat::Render::addArgument( arguments, "xres", renderSettings.getDataWindowWidthAsString() );
    FnKat::Render::addArgument( arguments, "yres", renderSettings.getDataWindowHeightAsString() );

    // If everything has gone well then serialize the arguments
    std::string proceduralArguments = "";
    FnKat::Render::serialiseCommandLineArguments( arguments, proceduralArguments );

    return proceduralArguments;
}

int PRManRender::buildProceduralSettings( FnKat::FnScenegraphIterator sgi, PRManRenderSettings &renderSettings )
{
    if( _ribDumpExpandProcedural )
        _sharedState.proceduralSettings.fullRecurse = true;
    else
        _sharedState.proceduralSettings.fullRecurse = false;

    _sharedState.proceduralSettings.printTimerInfo = false;
    _sharedState.proceduralSettings.frameNumber = getRenderTime();
    _sharedState.proceduralSettings.shutterOpenTime = renderSettings.getShutterOpen();
    _sharedState.proceduralSettings.shutterCloseTime = renderSettings.getShutterClose();

    float cropWindow[4];
    renderSettings.getCropWindow( cropWindow );
    _sharedState.proceduralSettings.cropWindow[0] = cropWindow[0];
    _sharedState.proceduralSettings.cropWindow[1] = cropWindow[1];
    _sharedState.proceduralSettings.cropWindow[2] = cropWindow[2];
    _sharedState.proceduralSettings.cropWindow[3] = cropWindow[3];

    int dataWindowSize[2];
    renderSettings.getDataWindowSize( dataWindowSize );
    _sharedState.proceduralSettings.xres = dataWindowSize[0];
    _sharedState.proceduralSettings.yres = dataWindowSize[1];

    // Query producer caching data (renderSettings.producerCaching) from the root scene graph location
    FnAttribute::IntAttribute limitProducerCaching = sgi.getAttribute( "renderSettings.producerCaching.limitProducerCaching" );
    if( limitProducerCaching.isValid() && limitProducerCaching.getValue() != 0 )
    {
        _sharedState.proceduralSettings.limitProducerCaching = true;
    }

    FnAttribute::IntAttribute geometricProducerCacheLimit = sgi.getAttribute( "renderSettings.producerCaching.geometricProducerCacheLimit" );
    if( _sharedState.proceduralSettings.limitProducerCaching && geometricProducerCacheLimit.isValid() )
    {
        _sharedState.proceduralSettings.geometricProducerCacheLimit = std::max( 1, geometricProducerCacheLimit.getValue() );
    }

    FnAttribute::IntAttribute geometricCacheInputQueueLength = sgi.getAttribute( "renderSettings.producerCaching.geometricCacheInputQueueLength" );
    if( _sharedState.proceduralSettings.limitProducerCaching && geometricCacheInputQueueLength.isValid() )
    {
        _sharedState.proceduralSettings.geometricCacheInputQueueLength = std::max( 1, geometricCacheInputQueueLength.getValue() );
    }

    FnAttribute::IntAttribute instanceCacheInterval = sgi.getAttribute( "renderSettings.producerCaching.instanceCacheInterval" );
    if( _sharedState.proceduralSettings.limitProducerCaching && instanceCacheInterval.isValid() )
    {
        _sharedState.proceduralSettings.producerInstanceCacheInterval = std::max( 1, instanceCacheInterval.getValue() );
    }

    FnAttribute::StringAttribute printCacheStatistics = sgi.getAttribute( "renderSettings.producerCaching.printCacheStatistics" );
    if( _sharedState.proceduralSettings.limitProducerCaching && printCacheStatistics.isValid() )
    {
        std::string printCacheStatistics_str = printCacheStatistics.getValue();
        if( pystring::count( printCacheStatistics_str, "Summary" ) )
        {
            _sharedState.proceduralSettings.printCacheStatistics_summary = true;
        }
        else if( pystring::count( printCacheStatistics_str, "Debug" ) )
        {
            _sharedState.proceduralSettings.printCacheStatistics_summary = true;
            _sharedState.proceduralSettings.printCacheStatistics_debug = true;
        }
    }

    return 0;
}

class ProceduralMembershipGroupsBuilder
{
private:
    static const char * m_tokensString;
    static RtToken m_tokens[];
    std::string m_proceduralMembershipGroups;
    const char * m_valuesArray[1];
    RtPointer m_values[2];
    std::vector<std::string> m_proceduralMembershipGroupsVector;

public:
    ProceduralMembershipGroupsBuilder(FnScenegraphIterator & sgIterator)
    {
        // Build the list of membership groups
        PRManProcedural::GetGlobalMembershipAndBuildVector(sgIterator, m_proceduralMembershipGroupsVector);

        // Join them into the membership string of the form "+group1,group2"
        if(m_proceduralMembershipGroupsVector.size() > 0)
        {
            m_proceduralMembershipGroups = "+" + pystring::join(",", m_proceduralMembershipGroupsVector);
        }

        // Store the membership group as the value of a single element array
        m_valuesArray[0] = const_cast<char*>(m_proceduralMembershipGroups.c_str());

        // Store them in a null terminated value array
        m_values[0] = m_valuesArray;
        m_values[1] = 0;
    }

    static RtToken * getTokens()
    {
        return m_tokens;
    }

    RtPointer * getValues()
    {
        return m_values;
    }

    const std::vector<std::string> & getGroupsVector() const
    {
        return m_proceduralMembershipGroupsVector;
    }
};

const char * ProceduralMembershipGroupsBuilder::m_tokensString = "uniform string membership";
RtToken ProceduralMembershipGroupsBuilder::m_tokens[] = { const_cast<char*>(ProceduralMembershipGroupsBuilder::m_tokensString), 0 };

int PRManRender::renderScene( FnScenegraphIterator sgi, PRManRenderSettings &renderSettings )
{

        
    RiWorldBegin();

    RtBound bounds = { -1.1e30, 1.1e30, -1.1e30, 1.1e30, -1.1e30, 1.1e30 };

    // Add visibility attributes for the procedural if we are not creating a debug RIB where
    // the procedurals are expanded to make sure they are seen.
    // An expanded debug RIB will contain a comment instead of the attributes explaining the
    // absence of this visibility mechanism, highlighting the difference to a captured RIB
    // output.
    if( _ribDump && _ribDumpExpandProcedural )
    {
        RiArchiveRecord( "comment", "All visibility attributes are set here to 1 to ensure the "
                                    "procedural is seen and expanded when rendering." );
        RiArchiveRecord( "comment", "The original values are stored in user attributes." );
    }
    else
    {
        addDefaultAttributes();
    }

    ProceduralMembershipGroupsBuilder groupingMemberships( sgi );
    RiAttributeV( (char*) "grouping", 1, ProceduralMembershipGroupsBuilder::getTokens(), groupingMemberships.getValues() );

    Slo_SetPath( (char*) _rmanShaderPath.c_str() );

    if( _ribDump )
    {
        std::string proceduralArguments = getProceduralArguments( renderSettings );

        if( _ribDumpExpandProcedural )
        {
            Subdivide( ConvertParameters( const_cast<char*>( proceduralArguments.c_str() ) ), 0 );
        }
        else
        {
#ifdef _WIN32
            std::string katanaProcedural = _katanaRmanDsoPath + "\\KatanaProcedural.dll";
#else
            std::string katanaProcedural = _katanaRmanDsoPath + "/KatanaProcedural.so";
#endif
            RtString args[] = { (char*) katanaProcedural.c_str(), (char*) proceduralArguments.c_str() };
            RiProcedural( (RtPointer) args, bounds, RiProcDynamicLoad, NULL );
        }
    }
    else
    {
        buildProceduralSettings( sgi, renderSettings );
        PRManProcedural::PrintVersion();

        // Create the producer packet
        PRManProcedural::ProducerPacket* producerPacket = new PRManProcedural::ProducerPacket;
        producerPacket->sgIterator = sgi;
        producerPacket->fullRecurse = _sharedState.proceduralSettings.fullRecurse;

        // Pass the sharedState for the procedural to copy
        PRManProcedural::SetSharedState(_sharedState);

        // Construct a new grouping map for the procedural to reset all group
        // memberships to false inside the procedural.
        const std::vector<std::string>& groups = groupingMemberships.getGroupsVector();
        if (!groups.empty())
        {
            producerPacket->grouping = PRManProcedural::ProducerPacket::GroupingMapRcPtr(
                    new PRManProcedural::ProducerPacket::GroupingMap );

            for (int i = 0; i < groups.size(); ++i)
            {
                producerPacket->grouping->insert( std::pair<std::string, bool>(groups[i], false) );
            }
        }

        // Call the procedural
        RiProcedural( producerPacket, bounds, PRManProcedural::Subdivide, PRManProcedural::Free );
    }

    RiWorldEnd();

    return 0;
}

// Plugin Registration code
DEFINE_RENDER_PLUGIN( PRManRender )

void registerPlugins()
{
    REGISTER_PLUGIN( PRManRender, "prman", 0, 1 );
}

