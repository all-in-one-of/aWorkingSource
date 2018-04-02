// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef EXAMPLEVIEWERDELEGATE_H_
#define EXAMPLEVIEWERDELEGATE_H_

#include <FnAttribute/FnGroupBuilder.h>
#include <FnViewer/plugin/FnViewerDelegate.h>
#include <FnViewer/plugin/FnViewerLocationEvent.h>
#include <FnAttribute/FnAttribute.h>

#include "SceneNode.h"

using Foundry::Katana::ViewerAPI::OptionIdGenerator;

/**
This class adds all of our required custom behaviour to the ViewerDelegate
interface. Currently it's main responsibility is maintaining the tree of
SceneNode objects, which represent locations in the scene graph, which can
be drawn by viewports.

It is important to not that since no OpenGL context is made current prior
to ViewerDelegate functions being called, it should not be the delegates
job to update graphics resources on the SceneNodes. Instead they should
be flagged as dirty, and will be setup by the viewports in a deferred manner.
*/
class ExampleViewerDelegate : public Foundry::Katana::ViewerAPI::ViewerDelegate
{
public:
    ExampleViewerDelegate();
    ~ExampleViewerDelegate();

    /// Returns a new instance of ExampleViewerDelegate.
    static Foundry::Katana::ViewerAPI::ViewerDelegate* create();

    /// Flushes any cached data.
    static void flush();

    /// Initializes the ViewerDelegate.
    void setup();

    /// Cleans up the resources.
    void cleanup();

    /// Stores the passed option in a map
    void setOption(OptionIdGenerator::value_type optionId,
        FnAttribute::Attribute attr);

    /// Returns the specified option from a map, or an invalid attribute.
    FnAttribute::Attribute getOption(
        OptionIdGenerator::value_type optionId);

    void locationEvent(
            const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
            bool locationHandled) override;

    void sourceLocationEvent(
            const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event)
                    override;

    /// Called when locations are selected in Katana
    void locationsSelected(const std::vector<std::string>& locations);

    /// Freeze the layer state when not visible.
    void freeze();

    /// Thaw the layer state when made visible.
    void thaw();

    /// Returns the root SceneNode.
    SceneNode* getSceneRoot() { return &m_sceneRoot; }

private:
    /// Marks all of the delegates attached viewports as dirty
    void dirtyAllViewports();

    /// Deactivate/activate the manipulators for current selected locations
    void reactivateManipulators();

    typedef std::map<OptionIdGenerator::value_type,
        FnAttribute::Attribute> AttrMap;

    /// A map of options attributes
    AttrMap m_options;

    /// The root of the SceneNode tree
    SceneNode m_sceneRoot;
};

#endif  // EXAMPLEVIEWERDELEGATE_H_
