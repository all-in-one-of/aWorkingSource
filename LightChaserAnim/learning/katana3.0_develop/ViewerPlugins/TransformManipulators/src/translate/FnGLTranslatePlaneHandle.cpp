// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "translate/FnGLTranslatePlaneHandle.h"
#include "translate/FnGLTranslateManipulator.h"
#include "FnManipulatorsConfig.h"
#include "FnMeshGeneration.h"
#include <FnViewer/plugin/FnViewportCamera.h>
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>

#include <algorithm>
#include <math.h>
#include <vector>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>

using Foundry::Katana::ViewerAPI::Vec4f;
using Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr;

using Foundry::Katana::ViewerUtils::GLStateRestore;
using Foundry::Katana::ViewerUtils::toImathMatrix44d;
using Foundry::Katana::ViewerUtils::toImathV3d;
using Foundry::Katana::ViewerUtils::toMatrix44d;
using Foundry::Katana::ViewerUtils::toVec3d;

FnLogSetup("Viewer.GLTranslatePlaneHandle");

GLTranslatePlaneHandle::GLTranslatePlaneHandle() :
    GLTransformManipulatorHandle(true)  // Rendered a the object's origin
{
    m_handlePriority = 1;
}

GLTranslatePlaneHandle::~GLTranslatePlaneHandle()
{
}

void GLTranslatePlaneHandle::initMesh()
{
    const float scale = isPlacedOnCenterOfInterest() ?
            FnManipulators::TranslateCoiScale : 1.0f;

    // Generate the triangle.
    FnKat::ViewerUtils::generateTriangle(
        Vec3f(0.0f, 0.0f, 0.0f),
        FnManipulators::TranslatePlaneTriangleLength * scale,
        FnManipulators::HandlePlaneThickness * scale,
        m_mesh);

    // Translate the triangle away from the origin.
    const float distanceToOrigin =
        (isPlacedOnCenterOfInterest() ?
            FnManipulators::TranslateCoiPlaneDistanceToOrigin:
            FnManipulators::TranslatePlaneDistanceToOrigin) +
        FnManipulators::TranslatePlaneTriangleLength * scale * 0.5f;
    m_meshXform.setTranslation(Imath::V3d(0.0, 1.0, 1.0) * distanceToOrigin);

    const bool useAlternativeColor = isPlacedOnCenterOfInterest();

    // Set the color for each triangle.
    // For the XY and XZ planes, we also have to rotate the triangle.
    if (m_normal.x == 1.0)
    {
        m_color = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorYZ
            : FnManipulators::HandleColorYZ;
    }
    else if (m_normal.y == 1.0)
    {
        m_color = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorXZ
            : FnManipulators::HandleColorXZ;

        m_meshXform = m_meshXform * Imath::rotationMatrix(
            Imath::V3d(1.0, 0.0, 0.0), Imath::V3d(0.0, -1.0, 0.0));
    }
    else
    {
        m_color = useAlternativeColor
            ? FnManipulators::HandleAlternativeColorXY
            : FnManipulators::HandleColorXY;

        m_meshXform = m_meshXform * Imath::rotationMatrix(
            Imath::V3d(1.0, 0.0, 0.0), Imath::V3d(0.0, 0.0, -1.0));
    }
}

void GLTranslatePlaneHandle::setup(const Imath::V3d& normal)
{
    m_normal = normal.normalized();

    // Initialize the Drawable
    initMesh();
}

void GLTranslatePlaneHandle::draw()
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

void GLTranslatePlaneHandle::pickerDraw(int64_t pickerId)
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

bool GLTranslatePlaneHandle::getDraggingPlane(Vec3d& origin, Vec3d& normal)
{
    if (!isDragging())
    {
        // We want to calculate origin and normal in world space, but only the
        // first time. This will minimize rounding errors.

        m_originWorldSpace = toVec3d(getOrigin());

        Imath::V3d normalWorldSpace;
        Imath::M44d xform = toImathMatrix44d(getXform());
        xform.multDirMatrix(m_normal, normalWorldSpace);
        m_normalWorldSpace = toVec3d(normalWorldSpace);
    }

    origin = m_originWorldSpace;
    normal = m_normalWorldSpace;

    return true;
}

void GLTranslatePlaneHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    Imath::V3d deltaWorldSpace =
        toImathV3d(currentPointOnPlane) - toImathV3d(initialPointOnPlane);

    Imath::V3d delta;
    Imath::M44d xform = toImathMatrix44d(getXform());
    Imath::M44d xformInverse = xform.inverse();
    xformInverse.multDirMatrix(deltaWorldSpace, delta);

    if (getTransformMode() == FnKat::ViewerUtils::kCenterOfInterest)
    {
        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyCoiTranslationToAllLocations(
            delta, deltaWorldSpace.length(), getOrientation(), isFinal);

        // Notice that the object itself is not translated in this mode.
    }
    else if (getTransformMode() == FnKat::ViewerUtils::kAroundCenterOfInterest)
    {
        Imath::M44d translationXform;
        translationXform.setTranslation(
            delta.normalized() * deltaWorldSpace.length());

        // Set the translation values
        applyXformToAllLocations(translationXform, isFinal);

        // Now that the objects are in their final position, we can apply the
        // rest of the manipulation (i.e. the orientation).
        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyTranslationAroundCoiToAllLocations(isFinal);
    }
    else
    {
        Imath::M44d translationXform;
        translationXform.setTranslation(
            delta.normalized() * deltaWorldSpace.length());

        // Set the translation values
        applyXformToAllLocations(translationXform, isFinal);
    }
}

void GLTranslatePlaneHandle::startDrag(const Vec3d& initialPointOnPlane,
    const Vec2i& initialMousePosition)
{
    GLTransformManipulatorHandle::startDrag(initialPointOnPlane,
        initialMousePosition);
}

void GLTranslatePlaneHandle::endDrag()
{
    GLTransformManipulatorHandle::endDrag();

    // This is to fix the mouse pointer snap issue when we're in object mode
    // for the COI manipulators.
    if (getTransformMode() == FnKat::ViewerUtils::kCenterOfInterest ||
        getTransformMode() == FnKat::ViewerUtils::kAroundCenterOfInterest)
    {
        GLTransformManipulator* const manip = getGLTransformManipulator();
        calculateAndSetLocalXform(manip->getLastLocationPath());
    }
}

void GLTranslatePlaneHandle::applyXformToLocation(
    const std::string& locationPath,
    const Imath::M44d& xform,
    bool isFinal)
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

Imath::M44d GLTranslatePlaneHandle::getOrientationXform(
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
