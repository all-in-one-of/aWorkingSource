// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "rotate/FnGLRotateAxisHandle.h"
#include <math.h>
#include <FnViewer/plugin/FnViewportCamera.h>
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>
#include "FnManipulatorsConfig.h"
#include "FnMeshGeneration.h"
#include "rotate/FnGLRotateManipulator.h"

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#endif

#define EPSILON 0.000001f

using namespace Foundry::Katana::ViewerUtils;
using namespace Foundry::Katana::ViewerAPI;

FnLogSetup("Viewer.GLRotateAxisHandle");

GLRotateAxisHandle::GLRotateAxisHandle()
    : GLTransformManipulatorHandle(false)
    , m_totalAngle(0)
    , m_isOrbital(true)
    , m_isViewHandle(false)
{
    m_handlePriority = 1;
}

GLRotateAxisHandle::~GLRotateAxisHandle()
{}

void GLRotateAxisHandle::initMesh()
{
    m_meshXform = Imath::rotationMatrix(Imath::V3d(0.0, 0.0, 1.0), m_axis);

    float centerRadius = m_isViewHandle ?
        FnManipulators::RotateViewRadius :
        FnManipulators::HandleTrunkLength + FnManipulators::HandleTrunkRadius;
    if (isPlacedOnCenterOfInterest())
    {
        centerRadius *= FnManipulators::RotateAroundCoiRadiusScale;
    }

    generateTorus(
        Vec3f(0.f, 0.f, 0.f),
        centerRadius,
        FnManipulators::HandleTrunkRadius,
        m_mesh);

    // Initialize slice mesh
    const int sliceNumVertices = s_sliceSections + 2;
    const int sliceNumIndices = s_sliceSections * 3;

    m_sliceVertices = std::vector<Vec3f>(sliceNumVertices, Vec3f(0.f, 0.f, 0.f));
    std::vector<Vec3f> sliceNormals(sliceNumVertices, Vec3f(0.f, 0.f, 0.f));
    std::vector<unsigned int> sliceIndices(sliceNumIndices, 0);

    int originIndex = sliceNumVertices - 1;
    for (int i = 0; i < sliceNumIndices / 3; ++i)
    {
        sliceIndices[i * 3] = originIndex;
        sliceIndices[i * 3 + 1] = i;
        sliceIndices[i * 3 + 2] = i + 1;
    }

    m_sliceMesh.setup(
        m_sliceVertices,
        sliceNormals,
        sliceIndices);

    // Initialize the outline.
    const int outlineNumIndices = sliceNumVertices * 2;
    std::vector<unsigned int> outlineIndices(outlineNumIndices);

    for (int i = 0; i < sliceNumVertices - 1; ++i)
    {
        outlineIndices[i * 2] = i;
        outlineIndices[i * 2 + 1] = i + 1;
    }
    outlineIndices[outlineNumIndices - 2] = sliceNumVertices - 1;
    outlineIndices[outlineNumIndices - 1] = 0;

    m_sliceOutlineMesh.setup(
        m_sliceVertices,
        sliceNormals,
        outlineIndices);
}

void GLRotateAxisHandle::updateSliceMesh(
    const Imath::V3d& initialAngle,
    double angle)
{
    // Make sure the angle is never more than 2*PI
    angle = fmod(angle, M_PI * 2);

    // Calculate the angle step and its cosine and sine
    const float step = angle / s_sliceSections;
    const float sinStep = std::sin(step);
    const float cosStep = std::cos(step);

    float sliceRadius = (m_isViewHandle ?
        FnManipulators::RotateViewRadius :
        FnManipulators::HandleTrunkLength + FnManipulators::HandleTrunkRadius) *
        FnManipulators::RotateSliceRadiusScale;
    if (isPlacedOnCenterOfInterest())
    {
        sliceRadius *= FnManipulators::RotateAroundCoiRadiusScale;
    }

    Vec3f pos = Vec3f(
        initialAngle.x * sliceRadius,
        initialAngle.y * sliceRadius,
        0.f);

    for (int i = 0; i < s_sliceSections; ++i)
    {
        m_sliceVertices[i] = pos;

        // rotate pos
        float x = pos.x;
        float y = pos.y;
        pos.x = x * cosStep - y * sinStep;
        pos.y = x * sinStep + y * cosStep;
    }
    m_sliceVertices[s_sliceSections] = pos;

    // update the mesh
    m_sliceMesh.updateVertices(m_sliceVertices);
    m_sliceOutlineMesh.updateVertices(m_sliceVertices);
}

