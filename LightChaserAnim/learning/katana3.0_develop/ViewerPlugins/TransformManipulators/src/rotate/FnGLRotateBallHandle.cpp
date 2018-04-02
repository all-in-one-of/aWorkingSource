// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "rotate/FnGLRotateBallHandle.h"
#include "rotate/FnGLRotateManipulator.h"

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

#include "FnMeshGeneration.h"
#include "FnManipulatorsConfig.h"

using namespace Foundry::Katana::ViewerUtils;
using namespace Foundry::Katana::ViewerAPI;

FnLogSetup("Viewer.GLRotateBallHandle");

GLRotateBallHandle::GLRotateBallHandle() :
    GLTransformManipulatorHandle(false)
{}

GLRotateBallHandle::~GLRotateBallHandle() {}

Orientation GLRotateBallHandle::getOrientation()
{
    // Force to be always facing the camera
    return VIEW;
}

void GLRotateBallHandle::setup()
{
    // Initialize a circle
    float radius = FnManipulators::HandleTrunkLength;
    if (isPlacedOnCenterOfInterest())
    {
        radius *= FnManipulators::RotateAroundCoiRadiusScale;
    }
    const unsigned int sections = 32;
    const float step = M_PI * 2 / sections;
    const unsigned int numVertices = sections + 1;
    const unsigned int numIndices = sections * 3;


    std::vector<Vec3f> vertices(numVertices);
    std::vector<Vec3f> normals(numVertices, Vec3f(0, 0, 1));
    std::vector<unsigned int> indices(numIndices);

    // central vertex
    vertices[0] = Vec3f(0, 0, 0);
    for (unsigned int i = 1; i <= sections; ++i)
    {
        const float angle =  M_PI * 2 - i * step;
        vertices[i] = Vec3f(cos(angle) * radius, sin(angle) * radius, 0);
    }

    unsigned int index = 0;
    for (unsigned int i = 1; i < sections; ++i)
    {
        indices[index++] = 0;
        indices[index++] = i;
        indices[index++] = i + 1;
    }
    indices[index++] = 0;
    indices[index++] = sections;
    indices[index++] = 1;

    m_mesh.setup(vertices, normals, indices);
}

void GLRotateBallHandle::draw()
{
    GLTransformManipulatorHandle::draw();

    // Get the default color
    Vec4f color = FnManipulators::RotateBallColor;
    double transparency = color.w;
    color = getDisplayColor(FnManipulators::RotateBallColor);
    color.w = transparency; // Force the color's alpha in every case

    // Use the shader and draw
    useDrawingShader(getXform(), color, true);
    m_mesh.draw();
}

void GLRotateBallHandle::pickerDraw(int64_t pickerId)
{
    GLTransformManipulatorHandle::pickerDraw(pickerId);

    // Use the shader and draw
    usePickingShader(getXform(), pickerId, m_handlePriority);
    m_mesh.draw(false);
}

bool GLRotateBallHandle::getDraggingPlane(Vec3d& origin,
    Vec3d& normal)
{
    // The axis is the inverse of the camera direction
    Imath::V3d cameraDir = toImathV3d(
        getViewport()->getActiveCamera()->getDirection());

    // Cache the normal
    m_normal = (-cameraDir).normalize();
    normal = toVec3d(m_normal);

    // The origin is the manipulator's origin
    origin = toVec3d(getOrigin());

    return true;
}

void GLRotateBallHandle::startDrag(const Vec3d& initialPointOnPlane,
    const Vec2i& initialMousePosition)
{
    GLTransformManipulatorHandle::startDrag(initialPointOnPlane,
        initialMousePosition);

    // Initializes the previous direction for the first call of drag()
    m_initialDir = getDirectionOnHemisphere(toImathV3d(initialPointOnPlane));
}

void GLRotateBallHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    // Get the direction to the projection on the hemisphere
    Imath::V3d currentDir = getDirectionOnHemisphere(
        toImathV3d(currentPointOnPlane));

    if (getTransformMode() == kAroundCenterOfInterest)
    {
        // Find angle between two vectors (vectors' length won't be 0).
        const double dotProduct = m_initialDir.dot(currentDir);
        const double cosAngle =
            dotProduct / m_initialDir.length() / currentDir.length();
        const double angle = std::acos(cosAngle);

        // And the axis.
        const Imath::V3d axis = m_initialDir.cross(currentDir);

        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyRotationAroundCoiToAllLocations(
            angle, axis, getOrientation(), isFinal);

        // Let it fall through, since we still have to change the objects'
        // orientation.
    }

    // Get the matrix that gives the rotation from the initial direction to
    // the current one
    Imath::M44d rotMatrix = Imath::rotationMatrix(m_initialDir, currentDir);

    // Set the rotation
    applyXformToAllLocations(rotMatrix, isFinal);
}

