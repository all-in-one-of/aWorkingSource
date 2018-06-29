// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef ARNOLD_PLUGIN_SHARED_STATE
#define ARNOLD_PLUGIN_SHARED_STATE

#include <ai.h>
#include <map>
typedef std::map<std::string, AtNode *> CameraMap_t;
typedef std::map<std::string, AtNode*> LightMap_t;
#include <string>
#include <FnRender/plugin/IdSenderFactory.h>
#ifdef _WIN32
#include <memory>
#else
#include <tr1/memory>
#endif
#include <FnAttribute/FnAttribute.h>
#include <boost/thread.hpp>

typedef bool (*OSLShaderExistsFnc_t)(const std::string & name, const std::string & path);
typedef std::string (*GetOSLTypeStringForParameterFnc_t)(const std::string & name,
        const std::string & param, const std::string & path);
typedef std::tr1::shared_ptr<std::vector<unsigned char> > ByteVectorRcPtr_t;
typedef std::map<std::string, ByteVectorRcPtr_t> ShaderIndexMap_t;
typedef std::map<std::string, AtNode*> ArnoldNodeMap_t;
typedef ArnoldNodeMap_t ShadermapType_t;
typedef std::map<std::string, int> NumPolysMap_t;

class ObjectIdState
{
public:
    Foundry::Katana::Render::IdSenderInterface* idSender;
    int64_t nextId;
    int64_t maxId;
    const char* hostNameString;
    int64_t frameID;
    bool generateIds;

    ObjectIdState():
            idSender(0x0),
            nextId(0),
            maxId(0),
            hostNameString(0x0),
            frameID(0),
            generateIds(false)
    {
    }
};


class OSLState
{
public:
    void* oslStubSo;
    bool oslStubChecked;
    OSLShaderExistsFnc_t oslShaderExists;
    GetOSLTypeStringForParameterFnc_t getOslTypeStringForParameter;

    OSLState():
            oslStubSo(0x0),
            oslStubChecked(false),
            oslShaderExists(),
            getOslTypeStringForParameter()
    {
    }
};

class ArnoldPluginState
{
private:
    ObjectIdState objectIdState;
    CameraMap_t cameraMap;
    LightMap_t lightMap;
    LightMap_t lightMapByPath;
    bool rerendering;
    float shutterOpen;
    float shutterClose;
    float cropLeft;
    float cropBottom;
    float cropWidth;
    float cropHeight;
    long xResolution;
    long yResolution;
    float frameTime;
    AtNode* driverKatanaNode;
    ShaderIndexMap_t shaderIndexMap;
    std::string lastLocationPath;
    std::string lastLocationHash;
    ShadermapType_t shaderMap;
    Foundry::Katana::StringAttribute terminalNamesAttr;
    NumPolysMap_t numPolysMap;
    bool objectIDStateInitialized;

    mutable boost::recursive_mutex sharedStateMutex;

public:
    OSLState oslState;

    ArnoldPluginState():
        objectIdState(),
        cameraMap(),
        lightMap(),
        lightMapByPath(),
        rerendering(false),
        shutterOpen(0.0f),
        shutterClose(0.0f),
        cropLeft(0.0f),
        cropBottom(1.0f),
        cropWidth(0.0f),
        cropHeight(1.0f),
        xResolution(0),
        yResolution(0),
        frameTime(0.0f),
        driverKatanaNode(0x0),
        shaderIndexMap(),
        lastLocationPath(),
        lastLocationHash(),
        shaderMap(),
        terminalNamesAttr(),
        numPolysMap(),
        objectIDStateInitialized(false),
        oslState()
    {
    }

    ArnoldPluginState& operator=(const ArnoldPluginState &rhs)
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

