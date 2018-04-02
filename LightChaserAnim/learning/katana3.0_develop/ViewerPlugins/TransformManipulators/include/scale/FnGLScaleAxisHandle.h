// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLSCALEAXISHANDLE_H_
#define GLSCALEAXISHANDLE_H_

#include <stdint.h>

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

/** @brief A handle to scale the selected object along an axis.
 *
 * This class represents a single handle that can be used to scale an object
 * along a specified direction axis vector. The handle is represented by a line
 * and a cube. The handle will be deformed while dragging, showing how much
 * scaling is being applied. A ghost version of the initial handle scaling will
 * also be shown so that the user can easily compare the current scaling with
 * the original at drag start.
 *
 * The handle's position will depend on the transform order. For example,
 * if the translation is applied before the scale, then the handles will be
 * placed away from the object's origin, since the scaling will pivot around
 * that point away from the origin.
 *
 * If a rotation is applied after the scaling, then the handles' orientation
 * can be deformed in non-uniform scalings, but the 3 axis will always be
 * orthogonal to each other.
 */
class GLScaleAxisHandle : public GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLScaleAxisHandle();

    /// Destructor.
    virtual ~GLScaleAxisHandle();

    /// Creates a new instance of GLScaleAxisHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLScaleAxisHandle();
    }

    /// Flushes any cached data.
    static void flush(){}

    /**
     * Draws the handle.
     */
    void draw();

    /**
     * Draws the manipulator with a specific selection picker ID.
     * Typically this ID is converted to a color value, and the handle is then
     * drawn with flat shading, so it can be identified in a look-up table.
     */
    void pickerDraw(int64_t pickerId);

    /// Initializes the axis and meshes. Called at handle creation.
    void setup(const Imath::V3d& axis);

    /// Forces the handle to only use object space orientation.
    virtual FnKat::ViewerUtils::Orientation getOrientation() { return OBJECT; }

protected:
    /**
     * The transform component: xform.interactive.scale
     */
    virtual std::string getComponentName() { return "scale"; }

    /**
     * Gets the dragging plane's origin and normal. The origin is always the
     * same as the in whole manipulator and the normal is given by the ray from
     * the camera's origin through the latest mouse coordinates.
     */
    bool getDraggingPlane(Vec3d& origin, Vec3d& normal);

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
     * Detects the amount of scale and applies it to all the manipulated
     * locations.
     */
    void drag(const Vec3d& initialPointOnPlane,
              const Vec3d& previousPointOnPlane,
              const Vec3d& currentPointOnPlane,
              const Vec2i& initialMousePosition,
              const Vec2i& previousMousePosition,
              const Vec2i& currentMousePosition,
              bool isFinal);

    /**
     * Applies the given scale transform to the given location.
     */
    void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal);

private:
    /**
     * Initializes the meshes used to draw the handle: the cube and the line.
     */
    void initMesh();

    /// The direction that this handle moves in.
    Imath::V3d m_axis;

    /// The rotation axis in world space. Cached in draw().
    Imath::V3d m_axisWorldSpace;

    /// Normal and origin on the dragging plane, which is cached once we start
    /// dragging.
    Vec3d m_originWorldSpace;
    Vec3d m_normalWorldSpace;

    /// The meshes of the handle
    Drawable m_cubeMesh;
    Drawable m_trunkMesh;

    /**
     * The transform of the meshes in relation to the handle's local transform.
     * The meshes are always the same for all 3 axis, with their scaling axis
     * being Y. This transform applies a rotation in order to align them to
     * their corresponding local axis (X,Y or Z).
     */
    Imath::M44d m_meshXform;

    Imath::M44d m_scaleXform;

    Vec4f m_axisColor;
};

#endif  // GLScaleManipulator_H_