Imath::V3d GLRotateBallHandle::getDirectionOnHemisphere(
    const Imath::V3d& pointOnPlane)
{
    // Caluclate the vector currentDir that goes from the origin to a point
    // defined by the mouse pointer. If pointOnPlane is inside the sphere, then
    // calculate its projection on the hemisphere facing the camera, otherwise
    // we will use the pointOnPlane itself, which will be on the plane. This
    // plane intercepts the sphere on its equator, so if the mouse is outside
    // the sphere its projection will always be on its equator.
    Imath::V3d currentDir;
    // Get the real radius and its square
    double radius = FnManipulators::HandleTrunkLength * getScale();
    if (isPlacedOnCenterOfInterest())
    {
            radius *= FnManipulators::RotateAroundCoiRadiusScale;
    }
    double radius2 = radius * radius;

    // Check if the projected point is inside the sphere
    Imath::V3d origin = getOrigin();
    double d2 = (pointOnPlane - origin).length2();
    if (d2 <= radius2)
    {
        // Calculate the distance from pointOnPlane to the
        // hemisphere, which is the height of the hemisphere at that point
        // using Pythagoras.
        double height = sqrt(radius2 - d2);

        // Get the vector from the origin to the projected point on the
        // hemisphere
        Imath::V3d pointOnHemisphere = pointOnPlane + m_normal * height;

        // The vector from the origin to the projection on the hemisphere
        currentDir = pointOnHemisphere - origin;
    }
    else
    {
        // Simply use the point on the plane, which is equivalent to a
        // direction to a point on the equator.
        currentDir = pointOnPlane - origin;
    }

    // Transform the current direction into manipulator orientation space
    toImathMatrix44d(getXform()).inverse().multDirMatrix(currentDir, currentDir);

    return currentDir.normalized();
}

void GLRotateBallHandle::applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal)
{
    GLTransformManipulator* manipulator = getGLTransformManipulator();

    // The 'xform' matrix is relative to its initial value.
    Imath::M44d rotateMatrix =
        manipulator->getInitialRotateXformFromLocation(locationPath);

    // Apply the rotation
    Imath::M44d resultMatrix = rotateMatrix * xform;

    // Get the euler angles in degrees and XYZ order
    Imath::Eulerd euler(resultMatrix,
        manipulator->getRotationOrder(locationPath));
    Imath::V3d angles = toDegrees(euler.toXYZVector());

    // Prepare the 4 values per axis (angle, axisX, axisY, axisZ). The
    // nodes that affect interactive transforms will always set the axis
    // to their natural values (ex: X will be [1, 0, 0]), even if they were
    // tweaked upstream. Because of that we can simply harcode them here.
    Imath::V4d rotateX(angles.x, 1, 0, 0);
    Imath::V4d rotateY(angles.y, 0, 1, 0);
    Imath::V4d rotateZ(angles.z, 0, 0, 1);

    // Set the rotation back into the node graph
    manipulator->setValue(locationPath, "xform.interactive.rotateX",
        FnAttribute::DoubleAttribute(&rotateX[0], 4, 4), isFinal);

    manipulator->setValue(locationPath, "xform.interactive.rotateY",
        FnAttribute::DoubleAttribute(&rotateY[0], 4, 4), isFinal);

    manipulator->setValue(locationPath, "xform.interactive.rotateZ",
        FnAttribute::DoubleAttribute(&rotateZ[0], 4, 4), isFinal);
}

FnAttribute::Attribute GLRotateBallHandle::getOption(
        Foundry::Katana::ViewerAPI::OptionIdGenerator::value_type optionId)
{
    // For the rotate handles, we don't want to hide the pointer.
    static const OptionIdGenerator::value_type s_hideMousePointer =
        OptionIdGenerator::GenerateId("HideMousePointer");
    static const OptionIdGenerator::value_type s_newMousePos =
        OptionIdGenerator::GenerateId("NewMousePosition");
    if (s_hideMousePointer == optionId || s_newMousePos == optionId)
    {
        return FnAttribute::Attribute();
    }

    return GLTransformManipulatorHandle::getOption(optionId);
}
