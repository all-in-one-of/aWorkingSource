// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLSCALEPLANEHANDLE_H_
#define GLSCALEPLANEHANDLE_H_

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <string>

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

/** @brief A handle to scale the selected object along YZ, XZ and XY planes.
 *
 * This class represents a single handle that can be used to scale an
 * object along an plane. The handle is represented by square. Dragging the
 * square will scale the object along the plane the square is drawn
 * onto.
 */
class GLScalePlaneHandle :
        public Foundry::Katana::ViewerUtils::GLTransformManipulatorHandle
{
public:
    /**
    * Constructor.
    */
    GLScalePlaneHandle();

    /**
    * Destructor.
    */
    virtual ~GLScalePlaneHandle();

    /**
    * Creates a new instance of GLScalePlaneHandle.
    */
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLScalePlaneHandle();
    }

    /**
    * Flushes any cached data.
    */
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

    /**
    * Initializes the meshes. Called at handle creation.
    */
    void setup(const Imath::V3d& normal);

    /**
    * Forces the handle to only use object space orientation.
    */
    virtual FnKat::ViewerUtils::Orientation getOrientation()
    {
        return FnKat::ViewerUtils::OBJECT;
    }

protected:
    /**
     * The transform component: xform.interactive.scale
     */
    virtual std::string getComponentName() { return "scale"; }

    /**
     * Gets the dragging plane's origin and normal. The origin is always the
     * same as in the whole manipulator and the normal is given by the normal
     * of the plane handle in world space.
     */
    bool getDraggingPlane(
        Foundry::Katana::ViewerAPI::Vec3d& origin,
        Foundry::Katana::ViewerAPI::Vec3d& normal);

    /**
     * Detects the amount of rotation in either orbital or linear drag,
     * depending on the direction of axis in relation to the camera direction,
     * and applies the rotation to the manipulated locations.
     */
    void drag(const Foundry::Katana::ViewerAPI::Vec3d& initialPointOnPlane,
              const Foundry::Katana::ViewerAPI::Vec3d& previousPointOnPlane,
              const Foundry::Katana::ViewerAPI::Vec3d& currentPointOnPlane,
              const Foundry::Katana::ViewerAPI::Vec2i& initialMousePosition,
              const Foundry::Katana::ViewerAPI::Vec2i& previousMousePosition,
              const Foundry::Katana::ViewerAPI::Vec2i& currentMousePosition,
              bool isFinal);

    /**
     * Applies the given scale transform to the given location.
     */
    void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal);

private:
    /**
     * Initializes the meshes used to draw the handle.
     */
    void initMesh();

    /**
    * The meshes of the handle. See initMesh().
    */
    Foundry::Katana::ViewerUtils::Drawable m_mesh;
    Imath::M44d m_meshXform;

    Imath::V3d m_normal;

    Foundry::Katana::ViewerAPI::Vec4f m_color;

    Foundry::Katana::ViewerAPI::Vec3d m_originWorldSpace;
    Foundry::Katana::ViewerAPI::Vec3d m_normalWorldSpace;

    bool m_isDragInProgress;
};

#endif  // GLSCALEPLANEHANDLE_H_
