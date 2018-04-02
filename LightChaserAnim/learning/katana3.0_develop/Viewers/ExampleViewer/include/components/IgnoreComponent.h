// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef EXAMPLE_IGNORE_COMPONENT_H_
#define EXAMPLE_IGNORE_COMPONENT_H_

#include <FnViewer/plugin/FnViewerDelegateComponent.h>
#include <FnViewer/plugin/FnViewerDelegate.h>
#include <FnAttribute/FnAttribute.h>

using Foundry::Katana::ViewerAPI::ViewerDelegateComponent;
using Foundry::Katana::ViewerAPI::ViewerDelegateWrapperPtr;

/**
 * @brief: A ViewerDelegateComponent that ignores locations and location types.
 *
 * This is a generic utility ViewerDelegateComponent that will ignore locations
 * and location types defined in two options in the ViewerDelegate. The options
 * are defined by the strings kIgnoreLocationsOption and
 * kIgnoreLocationTypesOption. Whenever the Viewer delegate has any of these
 * options set, then whenever a "locationEvent" callback function is called this
 * will check if the location and type match those options. If so, then these
 * functions will return "true", causing any subsequent ViewerDelegateComponents
 * and the ViewerDelegate to receive the event with locationHandled=true.
 */
class IgnoreComponent : public ViewerDelegateComponent
{

public:

    IgnoreComponent();
    ~IgnoreComponent();

    void setup();

    void cleanup();

    static Foundry::Katana::ViewerAPI::ViewerDelegateComponent* create();

    static void flush();

    bool locationEvent(
            const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
            bool locationHandled) override;

    void locationsSelected(const std::vector<std::string>& locations) override
            {}

    static const std::string kIgnoreLocationsOption;
    static const std::string kIgnoreLocationTypesOption;

private:
    /**
     * Checks if the given location's type is present in the Viewer Delegate'
     * option named kIgnoreLocationTypesOption.
     */
    bool getIgnoreLocationType(const std::string& locationPath);

    /**
     * Checks if the given location's path is present in the Viewer Delegate'
     * option named kIgnoreLocationsOption.
     */
    bool getIgnoreLocationPath(const std::string& locationPath);
};

#endif
