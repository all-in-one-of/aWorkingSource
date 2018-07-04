// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "translate/FnGLTranslateAxisHandle.h"
#include "translate/FnGLTranslateManipulator.h"
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
#include <algorithm>

#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>

#include "FnMeshGeneration.h"


using namespace Foundry::Katana::ViewerUtils;
using namespace Foundry::Katana::ViewerAPI;

// --- GLTranslateAxisHandle --------------------

FnLogSetup("Viewer.GLTranslateAxisHandle");

GLTranslateAxisHandle::GLTranslateAxisHandle() :
    GLTransformManipulatorHandle(true)  // Rendered a the object's origin
{
    m_handlePriority = 1;
}

GLTranslateAxisHandle::~GLTranslateAxisHandle()
{}

void GLTranslateAxisHandle::initMesh()
{
    const float scale = isPlacedOnCenterOfInterest() ?
            FnManipulators::TranslateCoiScale : 1.0f;

    // generate cone mesh
    generateCylinder(
        Vec3f(0.f, 0.f, FnManipulators::HandleTrunkLength * scale),
        FnManipulators::TranslateConeRadius * scale,
        0.001f,
        FnManipulators::TranslateConeLength * scale,
        m_coneMesh);

    // generate trunk mesh
    generateCylinder(
        Vec3f(0.f, 0.f, 0.f),
        FnManipulators::HandleTrunkRadius * scale,
        FnManipulators::HandleTrunkRadius * scale,
        FnManipulators::HandleTrunkLength * scale,
        m_trunkMesh);

    // Vertices initialized to (0, 0, 0). They'll be updated when the drag
    // starts.
    const std::vector<Vec3f> vertices(2);
    const std::vector<Vec3f> normals(2);
    std::vector<unsigned int> indices(2);
    indices[0] = 0;
    indices[1] = 1;
    m_guideLineMesh.setup(vertices, normals, indices);
    m_guideLineMesh.setLineWidth(2);

    m_meshXform = Imath::rotationMatrix(Imath::V3d(0.0, 0.0, 1.0), m_axis);
}

void GLTranslateAxisHandle::setup(const Imath::V3d& axis)
{
    // Set the axis
    m_axis = axis.normalized();

    const bool useAlternativeColor = isPlacedOnCenterOfInterest();

    // Set axis color
    if (axis.x == 1.0)
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

    // Initialize the mesh
    initMesh();
}

void GLTranslateAxisHandle::draw()
{
    GLTransformManipulatorHandle::draw();

    // Do not draw the handle if it's parallel to the camera.
    if (!isDragging() && isParallelToCamera(m_axis))
        return;

    // Concatenate the handle's transform with the mesh transform
    const Imath::M44d xform = m_meshXform * toImathMatrix44d(getXform());

    // While dragging, render the guide line for the axis.
    if (isDragging())
    {
        const Vec4f guideColor(
            m_axisColor.x * FnManipulators::GuideFactor,
            m_axisColor.y * FnManipulators::GuideFactor,
            m_axisColor.z * FnManipulators::GuideFactor,
            m_axisColor.w);
        useDrawingShader(Matrix44d(), guideColor, true);
        m_guideLineMesh.drawLines(false);
    }

    // Get the default color
    Vec4f color = getDisplayColor(m_axisColor);

    // We want to ignore the Z buffer and render on top.
    GLStateRestore glStateRestore(DepthBuffer);
    glDepthFunc(GL_ALWAYS);

    // Use the shader and draw
    useDrawingShader(toMatrix44d(xform), color, false);
    m_trunkMesh.draw();
    m_coneMesh.draw();
}

void GLTranslateAxisHandle::pickerDraw(int64_t pickerId)
{
    GLTransformManipulatorHandle::pickerDraw(pickerId);

    // Do not draw the handle if it's parallel to the camera.
    if (!isDragging() && isParallelToCamera(m_axis))
        return;

    // Concatenate the handle's transform with the mesh transform
    const Imath::M44d xform = m_meshXform * toImathMatrix44d(getXform());

    // We want to ignore the Z buffer and render on top.
    GLStateRestore glStateRestore(DepthBuffer);
    glDepthFunc(GL_ALWAYS);

    // Use the shader and draw
    usePickingShader(toMatrix44d(xform), pickerId, m_handlePriority);
    m_trunkMesh.draw(false);
    m_coneMesh.draw(false);
}