        objectIdState     = rhs.objectIdState;
        cameraMap         = rhs.cameraMap;
        lightMap          = rhs.lightMap;
        lightMapByPath    = rhs.lightMapByPath;
        rerendering     = rhs.rerendering;
        shutterOpen       = rhs.shutterOpen;
        shutterClose      = rhs.shutterClose;
        cropLeft          = rhs.cropLeft;
        cropBottom        = rhs.cropBottom;
        cropWidth         = rhs.cropWidth;
        cropHeight        = rhs.cropHeight;
        xResolution       = rhs.xResolution;
        yResolution       = rhs.yResolution;
        frameTime         = rhs.frameTime;
        driverKatanaNode  = rhs.driverKatanaNode;
        oslState          = rhs.oslState;
        shaderIndexMap    = rhs.shaderIndexMap;
        lastLocationPath  = rhs.lastLocationPath;
        lastLocationHash  = rhs.lastLocationHash;
        shaderMap         = rhs.shaderMap;
        terminalNamesAttr = rhs.terminalNamesAttr;
        numPolysMap       = rhs.numPolysMap;
        objectIDStateInitialized = rhs.objectIDStateInitialized;
        return *this;
    }

    ByteVectorRcPtr_t getShaderIndex(const std::string &location) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        ShaderIndexMap_t::const_iterator shaderIdIt = shaderIndexMap.find(location);
        if (shaderIdIt != shaderIndexMap.end())
        {
            ByteVectorRcPtr_t shaderId = shaderIdIt->second;
            return shaderId;
        }

        return ByteVectorRcPtr_t();
    }

    void setShaderIndex(const std::string &location, ByteVectorRcPtr_t shaderId)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        shaderIndexMap[location] = shaderId;
    }

    AtNode* getShader(const std::string &location) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        ShadermapType_t::const_iterator shaderIt = shaderMap.find(location);
        if (shaderIt != shaderMap.end())
        {
            AtNode* shaderNode = shaderIt->second;
            return shaderNode;
        }

        return NULL;
    }

    void setShader(const std::string &location, AtNode* shaderNode)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        shaderMap[location] = shaderNode;
    }

    int getNumPolys(const std::string &location) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        NumPolysMap_t::const_iterator numPolysIt = numPolysMap.find(location);
        if (numPolysIt != numPolysMap.end())
        {
            int numPolys = numPolysIt->second;
            return numPolys;
        }

        return -1;
    }

    void setNumPolys(const std::string &location, int numPolys)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        numPolysMap[location] = numPolys;
    }

    AtNode* getCameraLocationByPath(const std::string &location) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        CameraMap_t::const_iterator cam = cameraMap.find(location);
        if (cam != cameraMap.end())
        {
            AtNode* cameraNode = cam->second;
            return cameraNode;
        }

        return NULL;
    }

    void setCameraLocation(const std::string &location, AtNode* cameraNode)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        cameraMap[location] = cameraNode;
    }

    AtNode* getLightHandleForLocation(const std::string &location) const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        LightMap_t::const_iterator lgt = lightMapByPath.find(location);
        if (lgt != lightMapByPath.end())
        {
            AtNode* lightHandle = lgt->second;
            return lightHandle;
        }

        return NULL;
    }

    void setLightHandle(const std::string &name, AtNode* lightHandle)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        lightMap[name] = lightHandle;
    }

    void setLightHandleForLocation(const std::string &location, AtNode* lightHandle)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        lightMapByPath[location] = lightHandle;
    }

    int sizeOfLightMapByPath() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return (int)lightMapByPath.size();
    }

    void duplicateLightMapByPath(LightMap_t &newLightMap)
    {
        // Acquire the lock, then copy entries from lightMapByPath
        // in to newLightMap to prevent it from chaning benieth us.
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        for (LightMap_t::const_iterator iter = lightMapByPath.begin();
                iter != lightMapByPath.end(); ++iter)
        {
            newLightMap[iter->first] =  (*iter).second;
        }
    }

    void initializeObjectIdState(const char *hostname, long frameID)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        if(objectIDStateInitialized) return;

        objectIdState.hostNameString = hostname;
        objectIdState.frameID = frameID;
        objectIdState.generateIds = true;

        // Try to connect.
        if (!objectIdState.hostNameString) return;
        if (!objectIdState.frameID) return;

        objectIdState.idSender = FnKat::Render::IdSenderFactory::getNewInstance(std::string(objectIdState.hostNameString), objectIdState.frameID);
        objectIdState.idSender->getIds(&objectIdState.nextId, &objectIdState.maxId);

        objectIDStateInitialized = true;
    }

    void shutdownObjectId()
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        if (objectIdState.idSender != 0)
        {
            AiMsgInfo("[kat] Closing the socket for ObjectId transmission.");
            delete objectIdState.idSender;
            objectIdState.idSender = 0;
        }
    }

    bool isIdWritingEnabled() const
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

    boost::recursive_mutex::scoped_lock getScopedLock()
    {
        return boost::recursive_mutex::scoped_lock(sharedStateMutex);
    }

    void setDriverKatanaNode(AtNode* node)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        driverKatanaNode = node;
    }
    void setIsRerendering(bool value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        rerendering = value;
    }
    void setShutterOpen(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        shutterOpen = value;
    }
    void setShutterClose(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        shutterClose = value;
    }
    void setCropLeft(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        cropLeft = value;
    }
    void setCropBottom(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        cropBottom = value;
    }
    void setCropWidth(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        cropWidth = value;
    }
    void setCropHeight(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        cropHeight = value;
    }
    void setXResolution(long value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        xResolution = value;
    }
    void setYResolution(long value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        yResolution = value;
    }
    void setFrameTime(float value)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        frameTime = value;
    }

    AtNode*  getDriverKatanaNode() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return driverKatanaNode;
    }
    bool isRerendering() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return rerendering;
    }
    float getShutterOpen() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return shutterOpen;
    }
    float getShutterClose() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return shutterClose;
    }
    float getCropLeft() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return cropLeft;
    }
    float getCropBottom() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return cropBottom;
    }
    float getCropWidth() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return cropWidth;
    }
    float getCropHeight() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return cropHeight;
    }
    long getXResolution() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return xResolution;
    }
    long getYResolution() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return yResolution;
    }
    float getFrameTime() const
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);
        return frameTime;
    }
    Foundry::Katana::StringAttribute getTerminalNamesAttr()
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        if (!terminalNamesAttr.isValid())
        {
            std::vector<std::string> sb;
            sb.push_back("arnoldSurface");
            sb.push_back("arnoldBump");
            sb.push_back("arnoldDisplacement");
            sb.push_back("arnoldGI_Quickshade");
            sb.push_back("arnoldLight");
            sb.push_back("arnoldLightFilter");
            terminalNamesAttr = FnAttribute::StringAttribute(sb);
        }

        return terminalNamesAttr;
    }

    std::string updateLastLocationPath(const std::string& locationPath, const std::string& locationHash)
    {
        boost::recursive_mutex::scoped_lock lock(sharedStateMutex);

        if (locationPath != lastLocationPath)
        {
            lastLocationHash = locationHash;
            lastLocationPath = locationPath;
        }
        return lastLocationHash;
    }


};

/**
 * Helper class to pass information and state to the delegates through RenderOutputUtils::processLocation() down to ScenegraphLocationDelegate::process()
 */
class ArnoldSceneGraphLocationDelegateInput
{
public:
    ArnoldPluginState* sharedState;
    std::vector<AtNode*>* childNodes;
};

#endif
