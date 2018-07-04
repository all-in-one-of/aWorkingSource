// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include <math.h>
#include <algorithm>
#include <vector>
#include <GL/glew.h>
#include <GL/glu.h>
#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>
#include <FnViewer/plugin/FnViewportCamera.h>
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include "FnManipulatorsConfig.h"
#include "FnMeshGeneration.h"
#include "scale/FnGLScalePlaneHandle.h"
#include "scale/FnGLScaleManipulator.h"

using Foundry::Katana::ViewerAPI::Vec4f;
using Foundry::Katana::ViewerAPI::Vec2i;
using Foundry::Katana::ViewerAPI::Vec3d;
using Foundry::Katana::ViewerAPI::Matrix44d;
using Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr;

using Foundry::Katana::ViewerUtils::GLStateRestore;
using Foundry::Katana::ViewerUtils::toImathMatrix44d;
using Foundry::Katana::ViewerUtils::toImathV3d;
using Foundry::Katana::ViewerUtils::toMatrix44d;
using Foundry::Katana::ViewerUtils::toVec3d;

FnLogSetup("Viewer.GLScalePlaneHandle");

GLScalePlaneHandle::GLScalePlaneHandle() :
    GLTransformManipulatorHandle(false),
    m_isDragInProgress(false)
{}

GLScalePlaneHandle::~GLScalePlaneHandle()
{
}

void GLScalePlaneHandle::initMesh()
{
    // Generate the cube.
    FnKat::ViewerUtils::generateCube(
        Vec3f(0.0f, 0.0f, 0.0f),
        FnManipulators::ScalePlaneSquareLength,
        m_mesh);

    // Set the scale for the cube, so that it resembles like a plane, not like
    // a cube.
    const double thicknessScale =
        FnManipulators::HandlePlaneThickness /
        FnManipulators::ScalePlaneSquareLength;
    m_meshXform.setScale(Imath::V3d(thicknessScale, 1.0, 1.0));

    // Translate the cube away from the origin.
    const float distanceToOrigin = FnManipulators::ScalePlaneDistanceToOrigin +
        FnManipulators::ScalePlaneSquareLength * 0.5f;
    m_meshXform.translate(Imath::V3d(0.0, 1.0, 1.0) * distanceToOrigin);

    // Set the color for each triangle.
    // For the XY and XZ planes, we also have to rotate the triangle.
    if (m_normal.x == 1.0)
    {
        m_color = FnManipulators::HandleColorYZ;
    }
    else if (m_normal.y == 1.0)
    {
        m_color = FnManipulators::HandleColorXZ;

        m_meshXform = m_meshXform * Imath::rotationMatrix(
            Imath::V3d(1.0, 0.0, 0.0), Imath::V3d(0.0, -1.0, 0.0));
    }
    else
    {
        m_color = FnManipulators::HandleColorXY;

        m_meshXform = m_meshXform * Imath::rotationMatrix(
            Imath::V3d(1.0, 0.0, 0.0), Imath::V3d(0.0, 0.0, -1.0));
    }
}

void GLScalePlaneHandle::setup(const Imath::V3d& normal)
{
    m_normal = normal.normalized();

    // Initialize the Drawable
    initMesh();
}

void GLScalePlaneHandle::draw()
{
    GLTransformManipulatorHandle::draw();

    // Do not draw the handle if its normal is perpendicular to the camera.
    if (!isDragging() && isPerpendicularToCamera(m_normal))
        return;

    // When dragging, we want to ignore the Z buffer and render on top.
    GLStateRestore glStateRestore(Foundry::Katana::ViewerUtils::DepthBuffer);
    if (isDragging())
    {
        glDepthFunc(GL_ALWAYS);
    }

    // Use the shader and draw
    useDrawingShader(toMatrix44d(m_meshXform * toImathMatrix44d(getXform())),
        getDisplayColor(m_color), false);
    m_mesh.draw(true, true);
}

void GLScalePlaneHandle::pickerDraw(int64_t pickerId)
{
    GLTransformManipulatorHandle::pickerDraw(pickerId);

    // Do not draw the handle if its normal is perpendicular to the camera.
    if (!isDragging() && isPerpendicularToCamera(m_normal))
        return;

    // Use the shader and draw
    usePickingShader(toMatrix44d(m_meshXform * toImathMatrix44d(getXform())),
        pickerId, m_handlePriority);
    m_mesh.draw(false, true);
}

bool GLScalePlaneHandle::getDraggingPlane(Vec3d& origin, Vec3d& normal)
{
    if (!m_isDragInProgress)
    {
        // We want to calculate origin and normal in world space,
        // but only the first time.
        // This will minimize rounding errors.

        m_originWorldSpace = toVec3d(getOrigin());

        Imath::V3d normalWorldSpace;
        Imath::M44d xform = toImathMatrix44d(getXform());
        xform.multDirMatrix(m_normal, normalWorldSpace);
        m_normalWorldSpace = toVec3d(normalWorldSpace);

        m_isDragInProgress = true;
    }

    origin = m_originWorldSpace;
    normal = m_normalWorldSpace;

    return true;
}

void GLScalePlaneHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    const Imath::V3d origin = getOrigin();
    const Imath::V3d initialVector = toImathV3d(initialPointOnPlane) - origin;
    const Imath::V3d currentVector = toImathV3d(currentPointOnPlane) - origin;
    const double initialDistance = initialVector.length();
    const double currentDistance = currentVector.length();
    const double sign = initialVector.dot(currentVector) > 0 ? 1 : -1;
    const double scale = sign * currentDistance / initialDistance;

    // Get the matrix that scales along the axis
    Imath::M44d scaleXform;
    if (m_normal.x == 1)
        scaleXform.setScale(Imath::V3d(1, scale, scale));
    else if (m_normal.y == 1)
        scaleXform.setScale(Imath::V3d(scale, 1, scale));
    else if (m_normal.z == 1)
        scaleXform.setScale(Imath::V3d(scale, scale, 1));

    // Set the scaling values
    applyXformToLocation("", scaleXform, isFinal);

    // Update the dragging state.
    m_isDragInProgress = !isFinal;
}

void GLScalePlaneHandle::applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal)
{
    std::vector<std::string> locationPaths;
    getManipulator()->getLocationPaths(locationPaths);

    std::vector<std::string>::const_iterator it;
    for (it = locationPaths.begin(); it != locationPaths.end(); ++it)
    {
        FnAttribute::DoubleAttribute initialScaleAttr =
            getManipulator()->getValue(*it, "xform.interactive.initialScale");
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
