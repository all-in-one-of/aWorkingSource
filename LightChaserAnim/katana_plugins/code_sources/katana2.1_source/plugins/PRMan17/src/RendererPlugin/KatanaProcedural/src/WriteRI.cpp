// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <map>
#include <set>
#include <vector>
#include <list>
#include <exception>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>

#include <PRManProcedural.h>
#include <Timer.h>

#include <pystring/pystring.h>

#include <slo.h>
#include <rx.h>
#include <ErrorReporting.h>
#include <FnRender/plugin/ScenegraphLocationDelegate.h>

#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Statements.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Instances.h>

using namespace FnKat;

namespace PRManProcedural
{


// Write out all the lights in the scene.
// Pass in a producer pointing to "/root/world" and find the lightList.
void WriteRI_Lights(FnScenegraphIterator worldSgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Lights called", sharedState, worldSgIterator);

    if (!worldSgIterator.isValid())
        Report_Fatal("WriteRI_Lights called with null sg iterator.");
    if (worldSgIterator.getFullName() != "/root/world")
        Report_Fatal("WriteRI_Lights called with a producer other than /root/world", worldSgIterator);

    // Query the lightlist on /root/world
    FnAttribute::GroupAttribute localLightList = worldSgIterator.getAttribute("lightList");
    if (!localLightList.isValid())
    {
        Report_Warning("lightList not found at /root/world.  Lights will not be defined.", worldSgIterator);
        return;
    }

    //We want to use a cached iterator here for performance reasons
    FnScenegraphIterator rootIterator = worldSgIterator.getRoot();

    // For each light in the light list...
    int numLocalLights = localLightList.getNumberOfChildren();
    for (int i = 0; i < numLocalLights; i++)
    {

        // Discover the light's scenegraph location
        FnAttribute::GroupAttribute lightGroup = localLightList.getChildByIndex(i);
        if (!lightGroup.isValid())
        {
            Report_Warning("Contents of lightList attribute may be invalid.  Skipping a lightList entry", worldSgIterator);
            continue;
        }

        FnAttribute::StringAttribute pathAttr = lightGroup.getChildByName("path");
        if (!pathAttr.isValid())
        {
            Report_Warning("Contents of lightList attribute may be invalid.  Skipping a lightList entry", worldSgIterator);
            continue;
        }

        // Move iterator to light location; output!
        std::string lightPath = pathAttr.getValue();


        FnScenegraphIterator lightSgIterator = rootIterator.getByPath(lightPath);
        if (!lightSgIterator.isValid())
        {
            // A light in the light list which does not exist is not an error.
            // This will happen if you prune out a light, for example.
            continue;
        }

        if (lightSgIterator.getType() != "light")
        {
            std::ostringstream errorMsg;
            errorMsg << "An object appears in light list, but it is not a light!  It is of type ";
            errorMsg << "'" << lightSgIterator.getType() << "'";
            // Report the error text!
            Report_Warning(errorMsg.str(), lightSgIterator);

            if (lightSgIterator.getType() == "error")
            {
                Report_Error_Location(lightSgIterator);
            }
            continue;
        }

        // Muted lights don't get written to RIB
        FnAttribute::IntAttribute muteAttr = lightSgIterator.getAttribute("mute", true);
        if (muteAttr.isValid() && muteAttr.getValue() != 0) continue;

        FnAttribute::GroupAttribute materialAttr = lightSgIterator.getAttribute("material");
        if(!materialAttr.isValid() || materialAttr.getNumberOfChildren() < 1) //TODO: it was like this: (0 == materialAttr.get())
        {
            // If the light has no material, this is not an error.
            // We simply don't emit a light.
            continue;
        }

        RiAttributeBegin();
        RiTransformBegin();
        // this writes the light name, full transform, and material (which instantiates
        // the light source)
        WriteRI_Object_Name(lightSgIterator, sharedState);
        WriteRI_Object_GlobalTransform(lightSgIterator, sharedState);
        WriteRI_Object_Statements(lightSgIterator, sharedState);
        WriteRI_Object_Material(lightSgIterator, sharedState);
        RiTransformEnd();
        RiAttributeEnd();
    }
}


void WriteRI_CoordinateSystems(FnScenegraphIterator worldSgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_CoordinateSystems called", sharedState, worldSgIterator);

    if (!worldSgIterator.isValid())
        Report_Fatal("WriteRI_CoordinateSystems called with null producer.");
    if (worldSgIterator.getFullName() != "/root/world")
        Report_Fatal("WriteRI_CoordinateSystems called with a location other than /root/world", worldSgIterator);

    FnAttribute::GroupAttribute globalsAttr = worldSgIterator.getAttribute("globals");
    if (!globalsAttr.isValid())
    {
        return;
    }

    FnAttribute::GroupAttribute coordinateSystemsAttr = globalsAttr.getChildByName("coordinateSystems");
    if (!coordinateSystemsAttr.isValid())
    {
        return;
    }

    int numCoordSystems = coordinateSystemsAttr.getNumberOfChildren();
    FnScenegraphIterator rootSgIterator = worldSgIterator.getRoot();
    for (int i = 0; i < numCoordSystems; i++)
    {
        FnAttribute::StringAttribute pathAttr = coordinateSystemsAttr.getChildByIndex(i);
        std::string attrName = coordinateSystemsAttr.getChildName(i);

        if (!pathAttr.isValid())
        {
            Report_Debug(std::string("A coordinate system appears in the coordinateSystems list (") + \
                                        attrName + ") but it does not have a path", sharedState,
                                     worldSgIterator );
            continue;
        }

        std::string spacePath = pathAttr.getValue();
        FnScenegraphIterator spaceSgIterator = rootSgIterator.getByPath(spacePath);
        if (!spaceSgIterator.isValid())
        {
            Report_Warning("Coordinate system '" + spacePath + "' appears in coordinateSystems list, but not in scene graph", worldSgIterator);
            continue;
        }

        RiTransformBegin();
        WriteRI_Object_GlobalTransform(spaceSgIterator, sharedState);
        RiCoordinateSystem(const_cast<char*>(attrName.c_str()));
        RiTransformEnd();
    }
}

void WriteRI_Location(const ProducerPacket& producerPacket, PRManPluginState* sharedState, bool applyState)
{
    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    const std::string type = sgIterator.getType();

    if (type == "group")
    {
        if (applyState)
        {
            WriteRI_Object_LevelOfDetail(sgIterator, sharedState);
        }

        RiAttributeBegin();

        if (applyState)
        {
            WriteRI_Object_Name(sgIterator, sharedState);
            WriteRI_Object_Id(sgIterator, sharedState);
            FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
            WriteRI_Object_Transform(xformAttr, sgIterator, sharedState);
            WriteRI_Object_RelativeScopedCoordinateSystems(sgIterator, sharedState);
            WriteRI_Object_Statements(sgIterator, sharedState);
            WriteRI_Object_Material(sgIterator, sharedState);
            WriteRI_Object_IlluminationList(sgIterator, sharedState);
        }

        WriteRI_Group(producerPacket, sharedState);

        RiAttributeEnd();
        return;
    }

    if (type == "level-of-detail group")
    {
        if (applyState)
        {
            WriteRI_Object_LevelOfDetail(sgIterator, sharedState);
        }

        RiAttributeBegin();

        if (applyState)
        {
            WriteRI_Object_Name(sgIterator, sharedState);
            WriteRI_Object_Id(sgIterator, sharedState);
            FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
            WriteRI_Object_Transform(xformAttr, sgIterator, sharedState);
            WriteRI_Object_RelativeScopedCoordinateSystems(sgIterator, sharedState);
            WriteRI_Object_Statements(sgIterator, sharedState);
            WriteRI_Object_Material(sgIterator, sharedState);
            WriteRI_Object_IlluminationList(sgIterator, sharedState);
        }

        WriteRI_LevelOfDetailGroup(producerPacket, sharedState);

        RiAttributeEnd();
        return;
    }

    PRManSceneGraphLocationDelegateInput prmanSceneGraphLocationDelegateInput;
    prmanSceneGraphLocationDelegateInput.sharedState = sharedState;
    const bool pluginFound = RenderOutputUtils::processLocation(sgIterator, "prman", type, (void*)&prmanSceneGraphLocationDelegateInput, 0x0 );

    if(!pluginFound)
    {
        if (applyState)
        {
            WriteRI_Object_LevelOfDetail(sgIterator, sharedState);
        }

        RiAttributeBegin();

        if (applyState)
        {
            WriteRI_Object_Name(sgIterator, sharedState);
            WriteRI_Object_Id(sgIterator, sharedState);
            FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
            WriteRI_Object_Transform(xformAttr, sgIterator, sharedState);
            WriteRI_Object_RelativeScopedCoordinateSystems(sgIterator, sharedState);
            WriteRI_Object_Statements(sgIterator, sharedState);
            WriteRI_Object_Material(sgIterator, sharedState);
            WriteRI_Object_IlluminationList(sgIterator, sharedState);
        }

        WriteRI_Group(producerPacket, sharedState);

        RiAttributeEnd();
    }
}

// Currently no ScenegraphLocationDelegate is defined for level-of-detail, group, error, and light location types.
void WriteRI_Object_real(const ProducerPacket& producerPacket, PRManPluginState* sharedState)
{

    FnScenegraphIterator sgIterator = producerPacket.sgIterator;

	GroupAttribute lightAttr = sgIterator.getAttribute("lightList");
	//if(lightAttr.isValid())
	//{
	//	Report_Warning("Found a light list!", sgIterator);
	//}
    if (!sgIterator.isValid())
    {
        Report_Error("Attempt to write object from null geometry producer.");
        return;
    }

    FnAttribute::StringAttribute msgAttr = sgIterator.getAttribute("errorMessage");
    if (msgAttr.isValid())
    {
        Report_Error_Location(sgIterator);
    }

    msgAttr = sgIterator.getAttribute("warningMessage");
    if (msgAttr.isValid())
    {
        Report_Warning(msgAttr.getValue(), sgIterator);
    }

    // At root/world, take care of lights and named coordinate system preprocessing
    if (sgIterator.getFullName() == "/root/world")
    {
        //declare this in advance so that subsequent RiCurves calls calculate varying lengths properly
        //even in rib-writing mode
        RiBasis( RiBSplineBasis, 1, RiBSplineBasis, 1);

        WriteRI_Lights(sgIterator, sharedState);
        WriteRI_CoordinateSystems(sgIterator, sharedState);

        Report_Info("Render Startup Complete.");

        FnScenegraphIterator i = sgIterator.getRoot();
        FnAttribute::StringAttribute selectionListAttr = i.getAttribute("info.renderSelection");
        if (selectionListAttr.isValid())
        {
            FnAttribute::StringConstVector selectionVec = selectionListAttr.getNearestSample(0);
            std::ostringstream os;
            os << "Selection list is: \n";
            for (unsigned long i = 0; i < selectionVec.size(); ++i)
                os << "\t'" << selectionVec[i] << "'\n";
            Report_Info(os.str());
        }
        Report_Info("Rendering...\n");
    }


    // Don't output lights here; just skip them.
    if (sgIterator.getType() == "light")
        return;
    // Only output geometry from /root/world
    if (!(sgIterator.getFullName() == "/root" || sgIterator.getFullName().compare(0, 11, "/root/world") == 0))
        return;

    if (sgIterator.getType() == "error")
    {
        // error locations cause the render to fail out.
        RiAttributeBegin();
        RiAttributeEnd(); // balance the begin/end statements... exit doesn't actually exit
        Report_Fatal("Error location found.", sgIterator);
        exit(-1);
    }

    InstanceInfo instanceInfo;
    if (WriteRI_CheckInstance(sgIterator, instanceInfo))
    {
        // Check if the current location is visible
        if (IsVisible(sgIterator))
        {
            if (instanceInfo.isSource)
            {
                WriteRI_InstanceSource(producerPacket, instanceInfo, sharedState);
            }
            else
            {
                WriteRI_InstanceElement(producerPacket, instanceInfo, sharedState);
            }
        }
    }
    else
    {
        WriteRI_Location(producerPacket, sharedState);
    }
}


void WriteRI_Transforms( const std::vector<FnKat::RenderOutputUtils::Transform>& transforms )
{
    std::vector<FnKat::RenderOutputUtils::Transform>::const_iterator transformIt;

    for( transformIt = transforms.begin(); transformIt != transforms.end(); ++transformIt )
    {
        FnKat::RenderOutputUtils::TransformList transformList = (*transformIt).transformList;
        std::vector<float> sampleTimes = (*transformIt).sampleTimes;

        if( sampleTimes.size() > 1 )
        {
            std::vector<RtFloat> prmanSampleTimes;
            for( std::vector<float>::const_iterator sampleIt = sampleTimes.begin(); sampleIt != sampleTimes.end(); ++sampleIt )
            {
                prmanSampleTimes.push_back( (RtFloat) *sampleIt );
            }

            RtInt noSamples = prmanSampleTimes.size();
            RiMotionBeginV( noSamples, (RtFloat*) &prmanSampleTimes[0] );
        }

        for( FnKat::RenderOutputUtils::TransformList::const_iterator it = transformList.begin(); it != transformList.end(); ++it )
        {
            std::string transformType = it->first;
            FnKat::RenderOutputUtils::TransformData transform = it->second;

            if( transformType == "translate" && transform.size() == 3 )
            {
                RiTranslate( transform[0], transform[1], transform[2] );
            }
            else if( transformType == "rotate" && transform.size() == 4 )
            {
                RiRotate( transform[0], transform[1], transform[2], transform[3] );
            }
            else if( transformType == "scale" && transform.size() == 3 )
            {
                RiScale( transform[0], transform[1], transform[2] );
            }
            else if( transformType == "matrix" && transform.size() == 16 )
            {
                // 'transform' is already a vec<float>
                RtMatrix matrix = {{transform[ 0], transform[ 1], transform[ 2], transform[ 3]},
                                   {transform[ 4], transform[ 5], transform[ 6], transform[ 7]},
                                   {transform[ 8], transform[ 9], transform[10], transform[11]},
                                   {transform[12], transform[13], transform[14], transform[15]}};
                RiConcatTransform(matrix);
            }
        }

        if( sampleTimes.size() > 1 )
        {
            RiMotionEnd();
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// WriteRI_Object
//
// WriteRI_Object is the routine used to write a single location.
void WriteRI_Object(const ProducerPacket& producerPacket, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object called", producerPacket, sharedState);

    if (sharedState->proceduralSettings.printTimerInfo)
    {
        UTIL::Timer timer("Convert to RIB", &globalRIBCounter);
    }
    WriteRI_Object_real(producerPacket, sharedState);
}


// External entry point; write out an entire heirarchy.
void WriteRI(const ProducerPacket& producerPacket, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI called ", producerPacket, sharedState);

    FnScenegraphIterator sgIterator = producerPacket.sgIterator;
    if(!sgIterator.isValid())
    {
        Report_Debug("WriteRI finished ", producerPacket, sharedState);
        return;
    }

    // Create an instance cache
    InstanceCache instanceCache;

    FnScenegraphIterator initialSgIterator = sgIterator;
    FnScenegraphIterator currentInstanceCacheSgIterator = sgIterator;

    unsigned int currentInstanceCacheCount = 0;
    unsigned int numUniqueProducers = 1;

    // Write an entire level of the scene graph out to prman.
    while(sgIterator.isValid())
    {
        if(currentInstanceCacheCount > sharedState->proceduralSettings.producerInstanceCacheInterval)
        {
            currentInstanceCacheCount = 0;
            currentInstanceCacheSgIterator = sgIterator;
            numUniqueProducers++;
        }

        ProducerPacket localPacket;
        localPacket.sgIterator = sgIterator;
        localPacket.fullRecurse = producerPacket.fullRecurse;
        localPacket.enableStrictVisibilityTesting = producerPacket.enableStrictVisibilityTesting;
        localPacket.ribDump = producerPacket.ribDump;

        localPacket.grouping = producerPacket.grouping;

        WriteRI_Object(localPacket, sharedState);

        if(sharedState->proceduralSettings.limitProducerCaching)
        {
            instanceCache[sgIterator.getName()] = currentInstanceCacheSgIterator;
            ++currentInstanceCacheCount;
        }

        {
            if (sharedState->proceduralSettings.printTimerInfo)
            {
                UTIL::Timer timer("ScenegraphTraversal.getNextSibling", &globalScenegraphTraversalCounter);
            }

            sgIterator = sgIterator.getNextSibling();
        }
    }

    if(initialSgIterator.getParent().isValid())
    {
        RegisterInstanceCache(initialSgIterator.getParent().getFullName(), instanceCache, numUniqueProducers, sharedState);
    }

    Report_Debug("WriteRI finished ", producerPacket, sharedState);
}

}
