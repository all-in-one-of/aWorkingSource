// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>
#include <WriteRI_Util.h>
#include <Nullstream.h>

#ifdef _WIN32
#include <windowshelpers.h>
#endif //_WIN32
#include <ri.h>
#include <rx.h>
#include <dlfcn.h>
#ifndef _WIN32
#include <libgen.h>
#endif //_WIN32

#include <dlfcn.h>
#include <pystring/pystring.h>
#include <exception>
#include <algorithm>
#include <sstream>
#include <iostream>

// Conflict between Python and log4cplus over this symbol.
#undef HAVE_STAT

#include <pystring/pystring.h>
#include <ErrorReporting.h>

using namespace FnKat;

PRManPluginState sharedState;

namespace PRManProcedural
{

namespace
{
    // Note: this nullstream *must* appear before the Timers and Counters below it
    // since they may reference it and output to it in their destructors.
    nullstream nullStream;
}

UTIL::Timer   globalTotalTime("[Katana Procedural] Total Time ", 0, true, nullStream);
UTIL::Counter globalScenegraphTraversalCounter("[Katana Procedural] Scenegraph Traversal ", nullStream);
UTIL::Counter globalRIBCounter("[Katana Procedural] Total Rib Generation ", nullStream);

void PrintVersion()
{
    char *versionString;
    RxInfoType_t attr_type;
    int attr_count;

    RxRendererInfo( "versionstring", &versionString, sizeof(char*), &attr_type, &attr_count );

    std::string versionMessage = "Using PRMan Version: ";
    if( versionString )
    {
        versionMessage += versionString;
    }
    else
    {
        versionMessage += "unknown";
    }

    Report_Info(versionMessage);
}

void SetSharedState(const PRManPluginState& sharedStateIn)
{
    sharedState = sharedStateIn;
}

namespace
{
    // Setup the global settings to some sensible defaults.
    void Initialize(void)
    {
        sharedState.proceduralSettings.fullRecurse = false;
        sharedState.proceduralSettings.printTimerInfo = false;
        sharedState.proceduralSettings.frameNumber = 1;
        sharedState.proceduralSettings.shutterOpenTime = 0.0f;
        sharedState.proceduralSettings.shutterCloseTime = 0.0f;
        sharedState.proceduralSettings.cropWindow[0] = 0.0f;
        sharedState.proceduralSettings.cropWindow[1] = 1.0f;
        sharedState.proceduralSettings.cropWindow[2] = 0.0f;
        sharedState.proceduralSettings.cropWindow[3] = 1.0f;
        sharedState.proceduralSettings.debugStream = &nullStream;

        sharedState.proceduralSettings.xres = 0;
        sharedState.proceduralSettings.yres = 0;

        PRManProcedural::PrintVersion();
    }


    // Pick up on parameters
    bool ProcessGlobalSetting(std::vector<std::string>::const_iterator& it, const std::vector<std::string>::const_iterator& end_it)
    {
        if (*it == "-geolib3OpTree")
        {
            if (++it == end_it) return true;
            sharedState.proceduralSettings.optreeFilename = *it;
            return true;
        }

        if (*it == "-timer")
        {
            sharedState.proceduralSettings.printTimerInfo = true;
            return true;
        }

        if (*it == "-fullRecurse")
        {
            sharedState.proceduralSettings.fullRecurse = true;
            return true;
        }

        if (*it == "-frame")
        {
            if (++it == end_it) return true;
            sharedState.proceduralSettings.frameNumber = atof(it->c_str());
            return true;
        }

        if (*it == "-shutter")
        {
            if (++it == end_it) return true;
            sharedState.proceduralSettings.shutterOpenTime = atof(it->c_str());

            if (++it == end_it) return true;
            sharedState.proceduralSettings.shutterCloseTime = atof(it->c_str());

            return true;
        }

        if (*it == "-cropwindow")
        {
            if (++it == end_it) return true;
            sharedState.proceduralSettings.cropWindow[0] = atof(it->c_str());

            if (++it == end_it) return true;
            sharedState.proceduralSettings.cropWindow[1] = atof(it->c_str());

            if (++it == end_it) return true;
            sharedState.proceduralSettings.cropWindow[2] = atof(it->c_str());

            if (++it == end_it) return true;
            sharedState.proceduralSettings.cropWindow[3] = atof(it->c_str());

            return true;
        }

        if (*it == "-xres")
        {
            if (++it == end_it) return true;
            sharedState.proceduralSettings.xres = atoi(it->c_str());

            return true;
        }

        if (*it == "-yres")
        {
            if (++it == end_it) return true;
            sharedState.proceduralSettings.yres = atoi(it->c_str());

            return true;
        }


        if (*it == "-debug")
        {
            sharedState.proceduralSettings.debugStream = &std::cerr;
            return true;
        }

        if (*it == "-ribDump")
        {
            sharedState.proceduralSettings.ribDump = true;
            return true;
        }

        if (*it == "-useIDPass")
        {
            sharedState.proceduralSettings.useIDPass = true;
            return true;
        }

        return false;
    }