void GLRotateAxisHandle::setup(const Imath::V3d& axis)
{
    // Set the axis
    m_axis = axis.normalized();

    // This is to define an alternative color to be used when the manipulator
    // are placed on the COI.
    const bool useAlternativeColor = isPlacedOnCenterOfInterest();

    // Set axis color
    if (m_isViewHandle)
    {
        m_axisColor = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorCentreOpaque
            : FnManipulators::HandleColorCentreOpaque;
    }
    else if (axis.x == 1.0)
    {
        m_axisColor = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorX
            : FnManipulators::HandleColorX;
    }
    else if (axis.y == 1.0)
    {
        m_axisColor = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorY
            : FnManipulators::HandleColorY;
    }
    else if (axis.z == 1.0)
    {
        m_axisColor = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorZ
            : FnManipulators::HandleColorZ;
    }

    // Initialize the Drawable
    initMesh();
}

void GLRotateAxisHandle::draw()
{
    GLTransformManipulatorHandle::draw();

    // Concatenate the mesh transform with the handle's transform
    Imath::M44d handleXform = toImathMatrix44d(getXform());
    Imath::M44d xform = m_meshXform * handleXform;

    // Use the shader and draw
    useDrawingShader(
        toMatrix44d(xform),
        getDisplayColor(m_axisColor),
        false);
    m_mesh.draw();

    if (isDragging())
    {
        // This is to fully display the inner, which otherwise would appear
        // below the rotate ball.
        GLStateRestore glStateRestore(FnKat::ViewerUtils::DepthBuffer);
        glDepthFunc(GL_ALWAYS);

        useDrawingShader(
            toMatrix44d(m_initialXform),
            FnManipulators::RotateSliceColor,
            true);
        m_sliceMesh.draw(true, false); // with transparency and no culling

        useDrawingShader(
            toMatrix44d(m_initialXform),
            FnManipulators::RotateSliceOutlineColor,
            true);
        m_sliceOutlineMesh.setLineWidth(FnManipulators::RotateSliceLineWidth);
        m_sliceOutlineMesh.drawLines(true);  // with transparency
    }

    // Calculate and cache the axis in world space
    handleXform.multDirMatrix(m_axis, m_axisWorldSpace);
    m_axisWorldSpace.normalize();
}

void GLRotateAxisHandle::pickerDraw(int64_t pickerId)
{
    GLTransformManipulatorHandle::pickerDraw(pickerId);

    // Concatenate the mesh transform with the handle's transform
    Imath::M44d xform = m_meshXform * toImathMatrix44d(getXform());

    // Use the shader and draw
    usePickingShader(toMatrix44d(xform), pickerId, m_handlePriority);
    m_mesh.draw(false);
}

bool GLRotateAxisHandle::getDraggingPlane(Vec3d& origin, Vec3d& normal)
{
    if (!isDragging())
    {
        m_originWorldSpace = toVec3d(getOrigin());

        // Get the ray from the camera origin towards the last (x, y) window
        // coordinate. It will be used to calculate the angle with the axis.
        const Vec2i mousePosition = getLatestMousePosition();
        Vec3d rayPos, rayDir;
        ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();
        camera->getRay(mousePosition.x, mousePosition.y, rayPos, rayDir);
        const Imath::V3d viewVec = toImathV3d(rayDir);

        // If the angle is great enough, we'll use an orbital manipulation.
        static const double angleThreshold = 0.25;
        m_isOrbital = std::abs(viewVec.dot(m_axisWorldSpace)) > angleThreshold;

        if (m_isOrbital)
        {
            // The manipulation plane normal. In orbital mode, the normal
            // matches the axis direction in world space.
            m_normalWorldSpace = toVec3d(m_axisWorldSpace);
        }
        else
        {
            // Otherwise we use the direction of the ray from the camera origin
            // towards the last (x, y) window coordinate.
            m_normalWorldSpace = rayDir;
        }
    }

    origin = m_originWorldSpace;
    normal = m_normalWorldSpace;

    return true;
}

