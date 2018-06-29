#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <pystring/pystring.h>

#include <FnPluginSystem/FnPlugin.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolib/op/FnGeolibCookInterface.h>

#include <FnGeolib/util/AttributeKeyedCache.h>

#include "AbcCook.h"
#include "ArrayPropUtils.h"
#include "ArbitraryGeomParamUtils.h"
#include "ScalarPropUtils.h"
#include <Alembic/AbcCoreFactory/All.h>

#include <hdf5.h>  // for H5dont_atexit

namespace { // anonymous

class PathMap
{
public:
    PathMap()
    {
    };

    ~PathMap()
    {
    }

    AlembicIn::AbcCookPtr get(const std::string & iFullName,
                              Alembic::Abc::IObject & iParent,
                              const std::string & iName)
    {
        FnGeolibUtil::AutoMutex lock(mutex);
        std::map<std::string, AlembicIn::AbcCookPtr>::iterator it =
            pathMap.find(iFullName);
        if (it != pathMap.end())
        {
            return it->second;
        }

        AlembicIn::AbcCookPtr cookPtr(new AlembicIn::AbcCook());
        cookPtr->objPtr =
            Alembic::Abc::IObjectPtr(new Alembic::Abc::IObject(iParent, iName));
        pathMap[iFullName] = cookPtr;
        return cookPtr;
    }

private:
    std::map<std::string, AlembicIn::AbcCookPtr> pathMap;
    FnGeolibUtil::Mutex mutex;
};

typedef std::tr1::shared_ptr< PathMap > PathMapPtr;

struct ArchiveAndFriends
{
    ArchiveAndFriends(Alembic::Abc::IArchive & iArchive,
                      Alembic::AbcCoreFactory::IFactory::CoreType iCoreType) :
        pathMap(new PathMap())
    {
        objArchive = iArchive.getTop();
        coreType = iCoreType;
    }

    Alembic::Abc::IObject objArchive;
    Alembic::AbcCoreFactory::IFactory::CoreType coreType;
    PathMapPtr pathMap;

};

class AlembicCache :
    public FnGeolibUtil::AttributeKeyedCache< ArchiveAndFriends >
{
public:
    AlembicCache() :
        FnGeolibUtil::AttributeKeyedCache< ArchiveAndFriends >(100, 1000)
    {
        factory.setPolicy(Alembic::Abc::ErrorHandler::kQuietNoopPolicy);
    }

    // number of streams to open per file
    void setNumStreams( std::size_t iNumStreams )
    {
        factory.setOgawaNumStreams( iNumStreams );
    }

private:
    AlembicCache::IMPLPtr createValue(const FnAttribute::Attribute & iAttr)
    {
        AlembicCache::IMPLPtr val;
        Alembic::AbcCoreFactory::IFactory::CoreType coreType;

        Alembic::Abc::IArchive archive = factory.getArchive(
            FnAttribute::StringAttribute(iAttr).getValue(), coreType);

        if ( archive.valid() )
        {
            val.reset(new ArchiveAndFriends(archive, coreType));
        }

        return val;
    }

    Alembic::AbcCoreFactory::IFactory factory;
};

AlembicCache g_cache;

class AlembicInPrivateData : public Foundry::Katana::GeolibPrivateData
{
public:
    AlembicInPrivateData()
        : Foundry::Katana::GeolibPrivateData()
    {
    }

    virtual ~AlembicInPrivateData(){};