    // All environment-variable related settings are picked up here.
    bool ProcessEnvironmentSetting(std::vector<std::string>::const_iterator& it, const std::vector<std::string>::const_iterator& end_it)
    {
        if (*it == "-pythonPath")
        {
            if (++it == end_it) return true;
            std::string pythonPath = *it;


            // append -pythonPath to existing PYTHONPATH (if any)
            const char *pythonPathEnv = getenv("PYTHONPATH");
            if (pythonPathEnv)
            {
                pythonPath = std::string(pythonPathEnv) + ":" + pythonPath;
            }
            setenv("PYTHONPATH", pythonPath.c_str(), 1);
            return true;
        }

        if (*it == "-pluginPath")
        {
            if (++it == end_it) return true;
            std::string pluginPath = *it;
            setenv("KATANA_PLUGIN_PATH", pluginPath.c_str(), 1);
            return true;
        }

        if (*it == "-rmanShaderPath")
        {
            if (++it == end_it) return true;
            std::string shaderPath = *it;
            setenv("RMAN_SHADERPATH", shaderPath.c_str(), 1);
            return true;
        }

        if (*it == "-katanaBase")
        {
            if (++it == end_it) return true;
            std::string katanaBase = *it;
            setenv("KATANA_ROOT", katanaBase.c_str(), 1);
            return true;
        }

        if (*it == "-katanaVersion")
        {
            if (++it == end_it) return true;
            setenv("KATANA_VERSION", (*it).c_str(), 1);
            return true;
        }

        if (*it == "-katanaRelease")
        {
            if (++it == end_it) return true;
            setenv("KATANA_RELEASE", (*it).c_str(), 1);
            return true;
        }

        if (*it == "-katanaTmpdir")
        {
            if (++it == end_it) return true;
            setenv("KATANA_TMPDIR", (*it).c_str(), 1);
            return true;
        }

        return false;
    }




