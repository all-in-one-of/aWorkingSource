// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLCOIHANDLE_H_
#define GLCOIHANDLE_H_

#include <string>
#include <vector>

#include "FnDrawable.h"
#include "FnGLTransformManipulator.h"

#include <FnViewer/plugin/FnManipulatorHandle.h>
#include <FnViewer/plugin/FnMathTypes.h>


/** @brief This handle is not interactive and its only purpose is to render the
 * lines that connect objects and their center of interest.
 */
class GLCoiHandle :
    public Foundry::Katana::ViewerUtils::GLTransformManipulatorHandle
{
public:
    /// Constructor.
    GLCoiHandle();

    /// Destructor.
    virtual ~GLCoiHandle();

    /// Creates a new instance of GLTranslateAxisHandle.
    static Foundry::Katana::ViewerAPI::ManipulatorHandle* create()
    {
        return new GLCoiHandle();
    }

    /// Flushes any cached data.
    static void flush(){}

    /// Overridden to render the lines that join objects and their center of
    /// interest, except for the last location, which will display the
    /// manipulator's handles.
    void draw();

    /**
     * Implemented, but it does nothing.
     */
    void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal) {}

protected:
    /**
     * Implemented, but it does nothing.
     */
    virtual std::string getComponentName() { return std::string(); }

private:
    /**
     * Draws a tiny cross at the given position using the camera properties, so
     * that the geometry is always oriented towards the camera.
     */
    void drawCross(
        const Imath::V3d& cameraPosition,
        const Imath::V3d& cameraUpDirection,
        const Imath::V3d& centerOfInterest);

    std::vector<Vec3f> m_linesVertices;
    Foundry::Katana::ViewerUtils::Drawable m_linesMesh;
    std::vector<Imath::V3d> m_centerOfInterests;
    Foundry::Katana::ViewerUtils::Drawable m_crossMesh;
};

#endif  // GLTranslateManipulator_H_
