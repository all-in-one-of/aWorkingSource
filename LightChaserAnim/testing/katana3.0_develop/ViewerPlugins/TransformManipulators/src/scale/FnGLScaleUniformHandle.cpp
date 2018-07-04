// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "scale/FnGLScaleUniformHandle.h"
#include "scale/FnGLScaleManipulator.h"
#include "FnMeshGeneration.h"
#include "FnManipulatorsConfig.h"
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include <FnViewer/plugin/FnViewportCamera.h>

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#endif
#include <GL/glew.h>
#include <GL/glu.h>

#include <math.h>

#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>

#define SCALE_CENTER_CUBE_COLOR Vec4f(0, 1, 1, 1)

using namespace Foundry::Katana::ViewerUtils;
using namespace Foundry::Katana::ViewerAPI;

// --- GLScaleUniformHandle --------------------

FnLogSetup("Viewer.GLScaleUniformHandle");

GLScaleUniformHandle::GLScaleUniformHandle() :
    GLTransformManipulatorHandle(false)
{
    m_handlePriority = 1;
}

GLScaleUniformHandle::~GLScaleUniformHandle() {}

void GLScaleUniformHandle::setup()
{
    // generate cube mesh
    generateCube(
        Vec3f(0.f, 0.f, 0.f),
        FnManipulators::ScaleUniformCubeLength,
        m_cubeMesh);
}

void GLScaleUniformHandle::draw()
{
    GLTransformManipulatorHandle::draw();

    // Use the shader and draw
    useDrawingShader(
        getXform(),
        getDisplayColor(FnManipulators::HandleColorCentre),
        true);
    m_cubeMesh.draw();
}

void GLScaleUniformHandle::pickerDraw(int64_t pickerId)
{
    GLTransformManipulatorHandle::pickerDraw(pickerId);

    // Use the shader and draw
    usePickingShader(getXform(), pickerId, m_handlePriority);
    m_cubeMesh.draw(false);
}

bool GLScaleUniformHandle::getDraggingPlane(Vec3d& origin,
    Vec3d& normal)
{
     // The axis is the inverse of the camera direction
    Imath::V3d cameraDir = toImathV3d(
        getViewport()->getActiveCamera()->getDirection());
    normal = toVec3d((-cameraDir).normalize());

    // The origin is the manipulator's origin
    origin = toVec3d(
        toImathMatrix44d(getManipulator()->getXform()).translation());

    return true;
}

void GLScaleUniformHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    const double scaleMultiplier = 5.0;
    ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();

    // Get the world to screen transform
    const double* v = camera->getViewMatrix();
    const double* p = camera->getProjectionMatrix();
    Imath::M44d viewMatrix(v[0],  v[1],  v[2],  v[3],
                           v[4],  v[5],  v[6],  v[7],
                           v[8],  v[9],  v[10], v[11],
                           v[12], v[13], v[14], v[15]);
    Imath::M44d projMatrix(p[0],  p[1],  p[2],  p[3],
                           p[4],  p[5],  p[6],  p[7],
                           p[8],  p[9],  p[10], p[11],
                           p[12], p[13], p[14], p[15]);
    Imath::M44d toScreenXform = viewMatrix * projMatrix;

    // Get the current and previous points on plane in screen space
    Imath::V3d current = toImathV3d(currentPointOnPlane) * toScreenXform;
    Imath::V3d initial = toImathV3d(initialPointOnPlane) * toScreenXform;

    // Use the delta in X for the scale factor
    double scale = (current.x - initial.x) * scaleMultiplier + 1;

    // Create a uniform scale matrix
    m_scaleXform.makeIdentity();
    m_scaleXform.setScale(scale);

    applyXformToAllLocations(m_scaleXform, isFinal);
}

void GLScaleUniformHandle::applyXformToLocation(
    const std::string& locationPath, const Imath::M44d& xform, bool isFinal)
{
    GLTransformManipulator* manipulator = getGLTransformManipulator();

    const Imath::M44d initialXform =
        manipulator->getInitialScaleXformFromLocation(locationPath);

    // Apply the scale transform to the initial xform
    Imath::M44d transformMatrix = initialXform * xform;
    Imath::M33d scaleMatrix(
        transformMatrix[0][0], transformMatrix[0][1], transformMatrix[0][2],
        transformMatrix[1][0], transformMatrix[1][1], transformMatrix[1][2],
        transformMatrix[2][0], transformMatrix[2][1], transformMatrix[2][2]
    );

    // Extract the resulting scale values
    Imath::V3d scale(1, 1, 1);
    scale = scale * scaleMatrix;

    // Set the new scale in the node graph
    manipulator->setValue(locationPath, "xform.interactive.scale",
        FnAttribute::DoubleAttribute(&scale[0], 3, 3), isFinal);
}
