// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLTRANSLATEAXISHANDLE_H_
#define GLTRANSLATEAXISHANDLE_H_

#include <stdint.h>
#include <string>

#include "FnGLTransformManipulator.h"
#include "FnDrawable.h"

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>

using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

/** @brief A handle to translate the selected object along an axis.
 *
 * This class represents a single handle that can be used to translate an
 * object along a specified direction axis vector. The handle is represented by
 * a line and an arrow head. The handle will be translated while dragging. A
 * ghost version of the initial handle will also be shown so that the user can
 * easily compare the current translation with the original at drag start.
 *
 * The handle's position will always be at the object's origin.
 */
class GLTranslateAxisHandle : public GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLTranslateAxisHandle();

    /// Destructor.
    virtual ~GLTranslateAxisHandle();

    /// Creates a new instance of GLTranslateAxisHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLTranslateAxisHandle();
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

protected:
    /**
     * The transform component: xform.interactive.translate
     */
    virtual std::string getComponentName() { return "translate"; }

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
     * Detects the amount of translation and applies it to all the manipulated
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
     * Applies the given translation transform to the given location.
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
     * Initializes the meshes used to draw the handle: the cube and the line.
     */
    void initMesh();

    /// The direction that this handle moves in.
    Imath::V3d m_axis;

    /// The translation axis in world space. Cached in draw().
    Imath::V3d m_axisWorldSpace;

    /// The meshes of the handle. See initMesh().
    Drawable m_coneMesh;
    Drawable m_trunkMesh;

    // Drawable for the infinite line that represents the axis where the
    // dragging is occurring.
    Drawable m_guideLineMesh;

    Vec3d m_originWorldSpace;
    Vec3d m_normalWorldSpace;

    /**
     * The transform of the meshes in relation to the handle's local transform.
     * The meshes are always the same for all 3 axis, with their translation
     * axis being Y. This transform applies a rotation in order to align them
     * to their corresponding local axis (X,Y or Z).
     */
    Imath::M44d m_meshXform;

    Vec4f m_axisColor;
};

#endif  // GLTranslateManipulator_H_
