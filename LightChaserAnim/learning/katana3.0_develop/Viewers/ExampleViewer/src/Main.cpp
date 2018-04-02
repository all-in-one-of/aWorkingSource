// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
    #include <FnPlatform/Windows.h>
#endif
#include <FnViewer/plugin/FnViewerDelegate.h>
#include <FnViewer/plugin/FnViewportLayer.h>
#include <FnViewer/plugin/FnViewport.h>

#include "ExampleViewport.h"
#include "ExampleViewerDelegate.h"
#include "components/IgnoreComponent.h"
#include "components/LoggingComponent.h"
#include "layers/ExampleSceneLayer.h"

namespace // anonymous
{

    DEFINE_VIEWER_DELEGATE_PLUGIN(ExampleViewerDelegate);
    DEFINE_VIEWPORT_PLUGIN(ExampleViewport);

    DEFINE_VIEWER_DELEGATE_COMPONENT_PLUGIN(IgnoreComponent);
    DEFINE_VIEWER_DELEGATE_COMPONENT_PLUGIN(LoggingComponent);

    DEFINE_VIEWPORT_LAYER_PLUGIN(ExampleSceneLayer);

} // namespace anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(ExampleViewerDelegate, "ExampleViewerDelegate", 0, 1);
    REGISTER_PLUGIN(ExampleViewport, "ExampleViewport", 0, 1);
    REGISTER_PLUGIN(IgnoreComponent, "IgnoreComponent", 0, 1);
    REGISTER_PLUGIN(LoggingComponent, "LoggingComponent", 0, 1);
    REGISTER_PLUGIN(ExampleSceneLayer, "ExampleSceneLayer", 0, 1);
}

