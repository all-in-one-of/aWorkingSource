// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLTRANSLATESCREENPLANEHANDLE_H_
#define GLTRANSLATESCREENPLANEHANDLE_H_

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <string>

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

using Foundry::Katana::ViewerAPI::Vec2i;
using Foundry::Katana::ViewerAPI::Vec3d;
using Foundry::Katana::ViewerAPI::Matrix44d;

/** @brief A handle to translate the selected object along the screen plane.
 *
 * This class represents a single handle that can be used to translate an
 * object along the screen space plane. The handle is represented by a cyan
 * circle oriented to the camera. The handle will be translated while
 * dragging. A ghost version of the initial handle will also be shown so that
 * the user can easily compare the current translation with the original at drag
 * start.
 *
 * The handle's position will always be at the object's origin.
 */
class GLTranslateScreenPlaneHandle :
        public Foundry::Katana::ViewerUtils::GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLTranslateScreenPlaneHandle();

    /// Destructor.
    virtual ~GLTranslateScreenPlaneHandle();

    /// Creates a new instance of GLTranslateScreenPlaneHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLTranslateScreenPlaneHandle();
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

    /// Initializes the axis and meshes. Called at handle creation.
    void setup();

    virtual Foundry::Katana::ViewerUtils::Orientation getOrientation();

protected:
    /**
     * The transform component: xform.interactive.translate
     */
    virtual std::string getComponentName() { return "translate"; }

    /**
     * Gets the dragging plane's origin and normal. The origin is always the
     * same as in the whole manipulator and the the normal is given by the
     * rotation axis (see setup()) in world space.
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

private:
    /// The meshes of the handle. See initMesh().
    Foundry::Katana::ViewerUtils::Drawable m_mesh;

    Foundry::Katana::ViewerUtils::Orientation m_orientation;
    Vec3d m_manipOrigin;
};

#endif  // GLTRANSLATESCREENPLANEHANDLE_H_
