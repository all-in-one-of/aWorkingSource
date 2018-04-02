// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLSCALEUNIFORMHANDLE_H
#define GLSCALEUNIFORMHANDLE_H

#include <stdint.h>

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

/** @brief A handle to scale the selected object uniformly in 3 dimensions.
 *
 * This class represents a central scaling handle that can be used to scale
 * an object uniformly in all 3 dimensions. The user can drag horizontally,
 * increasing the uniform scaling by dragging to the right, and decreasing by
 * dragging to the left of the origin.
 *
 * The mesh used for the manipulator is a central cube.
 */
class GLScaleUniformHandle : public GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLScaleUniformHandle();

    /// Destructor.
    virtual ~GLScaleUniformHandle();

    /// Creates a new instance of GLScaleUniformHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLScaleUniformHandle();
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

    /// Initializes the meshes. Called at handle creation.
    void setup();

    /// Forces the handle to only use object space orientation.
    virtual FnKat::ViewerUtils::Orientation getOrientation() { return OBJECT; }

protected:
    /**
     * The transform component: xform.interactive.scale
     */
    virtual std::string getComponentName() { return "scale"; }

    /**
     * Gets the dragging plane's origin and normal. The origin is always the
     * same as the in whole manipulator and the the normal is given by the
     * camera direction (inverted).
     */
    bool getDraggingPlane(Vec3d& origin, Vec3d& normal);

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
    /// The meshe of the handle
    Drawable m_cubeMesh;
    Imath::M44d m_scaleXform;
};

#endif  // GLScaleManipulator_H_
