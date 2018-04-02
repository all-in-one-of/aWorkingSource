// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLTRANSLATEPLANEHANDLE_H_
#define GLTRANSLATEPLANEHANDLE_H_

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <string>

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

using Foundry::Katana::ViewerAPI::Vec2i;
using Foundry::Katana::ViewerAPI::Vec3d;
using Foundry::Katana::ViewerAPI::Vec4f;
using Foundry::Katana::ViewerAPI::Matrix44d;

/** @brief A handle to translate the selected object along YZ, XZ and XY planes.
 *
 * This class represents a single handle that can be used to translate an
 * object along an plane. The handle is represented by triangle. Dragging the
 * triangle will translate the object along the plane the triangle is drawn
 * onto.
 */
class GLTranslatePlaneHandle :
        public Foundry::Katana::ViewerUtils::GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLTranslatePlaneHandle();

    /// Destructor.
    virtual ~GLTranslatePlaneHandle();

    /// Creates a new instance of GLTranslatePlaneHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLTranslatePlaneHandle();
    }

    /// Flushes any cached data.
    static void flush(){}

    /**
     * Draws the handle.
     */
    void draw();

    /**
     * Draws the manipulator with a specific selection picker ID.
     * Typically this id is converted to a color value, and the handle is then
     * drawn with flat shading, so it can be identified in a look-up table.
     */
    void pickerDraw(int64_t pickerId);

    /// Initializes the meshes. Called at handle creation.
    void setup(const Imath::V3d& normal);

protected:
    /**
     * The transform component: xform.interactive.translate
     */
    virtual std::string getComponentName() { return "translate"; }

    /**
     * Gets the dragging plane's origin and normal. The origin is always the
     * same as in the whole manipulator and the normal is given by the normal
     * of the plane handle in world space.
     */
    bool getDraggingPlane(Vec3d& origin, Vec3d& normal);

    /**
     * Detects the amount of rotation in either orbital or linear drag,
     * depending on the direction of axis in relation to the camera direction,
     * and applies the rotation to the manipulated locations.
     */
    void drag(const Vec3d& initialPointOnPlane,
              const Vec3d& previousPointOnPlane,
              const Vec3d& currentPointOnPlane,
              const Vec2i& initialMousePosition,
              const Vec2i& previousMousePosition,
              const Vec2i& currentMousePosition,
              bool isFinal);

    /**
     * Initializes some initial values at drag start.
     */
    void startDrag(const Vec3d& initialPointOnPlane,
        const Vec2i& initialMousePosition);

    /**
     * Callback received when the drag action ends.
     */
    void endDrag();

    /**
     * Applies the given rotation transform to the given location. This
     * will also take the rotation order into account.
     */
    void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal);

    /**
     * Overridden to use the initial orientation when the drag action has
     * started.
     *
     * When the manipulation has started, we want to use the initial
     * orientation, so that manipulators whose translate operation imply an
     * orientation change do not get affected (i.e. the COI manipulators).
     *
     * For the standard Translate manipulator this makes no difference, as the
     * orientation never changes.
     */
    virtual Imath::M44d getOrientationXform(const std::string& locationPath);

private:
    /**
     * Initializes the meshes used to draw the handle.
     */
    void initMesh();

    /// The meshes of the handle. See initMesh().
    Foundry::Katana::ViewerUtils::Drawable m_mesh;
    Imath::M44d m_meshXform;

    Imath::V3d m_normal;

    Vec4f m_color;

    Vec3d m_originWorldSpace;
    Vec3d m_normalWorldSpace;
};

#endif  // GLTRANSLATEPLANEHANDLE_H_
