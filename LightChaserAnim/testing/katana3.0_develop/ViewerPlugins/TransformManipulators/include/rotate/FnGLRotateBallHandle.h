// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLROTATEBALLHANDLE_H_
#define GLROTATEBALLHANDLE_H_

#include <stdint.h>

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

/** @brief A handle to rotate the selected object in 2 degrees of freedom.
 *
 * This class represents a central rotation handle that can be used to rotate
 * an object in two degrees of freedom that are camera aligned. It uses a
 * constant VIEW orientation for that. It uses a metaphor of a trackball in the
 * center of the manipulator that can be rotated by dragging it vertically and
 * horizontally.
 *
 * The mesh used for the manipulator, although it might look like a sphere (and
 * that is the intention), it is in reality a flat filled circle, centered at
 * the manipulator's origin and always facing the camera. It is also slightly
 * moved away from the camera, so that it doesn't overlap or occlude other
 * handles placed closer to the origin. When dragging, the mouse position is
 * transformed into a direction vector that goes from the origin of the
 * manipulator, towards a point on the imaginary hemisphere that faces
 * the camera where the mouse pointer is. The amount of rotation is given by
 * the direction difference between the original direction vector at drag start
 * and the current one. The camera up vector is used to avoid rotations along
 * the camera direction vector (perpendicular to the screen).
 */
class GLRotateBallHandle : public GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLRotateBallHandle();

    /// Destructor.
    virtual ~GLRotateBallHandle();

    /// Creates a new instance of GLRotateBallHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLRotateBallHandle();
    }

    /// Flushes any cached data.
    static void flush(){}

    /**
     * Draws the handle.
     */
    void draw();

    /**
     * Draws the manipulator with a specific selection picker ID.
     * Typically this ID is converted to a color value, and the
     * handle is then drawn with flat shading, so it can be
     * identified in a look-up table.
     */
    void pickerDraw(int64_t pickerId);

    /// Initializes the mesh.
    void setup();

    /// Always returns VIEW as the orientation for this handle.
    Orientation getOrientation();

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
     * same as the whole manipulator and the the normal is given by the camera
     * direction vector (inversed).
     */
    bool getDraggingPlane(Vec3d& origin, Vec3d& normal);

    /**
     * Initializes the initial direction vector.
     */
    void startDrag(const Vec3d& initialPointOnPlane,
        const Vec2i& initialMousePosition);

    /**
     * Calculates the current direction vector and applies the rotation given
     * by the difference in its orientation and the initial one calculated in
     * startDrag().
     */
    void drag(const Vec3d& initialPointOnPlane,
              const Vec3d& previousPointOnPlane,
              const Vec3d& currentPointOnPlane,
              const Vec2i& initialMousePosition,
              const Vec2i& previousMousePosition,
              const Vec2i& currentMousePosition,
              bool isFinal);

    /**
     * Applies the given rotation transform to the given location. This
     * will also take the rotation order into account.
     */
    void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal);

private:
    /**
     * Given a point on the dragging manipulation plane, this gets the
     * corresponding point on the imaginary hemisphere above. This hemisphere
     * will always be facing the camera and is fundamental in the calculation
     * of the direction vectors (see startDrag() and drag()).
     */
    Imath::V3d getDirectionOnHemisphere(const Imath::V3d& pointOnPlane);

    /// The mesh of the filled circle.
    Drawable m_mesh;

    /// Caches the interaction plane normal. Set in getDraggingPlane()
    Imath::V3d m_normal;

    /**
    *   Caches the direction to the mouse projection on the hemisphere at drag
    *   start.
    */
    Imath::V3d m_initialDir;
};

#endif  // GLROTATEBALLHANDLE_H_
