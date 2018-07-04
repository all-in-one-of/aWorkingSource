// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef EXAMPLEVIEWPORT_H_
#define EXAMPLEVIEWPORT_H_

#include <FnViewer/plugin/FnViewport.h>
#include <FnViewer/plugin/FnViewportLayer.h>
#include <FnViewer/plugin/FnEventWrapper.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include "GL/glew.h"
#include "ExampleViewerDelegate.h"

#include <map>
#include <string>

using Foundry::Katana::ViewerAPI::FnEventWrapper;
using Foundry::Katana::ViewerAPI::OptionIdGenerator;
using Foundry::Katana::ViewerAPI::ViewerDelegate;

/**
 * This class implements the Viewport API and is used in conjuction with
 * the ExampleViewerDelegate to draw the scene.
 * The viewport itself does not do very much other than initialize the
 * OpenGL state, and primarily works as a container for ViewportLayers
 * where all of the specialized processing takes place.
 */
class ExampleViewport : public Foundry::Katana::ViewerAPI::Viewport
{
public:
    ExampleViewport();
    ~ExampleViewport();

    /// Returns a new instance of ExampleViewport.
    static Foundry::Katana::ViewerAPI::Viewport* create();

    /// Initializes glew and sets up the default OpenGL state.
    void setup();

    /// Cleans up the Viewport resources.
    void cleanup();

    /**
     * Processes UI events. Should return true if the event was handled,
     * otherwise false.
     */
    bool event(const FnEventWrapper& event);

    /**
     * Performs a background fill, draws the ViewportLayers and then clears the
     * dirty flag.
     */
    void draw();

    /// Updates the viewport dimensions.
    void resize(unsigned int width, unsigned int height);

    /// Stores the passed option in a map for later retrieval.
    void setOption(OptionIdGenerator::value_type optionId,
        FnAttribute::Attribute attr);

    /// Returns the specified option from a map or an invalid attribute.
    FnAttribute::Attribute getOption(
        OptionIdGenerator::value_type optionId);

    /// Flushes any cached data.
    static void flush();

    /// Freeze the layer state when not visible.
    void freeze();

    /// Thaw the layer state when made visible.
    void thaw();

protected:
    /// Creates the default built-in cameras.
    void createDefaultCameras();

private:
    /// Returns the viewer delegate for this viewport.
    ExampleViewerDelegate* _getViewerDelegate();

    /// Returns a pretty description of an event, for displaying as a status.
    std::string _getEventDescription(const FnEventWrapper& event) const;

private:
    typedef std::map<OptionIdGenerator::value_type,
        FnAttribute::Attribute> AttrMap;
    /// A map of options attributes
    AttrMap m_options;
    /// Color used to clear the color buffers.
    float _backgroundColor[3];
};

#endif  // EXAMPLEVIEWPORT_H_