    AlembicIn::AbcCookPtr cookPtr;
    PathMapPtr pathMap;
};

AlembicIn::AbcCookPtr getTopObject(const FnAttribute::StringAttribute & iAttr,
    const FnAttribute::StringAttribute & iRootAttr, std::string & oOpType,
    PathMapPtr & oPathMap)
{
    AlembicIn::AbcCookPtr retVal;
    Alembic::Abc::IObject obj;
    AlembicCache::IMPLPtr entry = g_cache.getValue(iAttr);

    if (!entry || !entry->objArchive.valid())
    {
        return retVal;
    }

    obj = entry->objArchive;

    std::string pathToRoot;
    if (iRootAttr.isValid())
    {
        pathToRoot = iRootAttr.getValue();
    }

    if (!pathToRoot.empty())
    {
        std::vector< std::string > tokens;
        pathToRoot = pystring::strip(pathToRoot, " /\n\t");
        pystring::split(pathToRoot, tokens, "/");
        std::vector< std::string >::const_iterator i;
        for (i = tokens.begin(); i != tokens.end(); ++i)
        {
            if (!obj.getChildHeader(*i))
            {
                return retVal;
            }
            obj = obj.getChild(*i);
        }
    }

    // TODO, do something with entry->coreType
    if (entry->coreType == Alembic::AbcCoreFactory::IFactory::kOgawa)
    {
        oOpType = "AlembicInOgawa";
    }
    else if (entry->coreType == Alembic::AbcCoreFactory::IFactory::kHDF5)
    {
        oOpType = "AlembicInHDF";
    }

    retVal = AlembicIn::AbcCookPtr(new AlembicIn::AbcCook());
    retVal->objPtr = Alembic::Abc::IObjectPtr(new Alembic::Abc::IObject(obj));

    if (retVal->objPtr->valid())
    {
        Alembic::Abc::ICompoundProperty prop = retVal->objPtr->getProperties();
        const Alembic::Abc::PropertyHeader * childBnds =
            prop.getPropertyHeader(".childBnds");
        if (childBnds)
        {
            FnAttribute::GroupBuilder staticGb;
            scalarPropertyToAttr(prop, *childBnds, "bound", retVal, staticGb);
            retVal->staticGroup = staticGb.build();
        }
    }

    oPathMap = entry->pathMap;
    return retVal;
}


// Report fatal errors to the scene graph and halt traversal.
static void reportAlembicError(Foundry::Katana::GeolibCookInterface &interface,
                               const std::string &filePath,
                               const std::string &errorMessage)
{
    interface.setAttr("type", FnAttribute::StringAttribute("error"));
    interface.setAttr("errorMessage",
                      FnAttribute::StringAttribute("Can not load: " +
                                                   filePath + " : " +
                                                   errorMessage));
    interface.stopChildTraversal();
    return;
}

// Extracts bookkeeping information from an AbcCook pointer and sets them
// under the 'info' attribute at the root location.
static void fillRootInfo(Foundry::Katana::GeolibCookInterface &interface,
                         AlembicIn::AbcCookPtr ioCookPtr)
{
    if (!ioCookPtr->objPtr || ioCookPtr->objPtr->getParent().valid())
    {
        return;
    }

    double fps = FnAttribute::DoubleAttribute(interface.getOpArg(
        "fps")).getValue(24.0, false);

    std::vector<double> frameRange(2);
    frameRange[0] = DBL_MAX;
    frameRange[1] = -DBL_MAX;

    Alembic::Abc::IArchive archive = ioCookPtr->objPtr->getArchive();
    uint32_t numSampling = archive.getNumTimeSamplings();

    // ignore the default time sampling at index 0
    for (uint32_t i = 1; i < numSampling; ++i)
    {
        Alembic::AbcCoreAbstract::index_t maxIndex =
            archive.getMaxNumSamplesForTimeSamplingIndex(i);

        if (maxIndex < 2 || maxIndex == INDEX_UNKNOWN)
        {
            continue;
        }

        Alembic::AbcCoreAbstract::TimeSamplingPtr ts =
            archive.getTimeSampling(i);

        std::vector<double> samples(maxIndex);
        for (Alembic::AbcCoreAbstract::index_t j = 0; j < maxIndex; ++j)
        {
            samples[j] = ts->getSampleTime(j) * fps;
        }

        if (samples.front() < frameRange[0])
        {
            frameRange[0] = samples.front();
        }

        if (samples.back() < frameRange[0])
        {
            frameRange[0] = samples.back();
        }

        if (samples.front() > frameRange[1])
        {
            frameRange[1] = samples.front();
        }

        if (samples.back() > frameRange[1])
        {
            frameRange[1] = samples.back();
        }

        std::stringstream strm;
        strm << "info.frameSamples" << i;
        interface.setAttr(strm.str().c_str(), FnAttribute::DoubleAttribute(
            &(samples.front()), samples.size(), 1));
    }

    if (frameRange[0] < frameRange[1])
    {
        interface.setAttr("info.frameRange", FnAttribute::DoubleAttribute(
            &(frameRange.front()), frameRange.size(), 1));
    }
}

// Builds and returns a GroupAttribute from an AbcCookPtr with all its
// static and non-static properties.
static FnAttribute::GroupAttribute getAllProps(
    const Foundry::Katana::GeolibCookInterface &interface,
    const AlembicIn::AbcCookPtr &ioCookPtr)
{
    if (!ioCookPtr->arrayProps.empty() || !ioCookPtr->scalarProps.empty() ||
        ioCookPtr->objPtr || ioCookPtr->visProp.valid())
    {
        // let's get our other args which we will need for our animated
        // reads
        AlembicIn::OpArgs args;
        args.currentTime = FnAttribute::FloatAttribute(interface.getOpArg(
            "system.timeSlice.currentTime")).getValue(0.0, false);
        args.shutterOpen = FnAttribute::FloatAttribute(interface.getOpArg(
            "system.timeSlice.shutterOpen")).getValue(0.0, false);
        args.shutterClose = FnAttribute::FloatAttribute(interface.getOpArg(
            "system.timeSlice.shutterClose")).getValue(0.0, false);
        args.numSamples = FnAttribute::IntAttribute(interface.getOpArg(
            "system.timeSlice.numSamples")).getValue(0, false);

        args.fps = FnAttribute::DoubleAttribute(interface.getOpArg(
            "fps")).getValue(24.0, false);

        std::string beyondRange =
            FnAttribute::StringAttribute(interface.getOpArg(
                "beyondRangeBehavior")).getValue("", false);

        if (beyondRange == "error")
        {
            args.behavior = AlembicIn::OpArgs::kError;
        }
        else if (beyondRange == "hold")
        {
            args.behavior = AlembicIn::OpArgs::kHold;
        }

        const int useOnlyShutterOpenCloseTimesFlag =
            FnAttribute::IntAttribute(interface.getOpArg(
                "useOnlyShutterOpenCloseTimes")).getValue(0, false);
        args.useOnlyShutterOpenCloseTimes =
            (useOnlyShutterOpenCloseTimesFlag == 1);

        FnAttribute::GroupBuilder bld;
        bld.deepUpdate(ioCookPtr->staticGroup);

        for (std::vector< AlembicIn::ArrayProp >::iterator at =
             ioCookPtr->arrayProps.begin();
             at != ioCookPtr->arrayProps.end(); ++at)
        {
            AlembicIn::arrayPropertyToAttr(*at, args, bld);
        }

        for (std::vector< AlembicIn::ScalarProp >::iterator st =
             ioCookPtr->scalarProps.begin();
             st != ioCookPtr->scalarProps.end(); ++st)
        {
            AlembicIn::scalarPropertyToAttr(*st, args, bld);
        }

        for (std::vector< AlembicIn::IndexedGeomParamPair >::iterator ft =
             ioCookPtr->forcedExpandProps.begin();
             ft != ioCookPtr->forcedExpandProps.end(); ++ft)
        {
            AlembicIn::indexedParamToAttr(*ft, args, bld);
        }

        if (ioCookPtr->animatedSchema)
        {
            AlembicIn::evalObject(ioCookPtr, args, bld);
        }

        return bld.build();

    }
    // no animation set our static attrs
    return ioCookPtr->staticGroup;
}

static void setAllAttrs(Foundry::Katana::GeolibCookInterface &interface,
                        const FnAttribute::GroupAttribute &attrs)
{
    for (int64_t i = 0; i < attrs.getNumberOfChildren(); ++i)
    {
        interface.setAttr(attrs.getChildName(i), attrs.getChildByIndex(i));
    }
}

// Utility function to handle extracting all the properties from an AbcCookPtr
// as a GroupAttribute and setting them on the current location.
static void fillAllProps(Foundry::Katana::GeolibCookInterface &interface,
                         AlembicIn::AbcCookPtr ioCookPtr)
{
    fillRootInfo(interface, ioCookPtr);
    FnAttribute::GroupAttribute allProps = getAllProps(interface, ioCookPtr);
    setAllAttrs(interface, allProps);
}

static void cookAlembic(Foundry::Katana::GeolibCookInterface &interface,
                        const std::string & iOpType,
                        AlembicIn::AbcCookPtr ioCookPtr,
                        PathMapPtr & iPathMap)
{
    if (!ioCookPtr->staticGroup.isValid())
    {
        FnGeolibUtil::AutoMutex lock(ioCookPtr->mutex);

        if (!ioCookPtr->staticGroup.isValid())
        {
            FnAttribute::GroupBuilder staticBld;
            AlembicIn::initAbcCook(ioCookPtr, staticBld);
            ioCookPtr->staticGroup = staticBld.build();
        }
    }

    fillAllProps(interface, ioCookPtr);

    // Check if we need to add "forceExpand".
    FnAttribute::IntAttribute addForceExpandAttr =
            interface.getOpArg("addForceExpand");

    // In case we found "addForceExpand" we have to remove it from the opArgs
    // so that it will be set only by the first-level children
    FnAttribute::Attribute childOpArgs = interface.getOpArg();

    if (addForceExpandAttr.isValid() && addForceExpandAttr.getValue(1, false))
    {
        interface.setAttr("forceExpand", FnAttribute::IntAttribute(1));

        FnAttribute::GroupBuilder newOpArgs;
        newOpArgs.update(childOpArgs);
        newOpArgs.del("addForceExpand");
        childOpArgs = newOpArgs.build();
    }

    // Check wether we need to copy the bounds from our parent
    FnAttribute::DoubleAttribute parentBound =
        interface.getOpArg("parentBound");
    if (parentBound.isValid())
    {
        FnAttribute::GroupBuilder boundsFromParentOpArgsGb;
        boundsFromParentOpArgsGb.set("parentBound", parentBound);
        interface.execOp("BoundsFromParent", boundsFromParentOpArgsGb.build());

        // Remove the parent bound from the OpArgs since we do not want the
        // grandchildren to set it.
        FnAttribute::GroupBuilder newOpArgs;
        newOpArgs.update(childOpArgs);
        newOpArgs.del("parentBound");
        childOpArgs = newOpArgs.build();
    }

    std::size_t numChildren = ioCookPtr->objPtr->getNumChildren();

    for (std::size_t i = 0; i < numChildren; ++i)
    {
        const Alembic::AbcCoreAbstract::ObjectHeader & header =
            ioCookPtr->objPtr->getChildHeader(i);

        AlembicInPrivateData * childData = new AlembicInPrivateData();

        childData->pathMap = iPathMap;
        childData->cookPtr = iPathMap->get(header.getFullName(),
            *ioCookPtr->objPtr, header.getName());

        interface.createChild(header.getName(), iOpType, childOpArgs,
            Foundry::Katana::GeolibCookInterface::ResetRootAuto, childData,
            AlembicInPrivateData::Delete);
    }
}

class AlembicInHDFOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeGlobalUnsafe);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        AlembicInPrivateData *privateData =
            static_cast<AlembicInPrivateData *>(interface.getPrivateData());

        if (!privateData || !privateData->cookPtr || !privateData->pathMap)
        {
            interface.stopChildTraversal();
            return;
        }

        cookAlembic(interface, "AlembicInHDF", privateData->cookPtr,
                    privateData->pathMap);
    }
};

class AlembicInOgawaOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        AlembicInPrivateData *privateData =
            static_cast<AlembicInPrivateData *>(interface.getPrivateData());

        if (!privateData || !privateData->cookPtr || !privateData->pathMap)
        {
            interface.stopChildTraversal();
            return;
        }

        cookAlembic(interface, "AlembicInOgawa", privateData->cookPtr,
                    privateData->pathMap);
    }
};

class AlembicInOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        // FIXME: Should this top level Alembic op being more optimistic?
        // (I.e., concurrent).  For the pure ogawa situation, it would be
        // nice if ThreadModeGlobalUnsafe was *never* encountered by the runtime.
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeGlobalUnsafe);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        FnAttribute::StringAttribute fileAttr = interface.getOpArg("fileName");

        FnAttribute::StringAttribute pathFromRootAttr =
            interface.getOpArg("pathFromRoot");

        if (!fileAttr.isValid() || fileAttr.getValue() == "")
        {
            interface.setAttr("type", FnAttribute::StringAttribute("error"));
            interface.setAttr("errorMessage",
                FnAttribute::StringAttribute("No file specified"));
            interface.stopChildTraversal();
            return;
        }

        PathMapPtr pathMap;
        std::string opType;

        AlembicIn::AbcCookPtr curObj =
            getTopObject(fileAttr, pathFromRootAttr, opType, pathMap);

        const std::string filePath = fileAttr.getValue();
        // Check and report our error conditions
        if(!curObj)
        {
            reportAlembicError(interface, filePath, "Top-level object was NULL");
            return;
        }

        if(!curObj->objPtr)
        {
            reportAlembicError(interface, filePath, "Got top-level object but objPtr was NULL");
            return;
        }

        if(!curObj->objPtr->valid())
        {
            reportAlembicError(interface, filePath, "Top-level objPtr was invalid");
            return;
        }

        if(opType == "")
        {
            reportAlembicError(interface, filePath, "Specified opType was empty");
            return;
        }

        if(!pathMap)
        {
            reportAlembicError(interface, filePath, "Path map was NULL");
            return;
        }

        // Check if we need to set bounds. On the root we can set it here,
        // for the children we pass it as opArg for them to deal with it.
        std::string addBounds = FnAttribute::StringAttribute(
            interface.getOpArg("addBounds")).getValue("none", false);
        FnAttribute::GroupAttribute childOpArgs = interface.getOpArg();

        if (addBounds != "none")
        {
            fillRootInfo(interface, curObj);
            FnAttribute::GroupAttribute allProps = getAllProps(interface,
                                                               curObj);

            FnAttribute::DoubleAttribute rootBound =
                allProps.getChildByName("bound");

            if (rootBound.isValid()
                && (addBounds == "both" || addBounds == "children"))
            {
                // If we want the children to inherit the bound we need to
                // pass it via the OpArgs
                FnAttribute::GroupBuilder gb;
                gb.update(childOpArgs);
                gb.set("parentBound", rootBound);
                childOpArgs = gb.build();
            }

            if (addBounds == "children")
            {
                // remove bound from the attribute to set
                FnAttribute::GroupBuilder gb;
                gb.update(allProps);
                gb.del("bound");
                allProps = gb.build();
            }
            setAllAttrs(interface, allProps);
        }

        // invoke the children
        std::size_t numChildren = curObj->objPtr->getNumChildren();

        for (std::size_t i = 0; i < numChildren; ++i)
        {
            const Alembic::AbcCoreAbstract::ObjectHeader & header =
                curObj->objPtr->getChildHeader(i);

            AlembicInPrivateData * childData = new AlembicInPrivateData();

            childData->pathMap = pathMap;
            childData->cookPtr = pathMap->get(header.getFullName(),
                *curObj->objPtr, header.getName());

            interface.createChild(header.getName(), opType,
                childOpArgs,
                Foundry::Katana::GeolibCookInterface::ResetRootAuto, childData,
                AlembicInPrivateData::Delete);
        }
    }

    static void flush()
    {
        g_cache.clear();
    }
};

