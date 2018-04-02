// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef EXAMPLE_LOGGGING_COMPONENT_H_
#define EXAMPLE_LOGGGING_COMPONENT_H_

#include <FnViewer/plugin/FnViewerDelegateComponent.h>
#include <FnViewer/plugin/FnViewerDelegate.h>

#include <vector>
#include <string>

using Foundry::Katana::ViewerAPI::ViewerDelegateComponent;
using Foundry::Katana::ViewerAPI::ViewerDelegateWrapperPtr;

/**
 * @brief: A ViewerDelegateComponent that logs its member function calls.
 *
 * This is a generic utility ViewerDelegateComponent that prints whenever any
 * of its "location*" callback functions are called. This can be used when
 * debugging the Viewer, for example.
 */
class LoggingComponent : public ViewerDelegateComponent
{

public:

    LoggingComponent();

    ~LoggingComponent();

    void setup();

    void cleanup();

    static Foundry::Katana::ViewerAPI::ViewerDelegateComponent* create();

    static void flush();

    bool locationEvent(
            const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
            bool locationHandled) override;
    void locationsSelected(const std::vector<std::string>& locations) override;
};


#endif
