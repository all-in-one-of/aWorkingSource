#include "components/LoggingComponent.h"
#include <iostream>

LoggingComponent::LoggingComponent() {}

LoggingComponent::~LoggingComponent() {}

void LoggingComponent::setup() {}

void LoggingComponent::cleanup() {}

Foundry::Katana::ViewerAPI::ViewerDelegateComponent* LoggingComponent::create()
{
    return new LoggingComponent();
}

void LoggingComponent::flush() {}

bool LoggingComponent::locationEvent(
        const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
        bool locationHandled)
{
    // TODO(DL): Print more information
    std::cout << "[LoggingComponent] : locationEvent: " <<
            event.locationPath << std::endl;
    return false;
}

void LoggingComponent::locationsSelected(
        const std::vector<std::string>& locations)
{
    std::cout << "[LoggingComponent] : locationsSelected: ";
    for (unsigned int i = 0; i < locations.size(); ++i)
    {
        std::cout << locations[i];
        if (i < locations.size() - 1) { std::cout << ", "; }
    }
    std::cout << std::endl;
}
