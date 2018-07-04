// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "translate/FnGLTranslateScreenPlaneHandle.h"
#include "translate/FnGLTranslateManipulator.h"
#include "FnMeshGeneration.h"
#include "FnManipulatorsConfig.h"
#include <FnViewer/plugin/FnViewportCamera.h>

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
using Foundry::Katana::ViewerUtils::toVec3d;

FnLogSetup("Viewer.GLTranslateScreenPlaneHandle");

GLTranslateScreenPlaneHandle::GLTranslateScreenPlaneHandle() :
    GLTransformManipulatorHandle(true),  // Rendered a the object's origin
    m_orientation(Foundry::Katana::ViewerUtils::WORLD)
{
    m_handlePriority = 1;
}

GLTranslateScreenPlaneHandle::~GLTranslateScreenPlaneHandle()
{
}

void GLTranslateScreenPlaneHandle::setup()
{
    const float scale = isPlacedOnCenterOfInterest() ?
            FnManipulators::TranslateCoiScale : 1.0f;

    generateSquare(
        Vec3f(0.f, 0.f, 0.f),
        FnManipulators::TranslateSquareLength * scale,
        m_mesh);
}

void GLTranslateScreenPlaneHandle::draw()
{
    // Change the orientation before calling the base definition. This will make
    // the mesh to be oriented towards the camera.
    m_orientation = Foundry::Katana::ViewerUtils::VIEW;
    GLTransformManipulatorHandle::draw();
    m_orientation = Foundry::Katana::ViewerUtils::WORLD;

    // When dragging, we want to ignore the Z buffer and render on top.
    GLStateRestore glStateRestore(Foundry::Katana::ViewerUtils::DepthBuffer);
    glDepthFunc(GL_ALWAYS);

    const Vec4f color = isPlacedOnCenterOfInterest()
        ? FnManipulators::HandleAlternativeColorCentre
        : FnManipulators::HandleColorCentre;

    // Use the shader and draw
    useDrawingShader(
        getXform(),
        getDisplayColor(color),
        true);
    m_mesh.draw(true, false);
}

void GLTranslateScreenPlaneHandle::pickerDraw(int64_t pickerId)
{
    // Change the orientation before calling the base definition. This will make
    // the mesh to be oriented towards the camera.
    m_orientation = Foundry::Katana::ViewerUtils::VIEW;
    GLTransformManipulatorHandle::pickerDraw(pickerId);
    m_orientation = Foundry::Katana::ViewerUtils::WORLD;

    // When dragging, we want to ignore the Z buffer and render on top.
    GLStateRestore glStateRestore(Foundry::Katana::ViewerUtils::DepthBuffer);
    glDepthFunc(GL_ALWAYS);

    // Use the shader and draw
    usePickingShader(getXform(), pickerId, m_handlePriority);
    m_mesh.draw(false, false);
}

Foundry::Katana::ViewerUtils::Orientation
GLTranslateScreenPlaneHandle::getOrientation()
{
    return m_orientation;
}

bool GLTranslateScreenPlaneHandle::getDraggingPlane(Vec3d& origin, Vec3d& normal)
{
    if (!isDragging())
    {
        // We want to cache the manipulator position
        // to minimise rounding errors.
        m_manipOrigin = toVec3d(getOrigin());
    }
    // Get the manipulator and the camera
    GLTransformManipulator* manip = getGLTransformManipulator();
    ViewportCameraWrapperPtr camera = manip->getViewport()->getActiveCamera();

    origin = m_manipOrigin;
    normal = camera->getDirection();

    return true;
}

void GLTranslateScreenPlaneHandle::drag(const Vec3d& initialPointOnPlane,
    const Vec3d& previousPointOnPlane,
    const Vec3d& currentPointOnPlane,
    const Vec2i& initialMousePosition,
    const Vec2i& previousMousePosition,
    const Vec2i& currentMousePosition,
    bool isFinal)
{
    Imath::V3d delta =
        toImathV3d(currentPointOnPlane) - toImathV3d(initialPointOnPlane);

    if (getTransformMode() == FnKat::ViewerUtils::kCenterOfInterest)
    {
        GLTransformManipulator* const manip = getGLTransformManipulator();
        manip->applyCoiTranslationToAllLocations(
            delta, delta.length(), getOrientation(), isFinal);

        // Notice that the object itself is not translated in this mode.
    }
    else if (getTransformMode() == FnKat::ViewerUtils::kAroundCenterOfInterest)
    {
        // Get the dragging along the axis in manipulator space.
        Imath::M44d translationXform;
        translationXform.setTranslation(delta);

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
        translationXform.setTranslation(delta);

        // Set the translation values
        applyXformToAllLocations(translationXform, isFinal);
    }
}

void GLTranslateScreenPlaneHandle::startDrag(const Vec3d& initialPointOnPlane,
    const Vec2i& initialMousePosition)
{
    GLTransformManipulatorHandle::startDrag(initialPointOnPlane,
        initialMousePosition);
}

void GLTranslateScreenPlaneHandle::endDrag()
{
    GLTransformManipulatorHandle::endDrag();
}

void GLTranslateScreenPlaneHandle::applyXformToLocation(
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