DEFINE_GEOLIBOP_PLUGIN(AlembicInOp)
DEFINE_GEOLIBOP_PLUGIN(AlembicInOgawaOp)
DEFINE_GEOLIBOP_PLUGIN(AlembicInHDFOp)


class CameraAndLightPathCache :
    public FnGeolibUtil::AttributeKeyedCache< FnAttribute::GroupAttribute >
{
private:
    CameraAndLightPathCache::IMPLPtr createValue(const FnAttribute::Attribute & iAttr)
    {
        CameraAndLightPathCache::IMPLPtr val(new FnAttribute::GroupAttribute);
        Alembic::AbcCoreFactory::IFactory factory;


        Alembic::AbcCoreFactory::IFactory::CoreType coreType;

        Alembic::Abc::IArchive archive = factory.getArchive(
                FnAttribute::StringAttribute(iAttr).getValue("", false),
                        coreType);

        if (archive.valid())
        {
            std::vector<std::string> cameras, lights;
            walk(archive.getTop(), cameras, lights);

            val.reset(new FnAttribute::GroupAttribute(
                    "cameras", FnAttribute::StringAttribute(cameras, 1),
                    "lights", FnAttribute::StringAttribute(lights, 1),
                            true));
        }

        return val;
    }

    void walk(Alembic::Abc::IObject object, std::vector<std::string> & cameras,
            std::vector<std::string> & lights)
    {
        if (!object.valid())
        {
            return;
        }

        if (Alembic::AbcGeom::ICamera::matches(object.getHeader()))
        {
            cameras.push_back(object.getFullName());
        }
        else if (Alembic::AbcGeom::ILight::matches(object.getHeader()))
        {
            lights.push_back(object.getFullName());
        }

        for (size_t i = 0, e = object.getNumChildren(); i < e; ++i)
        {
            walk(object.getChild(i), cameras, lights);
        }
    }
};