void GLRotateAxisHandle::startDrag(
    const Vec3d& initialPointOnPlane, const Vec2i& initialMousePosition)
{
    GLTransformManipulatorHandle::startDrag(
        initialPointOnPlane,
        initialMousePosition);

    // Collect some information used by the pie slice mesh
    m_initialXform = m_meshXform * toImathMatrix44d(getXform());
    m_initialAngle = toImathV3d(initialPointOnPlane) * m_initialXform.inverse();
    m_initialAngle.normalize();
    m_totalAngle = 0;
    updateSliceMesh(m_initialAngle, m_totalAngle);
}

void GLRotateAxisHandle::drag(
    const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    double angle = 0; // The amount of rotation

    if (m_isOrbital)
    {
        // Orbital drag:

        // Get the current and previous vectors that go from the origin of the
        // manip towards the mouse points on the dragging plane
        Imath::V3d origin = toImathV3d(m_originWorldSpace);
        Imath::V3d currentVec = toImathV3d(currentPointOnPlane) - origin;
        currentVec.normalize();
        Imath::V3d previousVec = toImathV3d(previousPointOnPlane) - origin;
        previousVec.normalize();

        // If the mouse actually moved
        if (!currentVec.equalWithAbsError(previousVec, EPSILON))
        {
            // Get the angle between the current vector and the previous one
            angle = std::acos(currentVec.dot(previousVec));

            // Correct the direction of the rotation if needed
            if (previousVec.cross(currentVec).dot(m_axisWorldSpace) < 0)
            {
                angle *= -1;
            }
        }
    }
    else
    {
        // Linear drag:
        // Amount of drag in pixels
        const Imath::V2i dragVec =
            toImathV2i(currentMousePosition - previousMousePosition);

        // Check if the drag should be horizontal or vertical by checking if
        // the axis and the camera up are closer to parallel or perpendicular.
        // Also check the direction of the resulting rotation, which depends if
        // the axis points in the same direction of the camera up (horizontal
        // drag) or of the camera left vector (in vertical drag).
        ViewportWrapperPtr viewport = getViewport();
        double dragDistance = 0;
        const double axisDotUp = m_axisWorldSpace.dot(
            toImathV3d(viewport->getActiveCamera()->getUp()));
        if (std::abs(axisDotUp) > 0.5)
        {
            // Horizontal drag distance.
            dragDistance =
                static_cast<double>(dragVec.x) / viewport->getWidth();
            if (axisDotUp < 0) { dragDistance *= -1; }
        }
        else
        {
            // Vertical drag distance.
            dragDistance =
                static_cast<double>(dragVec.y) / viewport->getHeight();
            const double axisDotLeft = m_axisWorldSpace.dot(
                toImathV3d(viewport->getActiveCamera()->getLeft()));
            if (axisDotLeft > 0) { dragDistance *= -1; }
        }

        // Dragging the mouse from the center of the Viewer to its border is
        // equivalent to a distance of 0.5 in screen space. A distance of
        // 1 will produce one full rotation (2 * PI).
        angle = dragDistance * M_PI * 2;
    }

    // Accumulate the total angle and update the pie slice mesh's vertices
    m_totalAngle += angle;
    updateSliceMesh(m_initialAngle, m_totalAngle);

    if (getTransformMode() == kAroundCenterOfInterest)
    {
        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyRotationAroundCoiToAllLocations(
            m_totalAngle, m_axis, getOrientation(), isFinal);

        // Let it fall through, since we still have to change the objects'
        // orientation.
    }

    // Set the rotation matrix using the angle relative to the previous value
    // instead of the initial value. This way we keep the delta angle as small
    // as possible, avoiding rotations > 180 deg in one update.
    Imath::M44d rotationMatrix;
    rotationMatrix.setAxisAngle(m_axis, angle);

    // Apply the rotation
    applyXformToAllLocations(rotationMatrix, isFinal);
}

void GLRotateAxisHandle::endDrag()
{
    // Reset the pie, otherwise we see artifacts when we start dragging.
    updateSliceMesh(Imath::V3d(0.0, 0.0, 0.0), 0);
}

void GLRotateAxisHandle::applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal)
{
    GLTransformManipulator* manipulator = getGLTransformManipulator();

    // Use the current rotation matrix instead of the initial. The 'xform'
    // matrix is relative to the previous value, not the initial.
    Imath::M44d rotateMatrix =
        manipulator->getRotateXformFromLocation(locationPath);

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

FnAttribute::Attribute GLRotateAxisHandle::getOption(
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
