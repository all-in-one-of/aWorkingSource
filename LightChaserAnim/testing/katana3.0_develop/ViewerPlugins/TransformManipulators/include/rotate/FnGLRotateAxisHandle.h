// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLROTATEAXISHANDLE_H_
#define GLROTATEAXISHANDLE_H_

#include <stdint.h>
#include <vector>
#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>
#include "FnDrawable.h"
#include "FnGLTransformManipulator.h"

using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

/** @brief A handle to rotate the selected object along an axis.
 *
 * This class represents a single handle that can be used to rotate an object
 * along a specified direction axis vector. It also shows how much the object
 * is being rotated with the currend drag using a pie slice mesh.
 *
 * The handle's position will depend on the transform order. For example, if
 * the translation is applied before the rotation, then the handles will be
 * placed away from the object's origin, since the rotation will pivot around
 * that point away from the origin.
 *
 * The transform created in drag() is relative to the rotation amount of the
 * previous call to drag(), which means that applyXformToLocation() will apply
 * this rotation transform to the previous value, rather than to the initial
 * rotation of the object at drag start. This allows a more stable rotation
 * calculation, given that the rotation amounts will be small on each drag.
 *
 * Depending on if the rotation plane is close to perpendicular to the screen
 * or not, the dragging will either be linear or oribital. Linear dragging is
 * used when the rotation plane is close to perpendicular to the camera/screen
 * plane. At this low gazing angles it is easier to simply allow the user to
 * lineraly drag vertically or horizontally instead of orbiting it. In orbital
 * dragging the user controls the amount of rotation by moving the mouse
 * pointer around (orbiting) the origin. This is used in higher gazing angles
 * in relation to the rotation plane and gives the user a more real-world-like
 * rotation interface.
 *
 * This also takes into account the rotation order, which doesn't affect the
 * orientation of the 3 rotation axis, but influences the rotation values
 * set by applyXformToLocation().
 */
class GLRotateAxisHandle : public GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLRotateAxisHandle();

    /// Destructor.
    virtual ~GLRotateAxisHandle();

    /// Creates a new instance of GLRotateAxisHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLRotateAxisHandle();
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
    void setup(const Imath::V3d& axis);

    /// Initializes the axis and meshes. Called at handle creation.
    /// This is a special setup function for a view handle that is always
    /// orientated towards the camera.
    void setupViewHandle()
    {
        m_isViewHandle = true;
        setup(Imath::V3d(0, 0, 1));
    }

    virtual Orientation getOrientation()
    {
        if (m_isViewHandle)
            return VIEW;

        return GLTransformManipulatorHandle::getOrientation();
    }

    /**
     * @brief Gets the option given the option ID.
     *
     * It has been overridden to disable hide the mouse pointer.
     */
    FnAttribute::Attribute getOption(
            Foundry::Katana::ViewerAPI::OptionIdGenerator::value_type optionId);

protected:
    /**
     * The transform component: xform.interactive.rotate?
     */
    virtual std::string getComponentName() { return "rotate?"; }

    /**
     * Gets the dragging plane's origin and normal. The origin is always the
     * same as in the whole manipulator and the the normal is given by the
     * rotation axis (see setup()) in world space.
     */
    bool getDraggingPlane(Vec3d& origin, Vec3d& normal);

    /**
     * Initializes the initial values for the pie slice and checks if the
     * dragging should be orbital or linear.
     */
    void startDrag(const Vec3d& initialPointOnPlane,
        const Vec2i& initialMousePosition);

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
     * Resets the pie slices to avoid artifacts when the drag starts again.
     */
    void endDrag();

    /**
     * Applies the given rotation transform to the given location. This
     * will also take the rotation order into account.
     */
    void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal);

private:
    /**
     * Initializes the meshes used to draw the handle.
     * Initializes the circle (composed of only lines) and the pie slice mesh.
     * The pie slice's vertices are not set here, as that is the responsibility
     * of updatePieSliceMesh().
     */
    void initMesh();

    /**
     * Updates the vertex positions of the pie slice mesh, used to show how
     * much rotation has been applied while dragging the handle. The vertices
     * are set according to an initial angle vector (a vector that goes from
     * the manipulator's origin to the first click point on the rotation
     * circle) and an agle that specifies how much rotation has been applied in
     * the current drag.
     */
    void updateSliceMesh(const Imath::V3d& initialAngle, double angle);

    /// The rotation axis in local space (X, Y or Z).
    Imath::V3d m_axis;

    /**
     * The rotation axis in world space. Cached in draw() because it can change
     * whenever the location's "xform" attribute is cooked.
     */
    Imath::V3d m_axisWorldSpace;

    /**
     * To cache the dragging plane when the drag starts, to minimize errors.
     */
    Vec3d m_originWorldSpace;
    Vec3d m_normalWorldSpace;

    /// The meshes of the handle. See initMesh().
    Drawable m_mesh;
    Drawable m_sliceMesh;
    Drawable m_sliceOutlineMesh;

    /// The vertex array for the pie slice. Used by updatePieSliceMesh().
    std::vector<Vec3f> m_sliceVertices;

    /**
     * The transform of the meshes in relation to the handle's local transform.
     * The meshes are always the same for all 3 axis, with their rotation axis
     * being Y. This transform applies a rotation in order to align them to
     * their corresponding local axis (X,Y or Z).
     */
    Imath::M44d m_meshXform;

    /**
     *  The pie slice mesh to world xform at drag start. This is used as the
     *  xform for the pie slice mesh. This is cached so that the slice is
     *  always drawn in the same space.
     */
    Imath::M44d m_initialXform;

    /**
     *  The angle vector at drag start, in world space. This is a normalized
     *  vector that defines the direction from the origin to the mouse point on
     *  plane at drag start. This is cached and used in the update of the pie
     *  slice mesh, defining the starting point of the slice.
     */
    Imath::V3d m_initialAngle;

    /// The total rotation angle so far, used by the pie slice.
    double m_totalAngle;

    /**
     * Specifies if the rotation type of the current drag is orbital or linear.
     * Linear dragging is used when the rotation plane is close to
     * perpendicular to the camera/screen plane. At this low gazing angles it
     * is easier to simply allow the user to lineraly drag vertically or
     * horizontally instead of orbiting it. In orbital dragging the user
     * controls the amount of rotation by moving the mouse pointer around
     * (orbiting) the origin. This is used in higher gazing angles in relation
     * to the rotation plane and gives the user a more real-world-like rotation
     * interface.
     */
    bool m_isOrbital;

    bool m_isViewHandle;

    Vec4f m_axisColor;

    static const int s_sliceSections = 64;
};

#endif  // GLROTATEAXISHANDLE_H_
