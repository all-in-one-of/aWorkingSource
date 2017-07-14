// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANPLUGINSTATE_H_
#define PRMANPLUGINSTATE_H_

#include <ri.h>
#include <rx.h>
#include <map>
#ifdef _WIN32
#include <hash_map>
#else
#include <tr1/unordered_map>
#endif
#include <FnRender/plugin/IdSenderFactory.h>

#ifdef _WIN32
typedef stdext::hash_map<std::string, std::string> AttributeTypeMap;
#else
// Use a hash map for this, because so many of the strings will be so similar,
// and because there will be so many strings in the map.
// This *could* be replaced with a regular map, with only a performance hit
// (that we've not measured yet).
typedef std::tr1::unordered_map<std::string, std::string> AttributeTypeMap;
#endif //_WIN32

#include <boost/thread.hpp>

struct ProceduralSettings
{
    // Serialized Geolib3 optree file to read
    std::string optreeFilename;

    // Print profiling information?
    bool  printTimerInfo;

    // Force expansion of all geometry?
    bool  fullRecurse;
    float frameNumber;
    float shutterOpenTime;
    float shutterCloseTime;
    float cropWindow[4];

    int xres;
    int yres;

    // Debug printout stream
    std::ostream* debugStream;

    // Producer cache settings
    bool limitProducerCaching;
    unsigned int geometricProducerCacheLimit;
    unsigned int geometricCacheInputQueueLength;
    unsigned int producerInstanceCacheInterval;
    bool printCacheStatistics_debug;
    bool printCacheStatistics_summary;
    bool ribDump;
    bool useIDPass;

    ProceduralSettings():
        optreeFilename(),
        printTimerInfo(false),
        fullRecurse(false),
        frameNumber(0.0f),
        shutterOpenTime(0.0f),
        shutterCloseTime(0.0f),
        cropWindow(),
        xres(0),
        yres(0),
        debugStream(0x0),
        limitProducerCaching(false),
        geometricProducerCacheLimit(256),
        geometricCacheInputQueueLength(64),
        producerInstanceCacheInterval(32),
        printCacheStatistics_debug(false),
        printCacheStatistics_summary(true),
        ribDump(false),
        useIDPass(false)
        {}
};

class ObjectIdState
{
public:
    int isWritingEnabled;
    Foundry::Katana::Render::IdSenderInterface* idSender;
    int64_t nextId;
    int64_t maxId;

    ObjectIdState() : isWritingEnabled(-1), idSender(0x0), nextId(0), maxId(0)
    {
    }
};

class PRManPluginState
{
public:
    ObjectIdState objectIdState;
    ProceduralSettings proceduralSettings;

    PRManPluginState()
      : objectIdState(),
        proceduralSettings(),
        scenegraphLightHandleMap(),
        attributeTypeMap(),
        objectIDStateInitialized(false),
        sharedStateMutex()
    {
    }

    PRManPluginState& operator=(const PRManPluginState &rhs)
    {
        if(this == &rhs)
            return *this;

        boost::unique_lock<boost::recursive_mutex>
            l1(sharedStateMutex, boost::defer_lock);
        boost::unique_lock<boost::recursive_mutex>
            l2(rhs.sharedStateMutex, boost::defer_lock);

        // boost::lock() will lock the given mutices in an order invariant
        // across all threads thereby ensuring deadlock cannot occur.
        boost::lock(l1, l2);

        objectIdState = rhs.objectIdState;
        proceduralSettings = rhs.proceduralSettings;
        scenegraphLightHandleMap = rhs.scenegraphLightHandleMap;
        attributeTypeMap = rhs.attributeTypeMap;
        objectIDStateInitialized = rhs.objectIDStateInitialized;
        return *this;
    }

    void setAttributeType(const std::string &key,
                          const std::string &typestr)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        attributeTypeMap[key] = typestr;
    }

    std::string getAttributeType(const std::string &key) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        AttributeTypeMap::const_iterator iter = attributeTypeMap.find(key);
        if (iter != attributeTypeMap.end())
        {
            return (*iter).second;
        }

        return std::string();
    }

    RtLightHandle getLightHandleForLocation(const std::string &location) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        std::map<std::string, RtLightHandle>::const_iterator lightHandle =
            scenegraphLightHandleMap.find( location );
        if( lightHandle == scenegraphLightHandleMap.end() )
            return RI_NULL;

        return lightHandle->second;
    }

    void setLightHandleForLocation(const std::string &location, RtLightHandle lightHandle)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        scenegraphLightHandleMap[location] = lightHandle;
    }

    void initializeObjectIdState()
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        if(objectIDStateInitialized) return;

        RxInfoType_t attr_type;
        int attr_count;

        long writeIDValue = 0;
        if ((RxAttribute("user:ProceduralGenerateID", &writeIDValue, sizeof(long), &attr_type, &attr_count) == 0)
            && (attr_type == RxInfoInteger) && (attr_count == 1) && (writeIDValue != 0))
        {
           objectIdState.isWritingEnabled = 1;
        }
        else
        {
            objectIdState.isWritingEnabled = 0;
            // Nothing left for us to do.
            return;
        }

        // Now set up the Id sender.
        // Try to connect.
        char* hostNameString = NULL;

        if ((0 != RxAttribute("user:ProceduralHostName", &hostNameString, sizeof(char*), &attr_type, &attr_count)) ||
            (attr_type != RxInfoStringV) || (attr_count != 1) || (!hostNameString))
        {
            // If we couldn't find a host name then we consider ID pass
            // disabled.
            objectIdState.isWritingEnabled = 0;
            return;
        }

        long frameID = 0;
        if ((0 != RxAttribute("user:ProceduralFrameID", &frameID, sizeof(long), &attr_type, &attr_count)) ||
            (attr_type != RxInfoInteger) || (attr_count != 1))
        {
            // If we don't known what frame we're rendering we consider
            // ID pass disabled.
            objectIdState.isWritingEnabled = 0;
            return;
        }

        objectIdState.idSender =
            FnKat::Render::IdSenderFactory::getNewInstance(std::string(hostNameString), frameID);
        objectIdState.idSender->getIds(&objectIdState.nextId,
                                       &objectIdState.maxId);

        objectIDStateInitialized = true;
    }

    bool isIdWritingEnabled()
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        return objectIDStateInitialized;
    }

    // Returns the next ID to allocate to a particular object in the
    // scenegraph.
    // Returns 0 if the ID allocation has exceeded the maximum allocation
    // provided by Katana.
    int64_t getNextId()
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        int64_t id_value = objectIdState.nextId;
        if(id_value >= objectIdState.maxId)
        {
            id_value = 0;
        }
        else
        {
            ++objectIdState.nextId;
        }

        return id_value;
    }

    void sendIdAssignment(int64_t id_value, const char *objectName)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        objectIdState.idSender->send(id_value, objectName);
    }

private:
    std::map<std::string, RtLightHandle> scenegraphLightHandleMap;
    AttributeTypeMap attributeTypeMap;
    bool objectIDStateInitialized;

    mutable boost::recursive_mutex sharedStateMutex;
};

/**
 * Helper class to pass information and state to the delegates through
 * RenderOutputUtils::processLocation() down to
 * ScenegraphLocationDelegate::process().
 *
 * Although in this case there only the PRManPluginState is included we
 * follow the same pattern as the Arnold plugins.
 */
class PRManSceneGraphLocationDelegateInput
{
public:
    PRManPluginState* sharedState;
};

#endif