bool GLTranslateAxisHandle::getDraggingPlane(Vec3d& origin,
    Vec3d& normal)
{
    if (!isDragging())
    {
        // Get the camera
        ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();

        // Calculate and cache the axis in world space
        toImathMatrix44d(getXform()).multDirMatrix(m_axis, m_axisWorldSpace);
        m_axisWorldSpace.normalize();

        // The manipulation plane origin in world space;
        const Imath::V3d origin(getOrigin());
        m_originWorldSpace = toVec3d(origin);

        // The manipulation plane normal. This is perpendicular to the axis and
        // is aimed towards the direction of the ray from the latest mouse
        // position.
        const Vec2i mousePosition = getLatestMousePosition();
        Vec3d rayPos, rayDir;
        camera->getRay(mousePosition.x, mousePosition.y, rayPos, rayDir);
        const Imath::V3d viewVec = toImathV3d(rayDir);
        m_normalWorldSpace = toVec3d(m_axisWorldSpace.cross(viewVec).cross(
            m_axisWorldSpace).normalized());

        // Update the guide line now.
        const double manipFixedScale = getScale() * 100000.0;
        const Imath::V3d lineStart =
            origin + m_axisWorldSpace * manipFixedScale;
        const Imath::V3d lineEnd = origin - m_axisWorldSpace * manipFixedScale;
        std::vector<Vec3f> vertices(2);
        vertices[0] = toVec3f(lineStart);
        vertices[1] = toVec3f(lineEnd);
        m_guideLineMesh.updateVertices(vertices);
    }

    origin = m_originWorldSpace;
    normal = m_normalWorldSpace;

    return true;
}

void GLTranslateAxisHandle::startDrag(const Vec3d& initialPointOnPlane,
    const Vec2i& initialMousePosition)
{
    GLTransformManipulatorHandle::startDrag(initialPointOnPlane,
        initialMousePosition);
}

void GLTranslateAxisHandle::endDrag()
{
    GLTransformManipulatorHandle::endDrag();

    // This is to fix the mouse pointer snap issue when we're in object mode
    // for the COI manipulators.
    if (getTransformMode() == kCenterOfInterest ||
        getTransformMode() == kAroundCenterOfInterest)
    {
        GLTransformManipulator* const manip = getGLTransformManipulator();
        calculateAndSetLocalXform(manip->getLastLocationPath());
    }
}

void GLTranslateAxisHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    // Project the world space delta on the axis in world space to get the
    // distance along the axis and the current point on the axis.
    const Imath::V3d delta =
        toImathV3d(currentPointOnPlane) - toImathV3d(initialPointOnPlane);
    const double distance = delta.dot(m_axisWorldSpace);

    if (getTransformMode() == kCenterOfInterest)
    {
        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyCoiTranslationToAllLocations(
            m_axis, distance, getOrientation(), isFinal);

        // Notice that the object itself is not translated in this mode.
    }
    else if (getTransformMode() == kAroundCenterOfInterest)
    {
        // Get the dragging along the axis in manipulator space.
        Imath::M44d translationXform;
        translationXform.setTranslation(m_axis * distance);

        // Set the translation values
        applyXformToAllLocations(translationXform, isFinal);

        // Now that the objects are in their final position, we can apply the
        // rest of the manipulation (i.e. the orientation).
        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyTranslationAroundCoiToAllLocations(isFinal);
    }
    else
    {
        // Get the dragging along the axis in manipulator space.
        Imath::M44d translationXform;
        translationXform.setTranslation(m_axis * distance);

        // Set the translation values
        applyXformToAllLocations(translationXform, isFinal);
    }
}

void GLTranslateAxisHandle::applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal)
{
    GLTransformManipulator* manipulator = getGLTransformManipulator();

    const Imath::M44d initialXform =
        manipulator->getInitialTranslateXformFromLocation(locationPath);

    // Apply the manipulation transform to the current translate value
    Imath::M44d translateMatrix = initialXform * xform;

    // Extract the resulting translation values
    Imath::V3d translate = translateMatrix.translation();

    // Set the translation back in the node graph
    manipulator->setValue(locationPath, "xform.interactive.translate",
        FnAttribute::DoubleAttribute(&translate[0], 3, 3), isFinal);
}

Imath::M44d GLTranslateAxisHandle::getOrientationXform(
    const std::string& locationPath)
{
    // When the manipulation has started, we want to use the initial
    // orientation, so that manipulators whose translate operation imply
    // an orientation change do not get affected (i.e. the COI manipulators).
    // For the standard Translate manipulator this makes no difference, as the
    // orientation never changes.
    if (isDragging())
    {
        return getGLTransformManipulator()->getInitialOrientationXform(
            locationPath, getOrientation());
    }

    return getGLTransformManipulator()->getOrientationXform(
        locationPath, getOrientation());
}
