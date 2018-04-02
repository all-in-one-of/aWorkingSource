// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SCENENODE_H
#define SCENENODE_H

#include "DrawableMesh.h"
#include <FnViewer/plugin/FnViewportLayer.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include <FnViewer/utils/FnGLShaderProgram.h>
#include <vector>
#include <set>
#include <string>

using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

/**
 * This class is used to represent a single scene graph location and provide
 * functionality that is useful for drawing the scene. Each node can have
 * multiple child locations. When a locations data has been changed the
 * node should be marked as dirty. The when dirty locations are encountered
 * during a draw, then any invalid data will be re-initialized.
 *
 * When a location is closed in Geolib3 the node should be set to inactive
 * and the children invalidated, to release OpenGL resources.
 * This will prevent the children of that node from being drawn, and free
 * up memory.
 */
class SceneNode
{
public:
    typedef std::vector<SceneNode*> NodeList;

    SceneNode(std::string locationPath, bool proxy=false);
    virtual ~SceneNode();

    /**
     * Called before every draw() to ensure the mesh is setup, and to push
     * transforms onto the global world matrix.
     *
     * Returns true if the node should be drawn.
     */
    virtual bool preDraw();

    /// Draws with the passed matrices and the current global world matrix.
    virtual void draw(FnAttribute::DoubleAttribute projectionMatrix,
                      FnAttribute::DoubleAttribute viewMatrix);

    /// Draws the children of this node.
    virtual void drawChildren(FnAttribute::DoubleAttribute projectionMatrix,
                              FnAttribute::DoubleAttribute viewMatrix);

    /// Draws a to the passed GLPicker using the current global world matrix.
    virtual void pickerDraw(ViewportLayer& layer,
                            GLShaderProgram& pickingShader,
                            FnAttribute::DoubleAttribute projectionMatrix,
                            FnAttribute::DoubleAttribute viewMatrix,
                            std::set<std::string>& ignoreLocations);

    /// Draws the node's children to to GLPicker frame buffer.
    virtual void pickerDrawChildren(ViewportLayer& layer,
                                    GLShaderProgram& pickingShader,
                                    FnAttribute::DoubleAttribute projectionMatrix,
                                    FnAttribute::DoubleAttribute viewMatrix,
                                    std::set<std::string>& ignoreLocations);

    /// Reverts draw state changes and pops from the global world matrix
    virtual void postDraw();

    /**
     * Adds a child node with the specified name.
     *
     * The name can be a posix-style path to allow children to be added deep
     * in the hierarchy from the root node. All intermediate nodes must already
     * exist.
     */
    void addChild(const std::string& name, SceneNode* child);

    /**
     * Remove the named child node from its parent.
     *
     * As with addChild() the name can be a posix-style path.
     */
    void removeChild(const std::string& name);

    /// Removes all child nodes.
    void removeAllChildren();

    /// Sets the dirty flag to trigger resources to be reloaded if necessary.
    void setDirty(bool dirty) { m_dirty = dirty; }

    /// Returns whether the node is dirty.
    bool isDirty() const { return m_dirty; }

    /// Sets the active flag to determine whether the node should be drawn.
    void setActive(bool active) { m_active = active; }

    /// Returns whether the node is active
    bool isActive() const { return m_active; }

    /// Sets the drawable mesh to be used for this location.
    void setDrawable(DrawableMesh* drawable);

    /// Sets the attributes for this location from geolib3 for dirty checks.
    void setAttributes(FnAttribute::GroupAttribute attr);

    /// TODO
    void setXform(Imath::M44d xform);

    /**
     * Returns the named child node. As with addChild() the name can be a
     * posix-style path.
     */
    SceneNode* getChild(const std::string& name);

    /// Returns the parent scene node.
    SceneNode* getParent() { return m_parent; }

    /// Returns the scene graph location path that this node is associated with.
    std::string getLocationPath() const { return m_locationPath; }

    /**
     * Invalidate this location and its children.
     * This is used to release the geometry data from memory without,
     * deleting the node tree topology (since Geolib3 doesn't make it
     * easy to reload child locations when a locationOpened event occurs.
     */
    void invalidate();

    /// Invalidate all of the children of this node.
    void invalidateChildren();

    /// Releases any OpenGL resources held by this node and its children.
    /// This could be an expensive operation if there are many nodes.
    void clearGLResources();

  private:
    /// Is the node data dirty.
    bool m_dirty;
    /// Is the node active.
    bool m_active;
    /// The mesh associated with this node.
    DrawableMesh* m_mesh;
    /// The scene graph location path of this node.
    std::string m_locationPath;
    /// The parent node
    SceneNode* m_parent;
    /// A list of child nodes
    NodeList m_children;
    /// A list of child node names for look-up purposes
    std::vector<std::string> m_nodeNames;
    /// The attributes from this nodes scene graph location.
    FnAttribute::GroupAttribute m_locationAttributes;
    /// The local morld transform matrix for this node
    Imath::Matrix44<double> m_worldMatrix;
    /// Is the node drawing proxy geometry.
    bool m_proxy;

    /**
     * The global world matrix that is modified each time preDraw() is called
     * to get the absolute world position of a node when it is drawn.
     */
    static Imath::Matrix44<double> globalWorldMatrix;
};

#endif  //SCENENODE_H