CameraAndLightPathCache g_cameraAndLightCache;



class AlembicInAddToLightAndCameraListsOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeGlobalUnsafe);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        interface.stopChildTraversal();

        FnAttribute::GroupAttribute pathsGroup =
                *(g_cameraAndLightCache.getValue(interface.getOpArg("fileName")));

        if (!pathsGroup.isValid())
        {
            return;
        }

        std::string scenegraphPath = FnAttribute::StringAttribute(
                interface.getOpArg("scenegraphPath")).getValue("/root", false);

        FnAttribute::StringAttribute camerasAttr =
                pathsGroup.getChildByName("cameras");

        if (camerasAttr.isValid() && camerasAttr.getNumberOfValues())
        {
            std::vector<std::string> cameraList;
            cameraList.reserve(camerasAttr.getNumberOfValues());

            FnAttribute::StringAttribute::array_type values =
                    camerasAttr.getNearestSample(0.0);

            for (FnAttribute::StringAttribute::array_type::const_iterator I =
                    values.begin(), E = values.end(); I != E; ++I)
            {
                cameraList.push_back(scenegraphPath + (*I));
            }

            interface.extendAttr("globals.cameraList",
                    FnAttribute::StringAttribute(cameraList), "", false);
        }

        //TODO, lightList if desired
    }
};

DEFINE_GEOLIBOP_PLUGIN(AlembicInAddToLightAndCameraListsOp)






} // anonymous

void registerPlugins()
{
    H5dont_atexit();
    REGISTER_PLUGIN(AlembicInOp, "AlembicIn", 0, 1);
    REGISTER_PLUGIN(AlembicInOgawaOp, "AlembicInOgawa", 0, 1);
    REGISTER_PLUGIN(AlembicInHDFOp, "AlembicInHDF", 0, 1);
    REGISTER_PLUGIN(AlembicInAddToLightAndCameraListsOp,
                "AlembicInAddToLightAndCameraLists", 0, 1);

}
