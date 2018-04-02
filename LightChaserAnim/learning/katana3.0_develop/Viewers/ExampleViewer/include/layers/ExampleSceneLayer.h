// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SCENELAYER_H_
#define SCENELAYER_H_

#include "DrawableMesh.h"
#include "SceneNode.h"

#include <FnViewer/plugin/FnViewportLayer.h>
#include <FnViewer/plugin/FnEventWrapper.h>
#include <FnViewer/utils/FnGLShaderProgram.h>

#include <FnAttribute/FnAttribute.h>
#include <string>
#include <map>

using Foundry::Katana::ViewerAPI::FnEventWrapper;
using Foundry::Katana::ViewerAPI::ViewportLayer;
using Foundry::Katana::ViewerAPI::ViewportWrapperPtr;

/**
 * This layer is responsible for traversing the tree of SceneNodes, drawing
 * each node as it goes.
 *
 * The SceneNode objects are owned by the ExampleViewerDelegate allowing
 * multiple viewports to share the same data on the graphics card.
 */
class ExampleSceneLayer : public ViewportLayer
{
public:
    typedef std::map<std::string, DrawableMesh*> MeshMap;
    typedef std::map<std::string, FnAttribute::GroupAttribute> MeshQueue;

    ExampleSceneLayer();
    virtual ~ExampleSceneLayer();

    /// Returns a new instance of ExampleSceneLayer.
    static ViewportLayer* create()
    {
        return new ExampleSceneLayer();
    }

    /// Retrieves a pointer to the ExampleViewerDelegate's root SceneNode.
    virtual void setup();

    /// Cleans up the resources.
    virtual void cleanup();

    /// Traverses the SceneNode tree drawing each visible location.
    virtual void draw();

    /// Draws the SceneNode tree for picking
    void pickerDraw(unsigned int x, unsigned int y,
                    unsigned int w, unsigned int h,
                    const PickedAttrsMap& ignoreAttrs);

    // Returns true if the event has been handled. Otherwise false, and the
    // event will be passed to the next layer
    virtual bool event(const FnEventWrapper& eventData);

    /// Updates to the new viewport dimensions
    virtual void resize(unsigned int width, unsigned int height);

    /// Freezes the layer state when not visible.
    void freeze() {}

    /// Thaws the layer state when made visible.
    void thaw() {}

protected:

    /*
     * Returns an attribute containing a list of scene graph locations that can
     * be selected at the passed window coordinates.
     *
     * Note that the OpenGL context must be current before this method is
     * called.
     */
    void getSelectableItems(int x, int y, int w, int h,
        std::vector<std::string>& selectedItems);

    void getLocationsFromPickedAttrsMap(
        const PickedAttrsMap& pickedMap,
        std::set<std::string>& locations);

private:
    // A pointer to the scene root node, typically owned by the viewer delegate
    SceneNode* m_sceneRoot;

    /// Shader used by pickerDraw().
    GLShaderProgram m_pickingShader;
};
#endif /* SCENELAYER_H_ */
