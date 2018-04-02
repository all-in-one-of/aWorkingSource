// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
    #include <FnPlatform/Windows.h>
#endif

#include "ExampleViewerDelegate.h"

#include <algorithm>
#include <iostream>

#include <FnViewer/plugin/FnViewport.h>
#include <FnViewer/plugin/FnManipulator.h>
#include <FnViewer/utils/FnDrawingHelpers.h>


using Foundry::Katana::ViewerAPI::ViewerDelegate;
using Foundry::Katana::ViewerAPI::ViewportWrapperPtr;
using Foundry::Katana::ViewerAPI::ManipulatorWrapperPtr;

using namespace Foundry::Katana::ViewerUtils;


ExampleViewerDelegate::ExampleViewerDelegate() : m_sceneRoot("/root")
{
}

ExampleViewerDelegate::~ExampleViewerDelegate()
{
}

ViewerDelegate* ExampleViewerDelegate::create()
{
    return new ExampleViewerDelegate();
}

void ExampleViewerDelegate::flush()
{
}

void ExampleViewerDelegate::setup()
{
}

void ExampleViewerDelegate::cleanup()
{
}

void ExampleViewerDelegate::locationEvent(
    const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
    bool locationHandled)
{
    // TODO(DL): Handle all event info, including locationHandled.
    if (event.stateChanges.locationRemoved)
    {
        // We need to make the GL context current, otherwise
        // drawable meshes (as well as their shaders) might be
        // destroyed in the wrong context.
        ViewportWrapperPtr vp = getViewport(0);
        if (vp)
            vp->makeGLContextCurrent();

        m_sceneRoot.removeChild(event.locationPath);
        dirtyAllViewports();
    }
    else if (event.stateChanges.attributesUpdated)
    {
        // Store this location data in the local scene hierarchy
        SceneNode* sgNode = m_sceneRoot.getChild(event.locationPath);
        if (!sgNode)
        {
            sgNode = new SceneNode(event.locationPath, false);
            m_sceneRoot.addChild(event.locationPath, sgNode);
        }

        sgNode->setAttributes(event.attributes);
        if (event.stateChanges.localXformUpdated)
        {
            // TODO(DL): Use isXformAbsolute
            sgNode->setXform(toImathMatrix44d(event.localXformMatrix));
        }

        dirtyAllViewports();
    }
}

void ExampleViewerDelegate::sourceLocationEvent(
    const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event)
{
    // Instancing not supported
}

void ExampleViewerDelegate::locationsSelected(
    const std::vector<std::string>& locations)
{
    //TODO: highlight the selected locations with a different color
    for (unsigned int i = 0; i < getNumberOfViewports(); ++i)
    {
        getViewport(i)->setDirty(true);
    }

    // Location selection changed, so reactivate the manipulators
    reactivateManipulators();
}

void ExampleViewerDelegate::setOption(OptionIdGenerator::value_type optionId,
        FnAttribute::Attribute attr)
{
    static const OptionIdGenerator::value_type s_reactivateManipulatorsHash =
        OptionIdGenerator::GenerateId("ReactivateManipulators");

    if (s_reactivateManipulatorsHash == optionId)
    {
        reactivateManipulators();
        return;
    }

    m_options[optionId] = attr;
}

FnAttribute::Attribute ExampleViewerDelegate::getOption(
    OptionIdGenerator::value_type optionId)
{
    AttrMap::iterator it = m_options.find(optionId);
    if(it != m_options.end())
    {
        return it->second;
    }

    return FnAttribute::Attribute();
}

void ExampleViewerDelegate::freeze()
{
}

void ExampleViewerDelegate::thaw()
{

}

void ExampleViewerDelegate::dirtyAllViewports()
{
    for(unsigned int i = 0; i < getNumberOfViewports(); ++i)
    {
        ViewportWrapperPtr vp = getViewport(i);
        vp->setDirty(true);
    }
}

void ExampleViewerDelegate::reactivateManipulators()
{
    // This could also be implemented in Python, but we do it here as a
    // reference to how to implement this in C++.
    const unsigned int viewportCount = getNumberOfViewports();
    for (unsigned int i = 0; i < viewportCount; ++i)
    {
        ViewportWrapperPtr viewport = getViewport(i);

        const unsigned int numManips =
            viewport->getNumberOfActiveManipulators();
        if (numManips > 0)
        {
            // Collect the currently active manipulators
            std::vector<std::string> manipNames;
            manipNames.reserve(numManips);

            for (unsigned int j = 0; j < numManips; ++j)
            {
                manipNames.push_back(
                    viewport->getActiveManipulator(j)->getPluginName());
            }

            // Deactivate all manipulators
            viewport->deactivateAllManipulators();

            // Get the list of selected locations
            std::vector<std::string> selectedLocations;
            getSelectedLocations(selectedLocations);

            // We want to remove the current look-through location from the
            // currently selected locations for each viewport.
            ViewportCameraWrapperPtr camera = viewport->getActiveCamera();
            if (camera && camera->hasLocationPath())
            {
                const std::vector<std::string>::iterator it =
                    std::find(selectedLocations.begin(),
                              selectedLocations.end(),
                              camera->getLocationPath());
                if (it != selectedLocations.end())
                {
                    selectedLocations.erase(it);
                }
            }

            // Activate the manipulators using the new locations. Note that the
            // list of locations can be empty, but we will keep the
            // manipulators activated with an empty list of locations anyway
            // The user might deselect all the locations, but the next time
            // a new one is selected we want the manipulator to be activated
            // still.
            for (unsigned int j = 0; j < numManips; ++j)
            {
                viewport->activateManipulator(manipNames[j], selectedLocations);
            }
        }
    }
}
