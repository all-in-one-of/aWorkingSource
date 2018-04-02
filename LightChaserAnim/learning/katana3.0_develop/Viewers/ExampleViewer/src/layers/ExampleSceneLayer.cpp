// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
    #include <FnPlatform/Windows.h>
#endif
#include <FnViewer/plugin/FnGLStateHelper.h>
#include <FnViewer/utils/FnDrawingHelpers.h>
#include "layers/ExampleSceneLayer.h"
#include "GL/glew.h"
#include <map>
#include <ImathGLU.h>
#include <algorithm>
#include <cmath>
#include <set>

#include "ExampleViewerDelegate.h"


// ============================================================================
//   ExampleSceneLayer
// ============================================================================

ExampleSceneLayer::ExampleSceneLayer() :
    m_sceneRoot(0x0)
{
}

ExampleSceneLayer::~ExampleSceneLayer()
{
}

void ExampleSceneLayer::setup()
{
    // This layer requires that the ViewerDelegate is an ExampleViewerDelegate
    ViewportWrapperPtr viewport = getViewport();
    Foundry::Katana::ViewerAPI::ViewerDelegateWrapperPtr delegate =
        viewport->getViewerDelegate();
    ExampleViewerDelegate* exampleDelegate =
        delegate->getPluginInstance<ExampleViewerDelegate>();
    if(exampleDelegate)
    {
        m_sceneRoot = exampleDelegate->getSceneRoot();
    }

    resize(viewport->getWidth(), viewport->getHeight());

    if (!m_pickingShader.isLinked())
    {
        // Set up the shader program
        std::string fullPath = getenv("KATANA_ROOT");
        fullPath.append("/plugins/Resources/Core/Shaders/GenericViewerPlugins/");
        fullPath.append("uniform_color");
        m_pickingShader.compileShader(fullPath + ".vert", VERTEX);
        m_pickingShader.compileShader(fullPath + ".frag", FRAGMENT);
        m_pickingShader.link();
    }
}

void ExampleSceneLayer::cleanup()
{
    // Clear Gl resources of the SceneRoot and its children
    if (m_sceneRoot)
    {
        m_sceneRoot->clearGLResources();
    }
}

void ExampleSceneLayer::draw()
{
    using namespace Foundry::Katana;

    if(!m_sceneRoot)
        return;

    ViewerUtils::GLStateRestore glStateRestore(ViewerUtils::Multisample);
    glEnable(GL_MULTISAMPLE);               // ViewerUtils::Multisample

    ViewportWrapperPtr viewport = getViewport();
    FnAttribute::DoubleAttribute projectionMat =
        viewport->getOption("ProjectionMatrix");
    FnAttribute::DoubleAttribute viewMat =
        viewport->getOption("ViewMatrix");

    // Starting at the scene root, begin traversing and drawing the scene
    if(m_sceneRoot->preDraw())
    {
        // If preDraw returns true, draw the location and its children
        m_sceneRoot->draw(projectionMat, viewMat);
        m_sceneRoot->drawChildren(projectionMat, viewMat);
    }
    m_sceneRoot->postDraw();

    glUseProgram(0);
}

bool ExampleSceneLayer::event(const FnEventWrapper& eventData)
{
    return false;
}

void ExampleSceneLayer::resize(unsigned int width, unsigned int height)
{
}

void ExampleSceneLayer::pickerDraw(unsigned int x, unsigned int y,
                                   unsigned int w, unsigned int h,
                                   const PickedAttrsMap& ignoreAttrs)
{
    std::set<std::string> ignoreLocations;
    getLocationsFromPickedAttrsMap(ignoreAttrs, ignoreLocations);

    ViewportWrapperPtr viewport = getViewport();
    FnAttribute::DoubleAttribute projectionMat =
        viewport->getOption("ProjectionMatrix");
    FnAttribute::DoubleAttribute viewMat =
        viewport->getOption("ViewMatrix");

    // Starting at the scene root, begin traversing and drawing the scene
    if(m_sceneRoot->preDraw())
    {
        // If preDraw returns true, draw the location and its children
        m_sceneRoot->pickerDraw(
            *this, m_pickingShader, projectionMat, viewMat, ignoreLocations);
        m_sceneRoot->pickerDrawChildren(
            *this, m_pickingShader, projectionMat, viewMat, ignoreLocations);
    }
    m_sceneRoot->postDraw();
}

void ExampleSceneLayer::getLocationsFromPickedAttrsMap(
    const PickedAttrsMap& pickedMap,
    std::set<std::string>& locations)
{
    PickedAttrsMap::const_iterator it;
    for (it = pickedMap.begin(); it != pickedMap.end(); ++it)
    {
        FnAttribute::GroupAttribute child = it->second;
        if (child.isValid())
        {
            // The convention for a location will be a GroupAttribute with
            // a child called "location" and the value must be a
            // StringAttribtue containing the location path in its value
            FnAttribute::StringAttribute locationAttr =
                child.getChildByName("location");
            if (locationAttr.isValid())
            {
                locations.insert(locationAttr.getValue());
            }
        } 
    }
}
