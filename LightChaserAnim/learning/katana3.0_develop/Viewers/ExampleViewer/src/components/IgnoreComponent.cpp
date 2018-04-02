#include "components/IgnoreComponent.h"

IgnoreComponent::IgnoreComponent() {}

IgnoreComponent::~IgnoreComponent() {}

const std::string IgnoreComponent::kIgnoreLocationsOption = "ignoreLocationPaths";
const std::string IgnoreComponent::kIgnoreLocationTypesOption = "ignoreLocationTypes";

void IgnoreComponent::setup() {}

void IgnoreComponent::cleanup() {}

Foundry::Katana::ViewerAPI::ViewerDelegateComponent* IgnoreComponent::create()
{
    return new IgnoreComponent();
}

void IgnoreComponent::flush() {}

bool IgnoreComponent::locationEvent(
        const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
        bool locationHandled)
{
    if (locationHandled)
    {
        return false;
    }
    return getIgnoreLocationPath(event.locationPath) ||
            getIgnoreLocationType(event.locationPath);
}

bool IgnoreComponent::getIgnoreLocationType(const std::string& locationPath)
{
    ViewerDelegateWrapperPtr delegate = getViewerDelegate();

    // Get the location type of the given location
    FnAttribute::GroupAttribute attrs = delegate->getAttributes(locationPath);
    if (!attrs.isValid()) { return false; }
    FnAttribute::StringAttribute locationTypeAttr = attrs.getChildByName("type");
    std::string locationType = locationTypeAttr.getValue("", false);
    if (locationType.empty()) { return false; }

    // Check if the ViewerDelegate has the option "ignoreLocationTypes"
    FnAttribute::StringAttribute ignoreAttr =
        delegate->getOption(kIgnoreLocationTypesOption);
    if (ignoreAttr.isValid())
    {
        // Check if the location type is in the ignoreLocationTypes option
        FnAttribute::StringConstVector ignoreValues = ignoreAttr.getNearestSample(0);
        for (size_t i = 0; i < ignoreValues.size(); ++i)
        {
            if (locationType == ignoreValues[i])
            {
                return true;
            }
        }
    }

    // The location type was not found in the ignoreLocationTypes option.
    return false;
}

bool IgnoreComponent::getIgnoreLocationPath(const std::string& locationPath)
{
    // Check if the ViewerDelegate has the option "ignoreLocationTypes"
    FnAttribute::StringAttribute ignoreAttr =
        getViewerDelegate()->getOption(kIgnoreLocationsOption);
    if (ignoreAttr.isValid())
    {
        // Check if the location type is in the ignoreLocationTypes option
        FnAttribute::StringConstVector ignoreValues = ignoreAttr.getNearestSample(0);
        for (size_t i = 0; i < ignoreValues.size(); ++i)
        {
            if (locationPath == ignoreValues[i])
            {
                return true;
            }
        }
    }

    // The location path was not found in the ignoreLocationPaths option.
    return false;
}
