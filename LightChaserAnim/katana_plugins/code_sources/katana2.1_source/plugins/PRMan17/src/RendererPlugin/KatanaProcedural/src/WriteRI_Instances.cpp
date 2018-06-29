// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <ErrorReporting.h>
#include <PRManProcedural.h>
#include <WriteRI_Instances.h>

#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

#include <boost/thread.hpp>

using namespace FnKat;

namespace PRManProcedural
{

// Global map holding all the producerPackets used as instance sources
ProducerPacketMap global_instanceDict;
boost::recursive_mutex global_producerInstanceSourceMapMutex;

void WriteRI_Object_State(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    WriteRI_Object_LevelOfDetail(sgIterator, sharedState);
    RiAttributeBegin();
    WriteRI_Object_Name(sgIterator, sharedState);
    WriteRI_Object_Id(sgIterator, sharedState);
    FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
    WriteRI_Object_Transform(xformAttr, sgIterator, sharedState);
    WriteRI_Object_RelativeScopedCoordinateSystems(sgIterator, sharedState);
    WriteRI_Object_Statements(sgIterator, sharedState);
    WriteRI_Object_Material(sgIterator, sharedState);
    WriteRI_Object_IlluminationList(sgIterator, sharedState);
}

bool WriteRI_CheckInstance(FnScenegraphIterator& sgIterator, InstanceInfo& info)
{
    if (!sgIterator.isValid())
    {
        return false;
    }

    const std::string type = sgIterator.getType();

    //
    // Conditions to be an instance source:
    //
    // - any location of type 'instance source' with the attributes instance.ID != ""
    // - any 'renderable' location type with the attributes instance.ID != "" and instance.type set
    //

    bool isInstanceSource = false;
    std::string instanceID;
    std::string geoInstanceID;
    std::string instanceType;

    FnAttribute::StringAttribute instanceIDAttr = sgIterator.getAttribute("instance.ID");
    if (instanceIDAttr.isValid())
    {
        instanceID = instanceIDAttr.getValue();
    }

    FnAttribute::StringAttribute instanceTypeAttr = sgIterator.getAttribute("instance.type");
    if (instanceTypeAttr.isValid())
    {
        instanceType = instanceTypeAttr.getValue();
    }

    FnAttribute::StringAttribute geoInstanceIDAttr = sgIterator.getAttribute("geometry.instanceSource");
    if (geoInstanceIDAttr.isValid())
    {
        geoInstanceID = geoInstanceIDAttr.getValue();
    }

    // 'instance source' locations could not have an instance.ID attribute !
    // In that case we use its full path
    if (type == "instance source")
    {
        if (instanceID.empty())
        {
            instanceID = sgIterator.getFullName();
        }

        isInstanceSource = true;
    }
    else if (type == "instance")
    {
        if (geoInstanceID.empty())
        {
            Report_Error("No valid instance-id found on instance", sgIterator);
            return false;
        }

        if (!instanceType.empty())
        {
            Report_Error("Invalid instance declaration", sgIterator);
            return false;
        }
    }
    else
    {
        // If instanceID is empty this is not an instance location
        if (instanceID.empty())
        {
            return false;
        }
        else
        {
            // If instanceID and instanceType are valid the current
            // location is considered to be an instance source, if not
            // the location is considered to be an instance element
            if (!instanceType.empty())
            {
                isInstanceSource = true;
            }
        }
    }

    // Only at this point if we have an instance source and 
    // instanceType is empty we set it to the default value: 'object'
    if (isInstanceSource && instanceType.empty())
    {
        instanceType = "object";
    }

    info.instanceID = isInstanceSource ? instanceID : geoInstanceID;
    info.instanceType = instanceType;
    info.isSource = isInstanceSource;

    return true;
}

void WriteRI_InstanceSource(const ProducerPacket& producerPacket, const InstanceInfo& info,
                            PRManPluginState* sharedState)
{
    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    if (!sgIterator.isValid() || !info.isSource)
    {
        return;
    }

    boost::recursive_mutex::scoped_lock lock(global_producerInstanceSourceMapMutex);
    ProducerPacketMap::iterator result = global_instanceDict.find(info.instanceID);

    // Is this an instance source ?
    if (result != global_instanceDict.end())
    {
        Report_Warning("Instance source already exists", sgIterator);
        return;
    }

    std::string message = "Found instance definition ";
    message += info.instanceID;
    Report_Debug(message, sharedState, sgIterator);

    ProducerPacket* newProducerPacket = NULL;

    if (info.instanceType == "object")
    {
        newProducerPacket = new ProducerPacket;
        newProducerPacket->fullRecurse = true;
        newProducerPacket->sgIterator = sgIterator;
        newProducerPacket->enableStrictVisibilityTesting = producerPacket.enableStrictVisibilityTesting;

        RtToken tokens[1];
        tokens[0] = RI_HANDLEID;
        RtPointer values[1];
        RtString id = const_cast<RtString>( info.instanceID.c_str() );
        values[0] = (RtPointer)( &id );

        RtObjectHandle objectHandle = RiObjectBeginV(1, tokens, values);
        if (objectHandle != NULL)
        {
            WriteRI_Location(*newProducerPacket, sharedState, false);
            RiObjectEnd();
        }
        else
        {
            Report_Error("Unable to create '" + info.instanceID + "' instance.", sgIterator);

            // Clean-up and return
            delete newProducerPacket;
            return;
        }

        newProducerPacket->producerData = objectHandle;
        newProducerPacket->producerType = kObjectInstance;

        newProducerPacket->neverDelete = true;
        newProducerPacket->copyGroupingFromParent(producerPacket.grouping, sgIterator);

        // Store the archive in the cache for future instances.
        global_instanceDict[info.instanceID] = newProducerPacket;
    }
    else if (info.instanceType == "inline archive")
    {
        std::string message = "Found instance definition (type:inline archive) ";
        message += info.instanceID;
        Report_Debug(message, sharedState, sgIterator);

        newProducerPacket = new ProducerPacket;
        newProducerPacket->fullRecurse = true;
        newProducerPacket->sgIterator = sgIterator;
        newProducerPacket->enableStrictVisibilityTesting = producerPacket.enableStrictVisibilityTesting;

        // Create the fully recursed RiArchive
        RtArchiveHandle arHandle = RiArchiveBegin((RtToken)info.instanceID.c_str(), RI_NULL);
        WriteRI_Location(*newProducerPacket, sharedState, false);
        RiArchiveEnd();

        newProducerPacket->producerData = arHandle;
        newProducerPacket->producerType = kArchive;

        newProducerPacket->neverDelete = true;
        newProducerPacket->copyGroupingFromParent(producerPacket.grouping, sgIterator);

        // Store the archive in the cache for future instances.
        global_instanceDict[info.instanceID] = newProducerPacket;
    }
    else if (info.instanceType == "katana")
    {
        std::string message = "Found instance definition (type:producer) ";
        message += info.instanceID;
        Report_Debug(message, sharedState, sgIterator);

        newProducerPacket = new ProducerPacket;

        *newProducerPacket = producerPacket;
        newProducerPacket->producerType = kKatanaInstance;
        newProducerPacket->copyGroupingFromParent(producerPacket.grouping, sgIterator);

        // Store the archive in the cache for future instances.
        global_instanceDict[info.instanceID] = newProducerPacket;
    }
}

void WriteRI_InstanceElement(const ProducerPacket& producerPacket, const InstanceInfo& info,
                             PRManPluginState* sharedState)
{
    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    if (!sgIterator.isValid() || info.isSource)
    {
        return;
    }

    boost::recursive_mutex::scoped_lock lock(global_producerInstanceSourceMapMutex);
    ProducerPacketMap::iterator result = global_instanceDict.find(info.instanceID);

    // Not an instance source, but an instance element
    if (result == global_instanceDict.end())
    {
        Report_Error("Unable to find instance source for '" + info.instanceID + "'.", sgIterator);
        return;
    }
    else
    {
        ProducerPacket * sourceProducerPacket = (*result).second;

        if (!sourceProducerPacket)
        {
            Report_Error("Invalid instance source '" + info.instanceID + "'.", sgIterator);
            return;
        }

        if (sourceProducerPacket->producerType == kArchive || sourceProducerPacket->producerType == kObjectInstance)
        {
            FnAttribute::DoubleAttribute boundAttr = sgIterator.getAttribute("bound");
            RtBound bound;
            if (boundAttr.isValid())
            {
                FillBoundFromAttr( &bound, boundAttr, sharedState );
                if (bound[0] == bound[1] && bound[2] == bound[3] && bound[4] == bound[5])
                {
                    Report_Warning("Empty bounding box detected (prman will not expand beyond this point).", sgIterator);
                    return;
                }
            }

            ProducerPacket * newProducerPacket = new ProducerPacket;
            WriteRI_Object_State(producerPacket.sgIterator, sharedState);

            *newProducerPacket = *sourceProducerPacket;

            // Take a copy of the sgIterator from the incoming ProducerPacket
            newProducerPacket->sgIterator = producerPacket.sgIterator;
            
            newProducerPacket->neverDelete = false;

            if (boundAttr.isValid())
            {
                RiProcedural(newProducerPacket, bound, PRManProcedural::Subdivide, PRManProcedural::Free);
            }
            else
            {
                PRManProcedural::Subdivide(newProducerPacket, 0);
                delete newProducerPacket;
            }
            RiAttributeEnd();
        }
        else if (sourceProducerPacket->producerType == kKatanaInstance)
        {
            WriteRI_Object_State(producerPacket.sgIterator, sharedState);
            WriteRI_Location(*sourceProducerPacket, sharedState, false);
            RiAttributeEnd();
        }
    }
}

}