    // Given the parameter string from prman, set up our state.
    void ProcessParameters(RtString paramstr)
    {
        // Chop up the paramstr into a vector of strings, split by whitespace.
        std::vector<std::string> paramVector;
        pystring::split(std::string(paramstr), paramVector);

        const std::vector<std::string>::const_iterator end_it = paramVector.end();
        for (std::vector<std::string>::const_iterator it = paramVector.begin(); it != end_it; ++it)
        {
            if (ProcessGlobalSetting(it, end_it))
                continue;

            if (ProcessEnvironmentSetting(it, end_it))
                continue;

            Report_Error("Unknown parameter specified '" + *it + "'");
        }

        // Set up the profiling classes.
        // If we don't want to profile, then just profile to a nullStream
        if (sharedState.proceduralSettings.printTimerInfo)
        {
            globalTotalTime.setOutputStream(std::cerr);
            globalScenegraphTraversalCounter.setOutputStream(std::cerr);
            globalRIBCounter.setOutputStream(std::cerr);
        }
        else
        {
            globalTotalTime.setOutputStream(nullStream);
            globalScenegraphTraversalCounter.setOutputStream(nullStream);
            globalRIBCounter.setOutputStream(nullStream);
        }
    }

} // anonymous namespace


/////
///// HACK!
/////
FnPluginHost * bootstrapGEOLIB(const std::string katanaPath)
{
    // The GEOLIB library will be in $KATANA_ROOT/bin
#ifdef _WIN32
    std::string geolibPath = katanaPath + "/bin/GeoAPI_cmodule.pyd";
#else
    std::string geolibPath = katanaPath + "/bin/GeoAPI_cmodule.so";
#endif

    //load GEOLIB
    void *geolibSO = dlopen(geolibPath.c_str(), RTLD_NOW);
    if(!geolibSO)
    {
        std::ostringstream os;
        os << "Error loading Geolib3 shared object.\n\t";
        os << "Error reports: '" << dlerror() << "'.";
        Report_Error(os.str());
        return 0x0;
    }

    // Get the getGeolibHost() function from GEOLIB
    void* func = dlsym(geolibSO, "getGeolibHost");
    if(!func)
    {
        Report_Error(dlerror());
        dlclose(geolibSO);
        return 0x0;
    }

    typedef FnPluginHost* (*GetGeolibHostFunc)(const char *);
    GetGeolibHostFunc getGeolibHostFunc = (GetGeolibHostFunc)func;

    // Get the host
    FnPluginHost *host = getGeolibHostFunc(katanaPath.c_str());

    // Inject the host (and consequently the suites) into the C++ plugin wrappers
    FnAttribute::Attribute::setHost(host);
    FnAttribute::GroupBuilder::setHost(host);

    return host;
}

FnSgIteratorHandle GetScenegraphIteratorHandle(const std::string & filterFilename,
    const std::string & katanaRoot)
{
    FnPluginHost *host = bootstrapGEOLIB(katanaRoot);
    if (!host)
    {
        Report_Error("renderboot: error getting host from GEOLIB bootstrap.");
        exit(1);
    }

    const FnSgIteratorHostSuite_v2 *sgitSuite =
        reinterpret_cast<const FnSgIteratorHostSuite_v2 *>(
            host->getSuite("ScenegraphIteratorHost", 2));
    if (!sgitSuite)
    {
        Report_Error("renderboot: error getting scenegraph iterator suite from GEOLIB bootstrap.");
        exit(1);
    }

    FnSgIteratorHandle sgitHandle = sgitSuite->getIteratorFromFile(filterFilename.c_str());
    return sgitHandle;
}

///
/// END HACK
///


//////////////////////////////////////////////////////////////////////////////
//
// ConvertParameters
//
RtPointer ConvertParameters(RtString paramstr)
{
    // Set everything up.
    try
    {
        Initialize();
    }
    catch (const std::exception& e)
    {
        std::ostringstream os;
        os << "Couldn't initialize due to '" << e.what() << "'.";
        Report_Error(os.str());
        return 0;
    }
    catch (...)
    {
        Report_Error("Couldn't initialize due to an un-handled exception.");
    }

    // Try to process the parameters, setup the python path.
    try
    {
        ProcessParameters(paramstr);
    }
    catch (const std::exception& e)
    {
        std::ostringstream os;
        os << "Couldn't process the parameters '" << e.what() << "'.";
        Report_Error(os.str());
        return 0;
    }
    catch (...)
    {
        Report_Error("Couldn't process the parameters due to an un-handled exception.");
        return 0;
    }

    Report_Debug("ConvertParameters entry", &sharedState);

    try
    {
        char* rootEnvVar = getenv("KATANA_ROOT");
        if(!rootEnvVar)
        {
            // Do not know KATANA_ROOT path
            Report_Error("KATANA_ROOT path not set");
            return 0;
        }
        std::string katanaRoot = std::string(rootEnvVar);
        /*if(!FnKat::RenderOutputUtils::bootstrapGEOLIB(katanaRoot))
        {
            //return an invalid iterator
            return 0;
        }*/

        FnScenegraphIterator sgIterator(GetScenegraphIteratorHandle(sharedState.proceduralSettings.optreeFilename, katanaRoot));
    
        if (!sgIterator.isValid())
        {
            Report_Fatal("Failed to build a Scenegraph Iterator.");
        }

        // Query PRManProcedural producer caching data (renderSettings.producerCaching) from root scenegraph location
        FnAttribute::IntAttribute limitProducerCaching = sgIterator.getAttribute("renderSettings.producerCaching.limitProducerCaching");
        if (limitProducerCaching.isValid() && limitProducerCaching.getValue() != 0)
        {
            sharedState.proceduralSettings.limitProducerCaching = true;
        }

        FnAttribute::IntAttribute geometricProducerCacheLimit = sgIterator.getAttribute("renderSettings.producerCaching.geometricProducerCacheLimit");
        if (sharedState.proceduralSettings.limitProducerCaching && geometricProducerCacheLimit.isValid())
        {
            sharedState.proceduralSettings.geometricProducerCacheLimit = std::max(1,geometricProducerCacheLimit.getValue());
        }

        FnAttribute::IntAttribute geometricCacheInputQueueLength = sgIterator.getAttribute("renderSettings.producerCaching.geometricCacheInputQueueLength");
        if (sharedState.proceduralSettings.limitProducerCaching && geometricCacheInputQueueLength.isValid())
        {
            sharedState.proceduralSettings.geometricCacheInputQueueLength = std::max(1,geometricCacheInputQueueLength.getValue());
        }

        FnAttribute::IntAttribute instanceCacheInterval = sgIterator.getAttribute("renderSettings.producerCaching.instanceCacheInterval");
        if (sharedState.proceduralSettings.limitProducerCaching && instanceCacheInterval.isValid())
        {
            sharedState.proceduralSettings.producerInstanceCacheInterval = std::max(1,instanceCacheInterval.getValue());
        }

        FnAttribute::StringAttribute printCacheStatistics = sgIterator.getAttribute("renderSettings.producerCaching.printCacheStatistics");
        if (sharedState.proceduralSettings.limitProducerCaching && printCacheStatistics.isValid())
        {
            std::string printCacheStatistics_str = printCacheStatistics.getValue();
            if(pystring::count(printCacheStatistics_str, "Summary"))
            {
                sharedState.proceduralSettings.printCacheStatistics_summary = true;
            }
            else if(pystring::count(printCacheStatistics_str, "Debug"))
            {
                sharedState.proceduralSettings.printCacheStatistics_summary = true;
                sharedState.proceduralSettings.printCacheStatistics_debug = true;
            }
        }

        // Return the data structure required for the first Subdivide
        ProducerPacket* producerPacket = new ProducerPacket;
        producerPacket->sgIterator = sgIterator;

        // Adopt the recursion state based on the global setting.
        // This is the one and only time we should query this information.
        producerPacket->fullRecurse = sharedState.proceduralSettings.fullRecurse;
        producerPacket->ribDump = sharedState.proceduralSettings.ribDump;

        // Grab the global pre-declaration of grouping membership
        {
            FnAttribute::StringAttribute groupingAttr =
                    sgIterator.getAttribute("prmanGlobalStatements.declarations.grouping.membership");

            if (groupingAttr.isValid())
            {
                ProducerPacket::GroupingMap *groupingMap = new ProducerPacket::GroupingMap;

                std::vector<std::string> workVector, workVector2;
                pystring::split(groupingAttr.getValue(), workVector);

                for ( std::vector<std::string>::iterator I = workVector.begin();
                        I!=workVector.end(); ++I )
                {
                    pystring::split(pystring::strip(*I, ","), workVector2, ",");

                    for ( std::vector<std::string>::iterator J = workVector2.begin();
                            J!=workVector2.end(); ++J )
                    {
                        std::string  entryName = pystring::strip((*J), "+-");
                        if (entryName.empty()) continue;

                        (*groupingMap)[entryName] = false;
                    }
                }

                if (groupingMap->size())
                {
                    producerPacket->grouping.reset(groupingMap);
                }
            }
        }

        return producerPacket;
    }
    catch(const std::exception &e)
    {
        std::ostringstream os;
        os << "Cannot read optree file '";
        os << sharedState.proceduralSettings.optreeFilename << "'.\n\t";
        os << "Exception reports '" << e.what() << "'.";
        Report_Error(os.str());

        return 0;
    }
    catch(...)
    {
        std::ostringstream os;
        os << "Cannot read script file '";
        os << sharedState.proceduralSettings.optreeFilename << "'.\n\t";
        os << "Un-handled exception.";

        Report_Error(os.str());
        return 0;
    }
    return 0; // Should never get here, but make gcc happy.
}

static void restore_visibility_attribute(const char* prmanAttrName, const char* katanaAttrName)
{
    RxInfoType_t valType;
    int valResultCount;
    RtInt intValue = 0;
    union {
        RtInt intValue;
        RtFloat floatValue;
    } value;

    if (0 == RxAttribute(katanaAttrName, &value, sizeof(value), &valType, &valResultCount))
    {
        intValue = (valType == RxInfoFloat) ? static_cast<int>(value.floatValue) : value.intValue;
        RtToken tokens[] = { const_cast<char*>(prmanAttrName), 0 };
        RtPointer values[] = { &intValue, 0 };
        RiAttributeV(const_cast<char*>("visibility"), 1, tokens, values);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Subdivide
//
RtVoid Subdivide(RtPointer data, RtFloat detail)
{
    Report_Debug("Subdivide entry", &sharedState);
    
    ProducerPacket* producerPacket = 0;
    
    try
    {
        producerPacket = reinterpret_cast<ProducerPacket*>(data);
        if (!producerPacket)
            Report_Fatal("Subdivide called with null data.");
        
        // If we have been handed a RiArchive as the Subdivided data,
        // Call it blindly and return immediately
        
        if(producerPacket->producerType == kArchive && producerPacket->producerData)
        {
            RiReadArchive( (RtToken)producerPacket->producerData, NULL, RI_NULL);
            return;
        }
        else if(producerPacket->producerType == kObjectInstance)
        {
            HandleObjectInstance(*producerPacket);
            return;
        }
        
        FnScenegraphIterator sgIterator;
        
        // If we've been handed a producer, use it!
        if(producerPacket->sgIterator.isValid())
        {
            sgIterator = producerPacket->sgIterator;
        }
        else if(producerPacket->traverseFromFirstChild)
        {
            // In this instance we'll have been passed the scene graph location
            // and should start traversal from the first child beneath it.
            FnScenegraphIterator parentIterator =\
                GetCachedSgIterator(producerPacket->scenegraphLocation,
                                    &sharedState);

            FnScenegraphIterator firstChild =\
                parentIterator.getFirstChild(true);

            sgIterator = firstChild;
            producerPacket->sgIterator = sgIterator;
        }
        else
        {
            // Otherwise, look up the specified location from the producer cache.
            sgIterator = GetCachedSgIterator(producerPacket->scenegraphLocation, &sharedState);

            // We pass the producer packet not the iterator down to WriteRI so
            // store it back into the packet.
            producerPacket->sgIterator = sgIterator;
        }
        
        if (!sgIterator.isValid())
        {
            Report_Fatal("Could not create a valid scene graph iterator from the specified subdivide data.");
        }
        
        if(!producerPacket->enableStrictVisibilityTesting && !producerPacket->fromImmediateExpansion)
        {
            // Restore the visibility attributes for the procedural if we are not creating a fully
            // expanded debug RIB. If we are creating an expandid RIB we just leave a comment explaining
            // that this is where that process would normally happen within a procedural.
            if( producerPacket->ribDump )
            {
                RiArchiveRecord( "comment", "The visibility attributes are restored here using the user attributes once the procedural is expanded." );
            }
            else
            {
                RxInfoType_t valType;
                int valResultCount;
                
                RtInt visibilitySet = 0;
                if ((0 == RxAttribute("user:katana_visibilitySet", &visibilitySet, sizeof(visibilitySet), &valType, &valResultCount))
                    && visibilitySet)
                {
                    restore_visibility_attribute("int camera", "user:katana_camera");
                    restore_visibility_attribute("int diffuse", "user:katana_diffuse");
                    restore_visibility_attribute("int specular", "user:katana_specular");
                    restore_visibility_attribute("int transmission", "user:katana_transmission");
                    restore_visibility_attribute("int photon", "user:katana_photon");
                    restore_visibility_attribute("int midpoint", "user:katana_midpoint");

                    RtInt falseValue = 0;
                    RtToken tokens[] = { const_cast<char *>("int katana_visibilitySet"), 0 };
                    RtPointer values[] = { &falseValue, 0 };
                    RiAttributeV(const_cast<char *>("user"), 1, tokens, values);
                }
            }
        }
        
        // Remove us from any group membership which is set to false
        if (producerPacket->grouping && producerPacket->groupHasBounds)
        {
            std::ostringstream buffer;
            bool foundOne = false;
            
            for ( ProducerPacket::GroupingMap::const_iterator I = producerPacket->grouping->begin();
                    I!=producerPacket->grouping->end(); ++I)
            {
                if (!((*I).second))
                {
                    buffer << (foundOne ?  "," : "-") << (*I).first;
                    foundOne = true;
                }
            }
            
            if (foundOne)
            {
                RtToken tokens[] = { const_cast<char *>("uniform string membership"), 0 };
                std::string groupNames = buffer.str();
                
                const char * stringValue[] = {groupNames.c_str(), 0};
                RtPointer values[] = { stringValue, 0 };
                RiAttributeV(const_cast<char *>("grouping"), 1, tokens, values);
            }
            
        }

        PRManProcedural::WriteRI(*producerPacket, &sharedState);
        
        Report_Debug("Subdivide exit", &sharedState, sgIterator);
    }
    catch(const std::exception &e)
    {
        std::string errorLocation;
        if(producerPacket && producerPacket->sgIterator.isValid())
        {
            errorLocation = producerPacket->sgIterator.getFullName();
        }
        else if(producerPacket && !producerPacket->scenegraphLocation.empty())
        {
            errorLocation = producerPacket->scenegraphLocation;
        }
        else
        {
            errorLocation = "unknown scenegraph location";
        }
        
        std::ostringstream os;
        os << "Subdivide failed at '";
        os << errorLocation << "'.\n\t";
        os << "Exception reports '" << e.what() << "'.";
        Report_Error(os.str());
    }
    catch(...)
    {
        std::string errorLocation;
        if(producerPacket && producerPacket->sgIterator.isValid())
        {
            errorLocation = producerPacket->sgIterator.getFullName();
        }
        else if(producerPacket && !producerPacket->scenegraphLocation.empty())
        {
            errorLocation = producerPacket->scenegraphLocation;
        }
        else
        {
            errorLocation = "unknown scenegraph location";
        }
        
        std::ostringstream os;
        os << "Subdivide failed at '";
        os << errorLocation << "'.\n\t";
        os << "Un-handled exception.";
        Report_Error(os.str());
    }
}


//////////////////////////////////////////////////////////////////////////////
//
// Free
//
RtVoid Free( RtPointer data )
{

    Report_Debug("Free invoked", &sharedState);

    ProducerPacket* producerPacket = reinterpret_cast<ProducerPacket*>(data);
    if(!producerPacket) return;
    if(producerPacket->neverDelete) return;
    delete producerPacket;
}


void ProducerPacket::copyGroupingFromParent(
        GroupingMapRcPtr parentGrouping,
        FnScenegraphIterator useIterator)
{
    if (!parentGrouping) return;
    if (!useIterator.isValid()) useIterator = sgIterator;
    if (!useIterator.isValid()) return;
    
    grouping = parentGrouping;
    
    FnAttribute::StringAttribute groupingAttr = useIterator.getAttribute("prmanStatements.attributes.grouping.membership");
    
    if (groupingAttr.isValid())
    {
        grouping.reset(new GroupingMap());
        *grouping = *parentGrouping;
        
        std::vector<std::string> workVector, workVector2;
        pystring::split(groupingAttr.getValue(), workVector);
        
        for ( std::vector<std::string>::iterator I = workVector.begin();
                I!=workVector.end(); ++I )
        {
            pystring::split(pystring::strip(*I, ","), workVector2, ",");
            
            for ( std::vector<std::string>::iterator J = workVector2.begin();
                    J!=workVector2.end(); ++J )
            {
                const std::string & entryName = (*J);
                if (entryName.empty()) continue;
                
                bool addTo = true;
                std::string name = entryName;
                
                switch(entryName[0])
                {
                case '-':
                    addTo = false;
                    name = pystring::slice(entryName, 1);
                    break;
                case '+':
                    name = pystring::slice(entryName, 1);
                    break;
                default:
                    break;
                }
                
                if (name.empty()) continue;

                // This will either add or override
                // the grouping
                (*grouping)[name] = addTo;
            }
        }
    }
}

void GetGlobalMembershipAndBuildVector(FnScenegraphIterator & sgIterator, std::vector<std::string> & stringArray)
{
    const FnAttribute::StringAttribute groupingAttr = sgIterator.getAttribute(
            "prmanGlobalStatements.declarations.grouping.membership");

    if (groupingAttr.isValid())
    {
        std::vector<std::string> splitBySpaces;
        std::vector<std::string> splitByCommas;

        pystring::split(groupingAttr.getValue(), splitBySpaces);

        std::vector<std::string>::iterator splitSpacesIt = splitBySpaces.begin();
        for ( ; splitSpacesIt != splitBySpaces.end(); ++splitSpacesIt )
        {
            pystring::split(pystring::strip(*splitSpacesIt, ","), splitByCommas, ",");

            std::vector<std::string>::iterator splitCommasIt = splitByCommas.begin();
            for ( ; splitCommasIt != splitByCommas.end(); ++splitCommasIt )
            {
                std::string entryName = pystring::strip((*splitCommasIt), "+-");
                if (!entryName.empty())
                {
                    stringArray.push_back(entryName);
                }
            }
        }
    }
}

} // namespace PRManProcedural
