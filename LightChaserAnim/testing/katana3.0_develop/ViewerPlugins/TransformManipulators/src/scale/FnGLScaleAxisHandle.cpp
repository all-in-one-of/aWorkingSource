// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "scale/FnGLScaleAxisHandle.h"
#include "scale/FnGLScaleManipulator.h"
#include "FnManipulatorsConfig.h"
#include "FnMeshGeneration.h"
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include <FnViewer/plugin/FnViewportCamera.h>

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#endif
#include <GL/glew.h>
#include <GL/glu.h>

#include <math.h>
#include <algorithm>

#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>

#define SCALE_LINE_LENGTH 1
#define SCALE_LINE_WIDTH 3

using namespace Foundry::Katana::ViewerUtils;
using namespace Foundry::Katana::ViewerAPI;

// --- GLScaleAxisHandle --------------------

FnLogSetup("Viewer.GLScaleAxisHandle");

GLScaleAxisHandle::GLScaleAxisHandle() :
    GLTransformManipulatorHandle(false)
{
    m_handlePriority = 1;
}

GLScaleAxisHandle::~GLScaleAxisHandle()
{}

void GLScaleAxisHandle::initMesh()
{
    const float cubeZ = FnManipulators::HandleTrunkLength +
                        (FnManipulators::ScaleCubeLength * 0.5f);

    // generate cube mesh
    generateCube(
        Vec3f(0.f, 0.f, cubeZ),
        FnManipulators::ScaleCubeLength,
        m_cubeMesh);

    // generate trunk mesh
    generateCylinder(
        Vec3f(0.f, 0.f, 0.f),
        FnManipulators::HandleTrunkRadius,
        FnManipulators::HandleTrunkRadius,
        FnManipulators::HandleTrunkLength,
        m_trunkMesh);

    m_meshXform = Imath::rotationMatrix(Imath::V3d(0.0, 0.0, 1.0), m_axis);
}

void GLScaleAxisHandle::setup(const Imath::V3d& axis)
{
    // Set the axis
    m_axis = axis.normalized();

    // Set axis color
    if (axis.x == 1.0)
    {
        m_axisColor = FnManipulators::HandleColorX;
    }
    else if (axis.y == 1.0)
    {
        m_axisColor = FnManipulators::HandleColorY;
    }
    else if (axis.z == 1.0)
    {
        m_axisColor = FnManipulators::HandleColorZ;
    }

    // Initialize the mesh
    initMesh();
}

void GLScaleAxisHandle::draw()
{
    GLTransformManipulatorHandle::draw();

    // Do not draw the handle if it's parallel to the camera.
    if (!isDragging() && isParallelToCamera(m_axis))
        return;

    const bool dragging = isDragging();

    // When dragging, we want to ignore the Z buffer and render on top.
    GLStateRestore glStateRestore(DepthBuffer);
    if (dragging)
    {
        glDepthFunc(GL_ALWAYS);
    }

    // Concatenate the handle's transform with the mesh transform
    Imath::M44d handleXform = toImathMatrix44d(getXform());

    // Use the shader and draw
    useDrawingShader(
        toMatrix44d(m_meshXform * handleXform),
        getDisplayColor(m_axisColor),
        false);
    m_trunkMesh.draw();
    m_cubeMesh.draw();

    // Calculate and cache the axis in world space
    handleXform.multDirMatrix(m_axis, m_axisWorldSpace);
    m_axisWorldSpace.normalize();
}

void GLScaleAxisHandle::pickerDraw(int64_t pickerId)
{
    GLTransformManipulatorHandle::pickerDraw(pickerId);

    // Do not draw the handle if it's parallel to the camera.
    if (!isDragging() && isParallelToCamera(m_axis))
        return;

    // Concatenate the handle's transform with the mesh transform
    Imath::M44d xform = m_meshXform * toImathMatrix44d(getXform());

    // Use the shader and draw
    usePickingShader(toMatrix44d(xform), pickerId, m_handlePriority);
    m_trunkMesh.draw(false);
    m_cubeMesh.draw(false);
}

bool GLScaleAxisHandle::getDraggingPlane(Vec3d& origin, Vec3d& normal)
{
    if (!isDragging())
    {
        // Get the camera
        ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();

        // The manipulation plane origin in world space;
        m_originWorldSpace = toVec3d(getOrigin());

        // The manipulation plane normal. This is perpendicular to the axis and
        // is aimed towards the direction of the ray from the latest mouse
        // position.
        const Vec2i mousePosition = getLatestMousePosition();
        Vec3d rayPos, rayDir;
        camera->getRay(mousePosition.x, mousePosition.y, rayPos, rayDir);
        const Imath::V3d viewVec = toImathV3d(rayDir);
        m_normalWorldSpace = toVec3d(m_axisWorldSpace.cross(viewVec).cross(
            m_axisWorldSpace).normalized());
    }

    origin = m_originWorldSpace;
    normal = m_normalWorldSpace;

    return true;
}

void GLScaleAxisHandle::startDrag(const Vec3d& initialPointOnPlane,
    const Vec2i& initialMousePosition)
{
    GLTransformManipulatorHandle::startDrag(initialPointOnPlane,
        initialMousePosition);
}

void GLScaleAxisHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    Imath::V3d origin = getOrigin();

    // The initial distance to the origin along the world space axis
    Imath::V3d initialDistance = toImathV3d(initialPointOnPlane) - origin;
    double initialDistanceOnAxis = initialDistance.dot(m_axisWorldSpace);

    // Get the distance of the current point on plane to the origin along
    // the world space axis.
    Imath::V3d distance = toImathV3d(currentPointOnPlane) - origin;
    double distanceOnAxis = distance.dot(m_axisWorldSpace);

    // Calulate the scaling amount corresponding to the current drag amount
    Imath::V3d scale = Imath::V3d(1, 1, 1);
    scale += m_axis * ((distanceOnAxis / initialDistanceOnAxis) - 1);

    // Get the matrix that scales along the axis
    m_scaleXform.makeIdentity();
    m_scaleXform.setScale(scale);

    // Set the scaling values
    applyXformToLocation("", m_scaleXform, isFinal);
}

void GLScaleAxisHandle::endDrag()
{
    GLTransformManipulatorHandle::endDrag();
}

void GLScaleAxisHandle::applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal)
{
    std::vector<std::string> locationPaths;
    getManipulator()->getLocationPaths(locationPaths);

    std::vector<std::string>::const_iterator it;
    for (it = locationPaths.begin(); it != locationPaths.end(); ++it)
    {
        FnAttribute::DoubleAttribute initialScaleAttr = getManipulator()->getValue(
            *it, "xform.interactive.initialScale");
        if (!initialScaleAttr.isValid())
        {
            continue;
        }

        FnAttribute::DoubleConstVector s = initialScaleAttr.getNearestSample(0);
        if (s.size() < 3)
        {
            continue;
        }

        GLTransformManipulator* manipulator = getGLTransformManipulator();

        // Apply the scale transform to the initial scale
        Imath::V4d scale = Imath::V4d(s[0], s[1], s[2], 1);
        scale = scale * xform;

        // Set the new scale in the node graph
        manipulator->setValue(*it, "xform.interactive.scale",
            FnAttribute::DoubleAttribute(&scale[0], 3, 3), isFinal);
    }
}
