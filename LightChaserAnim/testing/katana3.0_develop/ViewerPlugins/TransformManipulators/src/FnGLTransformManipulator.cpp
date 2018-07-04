// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "FnGLTransformManipulator.h"
#include "FnManipulatorsConfig.h"
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnGeolibServices/FnXFormUtil.h>

#include <ImathMatrixAlgo.h>
#include <ImathQuat.h>

// 'Close to parallel/perpendicular' angle tolerance in degrees
static const double kAngleTolerance = 5.0;
static const double kParallelMinAbsCosAngle =
    std::cos(kAngleTolerance * M_PI / 180.0);
static const double kPerpendicularMaxAbsCosAngle =
    std::cos((90.0 - kAngleTolerance) * M_PI / 180.0);

///////////////////////////
// GLTranslateManipulator classes implementation
///////////////////////////
namespace Foundry
{
namespace Katana
{
namespace ViewerUtils
{

GLTransformManipulator::GLTransformManipulator() :
      m_globalScale(1)
{}

GLTransformManipulator::~GLTransformManipulator() {}

void GLTransformManipulator::draw()
{
    // If there are no locations being manipulated currently, then do not draw
    // the handles.
    std::string locationPath = getLastLocationPath();
    if (!locationPath.empty())
    {
        GLManipulator::draw(); // draw handles
    }
}

void GLTransformManipulator::pickerDraw(int64_t pickerOffset)
{
    // If there are no locations being manipulated currently, then do not draw
    // the handles.
    std::string locationPath = getLastLocationPath();
    if (!locationPath.empty())
    {
        GLManipulator::pickerDraw(pickerOffset); // draw handles
    }
}

ViewerDelegateWrapperPtr GLTransformManipulator::getViewerDelegate()
{
    return getViewport()->getViewerDelegate();
}

double GLTransformManipulator::getFixedSizeScale(Imath::V3d point)
{
    ViewportWrapperPtr viewport = getViewport();

    const int width = viewport->getWidth();
    const int height = viewport->getHeight();

    Imath::M44d viewMatrix = toImathMatrix44d(viewport->getViewMatrix44d());
    Imath::M44d projMatrix = toImathMatrix44d(viewport->getProjectionMatrix());

    Imath::M44d toScreenXform = viewMatrix * projMatrix;
    Imath::M44d screenToManip = toScreenXform.inverse();

    Imath::V3d a = point * toScreenXform;
    Imath::V3d b(a.x, a.y, a.z);

    // This is a pre-defined magic number to provide the manipulators with
    // a decent size, given the default global scale (1.0) and their current
    // drawables' size.
    const double magicFactor = 120.0;

    if (width < height)
    {
        b.x += m_globalScale * magicFactor / width;
    }
    else
    {
        b.y += m_globalScale * magicFactor / height;
    }

    return (a * screenToManip - b * screenToManip).length();
}

Imath::M44d GLTransformManipulator::getInitialTranslateXformFromLocation(
    const std::string& locationPath)
{
    Imath::M44d translateMatrix;

    // Get the current translate matrix (in component space). Leave it as
    // identity if there is no attribute or it doesn't have 3 doubles
    FnAttribute::DoubleAttribute transAttr = getValue(
        locationPath, "xform.interactive.initialTranslate");

    if (transAttr.isValid())
    {
        FnAttribute::DoubleConstVector t = transAttr.getNearestSample(0);
        if (t.size() == 3)
        {
            translateMatrix.setTranslation(Imath::V3d(t[0], t[1], t[2]));
        }
    }

    return translateMatrix;
}

Imath::M44d GLTransformManipulator::getTranslateXformFromLocation(
    const std::string& locationPath)
{
    Imath::M44d translateMatrix;

    // Get the current translate matrix (in component space). Leave it as
    // identity if there is no attribute or it doesn't have 3 doubles
    FnAttribute::DoubleAttribute transAttr = getValue(
        locationPath, "xform.interactive.translate");

    if (transAttr.isValid())
    {
        FnAttribute::DoubleConstVector t = transAttr.getNearestSample(0);
        if (t.size() == 3)
        {
            translateMatrix.setTranslation(Imath::V3d(t[0], t[1], t[2]));
        }
    }

    return translateMatrix;
}

Imath::M44d GLTransformManipulator::getInitialRotateXformFromLocation(
    const std::string& locationPath)
{
    Imath::M44d rotateMatrix;

    FnAttribute::DoubleAttribute rotateXAttr =
        getValue(locationPath, "xform.interactive.initialRotateX");
    FnAttribute::DoubleAttribute rotateYAttr =
        getValue(locationPath, "xform.interactive.initialRotateY");
    FnAttribute::DoubleAttribute rotateZAttr =
        getValue(locationPath, "xform.interactive.initialRotateZ");

    if (rotateXAttr.isValid() && rotateYAttr.isValid() && rotateZAttr.isValid())
    {
        Imath::V3d rotationVec(rotateXAttr.getValue(),
                               rotateYAttr.getValue(),
                               rotateZAttr.getValue());
        // Convert to radians
        rotationVec = toRadians(rotationVec);

        Imath::Eulerd euler(getRotationOrder(locationPath));
        euler.setXYZVector(rotationVec);
        rotateMatrix = euler.toMatrix44();
    }

    return rotateMatrix;
}

Imath::M44d GLTransformManipulator::getRotateXformFromLocation(
    const std::string& locationPath)
{
    Imath::M44d rotateMatrix;

    FnAttribute::DoubleAttribute rotateXAttr =
        getValue(locationPath, "xform.interactive.rotateX");
    FnAttribute::DoubleAttribute rotateYAttr =
        getValue(locationPath, "xform.interactive.rotateY");
    FnAttribute::DoubleAttribute rotateZAttr =
        getValue(locationPath, "xform.interactive.rotateZ");

    if (rotateXAttr.isValid() && rotateYAttr.isValid() && rotateZAttr.isValid())
    {
        Imath::V3d rotationVec(rotateXAttr.getValue(),
                               rotateYAttr.getValue(),
                               rotateZAttr.getValue());
        // Convert to radians
        rotationVec = toRadians(rotationVec);

        Imath::Eulerd euler(getRotationOrder(locationPath));
        euler.setXYZVector(rotationVec);
        rotateMatrix = euler.toMatrix44();
    }

    return rotateMatrix;
}

Imath::M44d GLTransformManipulator::getInitialScaleXformFromLocation(
    const std::string& locationPath)
{
    // Get the current scale matrix (in component space). Leave it as identity
    // if there is no scale attribute or it doesn't have 3 doubles
    Imath::M44d scaleMatrix;

     FnAttribute::DoubleAttribute scaleAttr = getValue(
        locationPath, "xform.interactive.initialScale");

    if (scaleAttr.isValid())
    {
        FnAttribute::DoubleConstVector s = scaleAttr.getNearestSample(0);
        if (s.size() == 3)
        {
            scaleMatrix.setScale(Imath::V3d(s[0], s[1], s[2]));
        }
    }

    return scaleMatrix;
}

Imath::M44d GLTransformManipulator::getScaleXformFromLocation(
    const std::string& locationPath)
{
    // Get the current scale matrix (in component space). Leave it as identity
    // if there is no scale attribute or it doesn't have 3 doubles
    Imath::M44d scaleMatrix;

     FnAttribute::DoubleAttribute scaleAttr = getValue(
        locationPath, "xform.interactive.scale");

    if (scaleAttr.isValid())
    {
        FnAttribute::DoubleConstVector s = scaleAttr.getNearestSample(0);
        if (s.size() == 3)
        {
            scaleMatrix.setScale(Imath::V3d(s[0], s[1], s[2]));
        }
    }

    return scaleMatrix;
}

Imath::V3d GLTransformManipulator::getInitialPositionFromLocation(
    const std::string& locationPath)
{
    const FnAttribute::DoubleAttribute attr = getValue(
        locationPath, "_precalculated.initialPosition");
    const FnAttribute::DoubleConstVector v = attr.getNearestSample(0);
    return v.size() == 3 ? Imath::V3d(v[0], v[1], v[2]) : Imath::V3d();
}

Imath::V3d GLTransformManipulator::getInitialCoiFromLocation(
    const std::string& locationPath)
{
    const FnAttribute::DoubleAttribute attr = getValue(
        locationPath, "_precalculated.initialCenterOfInterest");
    const FnAttribute::DoubleConstVector v = attr.getNearestSample(0);
    return v.size() == 3 ? Imath::V3d(v[0], v[1], v[2]) : Imath::V3d();
}

Imath::M44d GLTransformManipulator::getInitialOrientationXform(
    const std::string& locationPath,
    Orientation orient)
{
    // Set the xform matrix according to the manip space
    Imath::M44d rotateMatrix;
    switch (orient)
    {
    case VIEW:
    {
        ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();

        // The z axis
        Imath::V3d zDir(0.0, 0.0, -1.0);

        // Make sure that the camera up vector is kept intact
        rotateMatrix = Imath::rotationMatrixWithUpDir(
            zDir,
            toImathV3d(camera->getDirection()) * -1.0,
            toImathV3d(camera->getUp()));

        break;
    }
    case OBJECT:
    {
        // Get the initial world xform (since this is cached when the drag
        // starts, it may fail).
        const FnAttribute::DoubleAttribute attr = getValue(
            locationPath, "_precalculated.initialWorldXform");
        const FnAttribute::DoubleAttribute::array_type xformVector =
            attr.getNearestSample(0.0f);
        if (xformVector.size() == 16)
        {
            memcpy(rotateMatrix.x, xformVector.data(), sizeof(double) * 16);
        }
        else
        {
            // If it fails, use the current orientation of the object.
            rotateMatrix = toImathMatrix44d(
                getViewerDelegate()->getWorldXform(locationPath));
        }

        Imath::removeScalingAndShear(rotateMatrix, false);
        rotateMatrix = Imath::extractQuat(rotateMatrix).toMatrix44();

        break;
    }
    case WORLD:
    default:
    {
        // Leave the rotate matrix as identity
        break;
    }
    }

    return rotateMatrix;
}

Imath::M44d GLTransformManipulator::getOrientationXform(
    const std::string& locationPath,
    Foundry::Katana::ViewerUtils::Orientation orient)
{
    // Set the xform matrix according to the manip space
    Imath::M44d rotateMatrix;
    switch (orient)
    {
    case VIEW:
    {
        ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();

        // The z axis
        Imath::V3d zDir(0.0, 0.0, -1.0);

        // Make sure that the camera up vector is kept intact
        rotateMatrix = Imath::rotationMatrixWithUpDir(
            zDir,
            toImathV3d(camera->getDirection()) * -1.0,
            toImathV3d(camera->getUp()));

        break;
    }
    case OBJECT:
    {
        // Use only the rotation
        rotateMatrix = toImathMatrix44d(
            getViewerDelegate()->getWorldXform(locationPath));

        Imath::removeScalingAndShear(rotateMatrix, false);
        rotateMatrix = Imath::extractQuat(rotateMatrix).toMatrix44();

        break;
    }
    case WORLD:
    default:
    {
        // Leave the rotate matrix as identity
        break;
    }
    }

    return rotateMatrix;
}

void GLTransformManipulator::applyRotationAroundCoiToAllLocations(
    double angle,
    const Imath::V3d& axis,
    Orientation orientation,
    bool isFinal)
{
    std::vector<std::string> paths;
    getLocationPaths(paths);

    for (std::vector<std::string>::const_iterator it = paths.begin();
         it != paths.end(); ++it)
    {
        // Get the object's position.
        const Imath::V3d initialObjectPos = getInitialPositionFromLocation(*it);

        // Get the initial COI (it won't move).
        const Imath::V3d initialCoi = getInitialCoiFromLocation(*it);

        // Calculate the axis in world space using the orientation and the local
        // axis.
        const Imath::M44d orientationXform =
            getInitialOrientationXform(*it, orientation);
        Imath::V3d axisWorldSpace;
        orientationXform.multDirMatrix(axis, axisWorldSpace);
        axisWorldSpace.normalize();

        // Calculate initial and current directions.
        const Imath::V3d initialDirection = initialCoi - initialObjectPos;
        Imath::V3d currentDirection;
        Imath::M44d rotateMatrix;
        rotateMatrix.setAxisAngle(axisWorldSpace, angle);
        rotateMatrix.multDirMatrix(initialDirection, currentDirection);

        // Calculate the current position using the current direction.
        const Imath::V3d currentObjectPos = initialCoi - currentDirection;

        // And change the location as well.
        applyTranslationToLocation(
            *it, currentObjectPos - initialObjectPos, orientation, isFinal);
    }
}

void GLTransformManipulator::applyCoiTranslationToAllLocations(
    const Imath::V3d& delta,
    double distance,
    Orientation orientation,
    bool isFinal)
{
    std::vector<std::string> paths;
    getLocationPaths(paths);

    for (std::vector<std::string>::const_iterator it = paths.begin();
         it != paths.end(); ++it)
    {
        // Calculate the delta in world space using the manipulation delta
        // and the orientation.
        const Imath::M44d orientXform =
            getInitialOrientationXform(*it, orientation);
        Imath::V3d deltaWorldSpace;
        orientXform.multVecMatrix(delta, deltaWorldSpace);
        deltaWorldSpace.normalize();

        // Get the object's position (it won't move, it's the COI that changes).
        const Imath::V3d objectPos = getInitialPositionFromLocation(*it);

        // Get both the initial COI and COI in world space.
        const Imath::V3d initialCoi = getInitialCoiFromLocation(*it);
        const Imath::V3d currentCoi = initialCoi + deltaWorldSpace * distance;

        // Calculate initial and current directions.
        const Imath::V3d initialDirection = initialCoi - objectPos;
        const Imath::V3d currentDirection = currentCoi - objectPos;

        // Calculate the rotate matrix to change from previous direction to the
        // current direction. Up vector always looking at the sky.
        const Imath::M44d rotateMatrix = Imath::rotationMatrixWithUpDir(
            initialDirection, currentDirection, Imath::V3d(0.0, 1.0, 0.0));

        // And change the orientation for the location to be looking towards the
        // new direction.
        applyOrientationToLocation(*it, rotateMatrix, isFinal);

        // And set the center of interest.
        const double centerOfInterest = currentDirection.length();
        setValue(
            *it,
            "geometry.centerOfInterest",
            FnAttribute::DoubleAttribute(centerOfInterest),
            isFinal);
    }
}

void GLTransformManipulator::applyTranslationAroundCoiToAllLocations(
    bool isFinal)
{
    std::vector<std::string> paths;
    getLocationPaths(paths);

    for (std::vector<std::string>::const_iterator it = paths.begin();
         it != paths.end(); ++it)
    {
        // Get the current xform.
        const Imath::M44d currentXform =
            toImathMatrix44d(getViewerDelegate()->getWorldXform(*it));

        // Get the object's position and the final position.
        const Imath::V3d initialObjectPos = getInitialPositionFromLocation(*it);
        const Imath::V3d currentObjectPos = currentXform.translation();

        // Get both the initial COI (it won't move).
        const Imath::V3d initialCoi = getInitialCoiFromLocation(*it);

        // Calculate initial and current directions.
        const Imath::V3d initialDirection = initialCoi - initialObjectPos;
        const Imath::V3d currentDirection = initialCoi - currentObjectPos;

        // Calculate the rotate matrix to change from previous direction to the
        // current direction. Up vector always looking at the sky.
        const Imath::M44d rotateMatrix = Imath::rotationMatrixWithUpDir(
            initialDirection, currentDirection, Imath::V3d(0.0, 1.0, 0.0));

        // And change the orientation for the location to be looking towards the
        // new direction.
        applyOrientationToLocation(*it, rotateMatrix, isFinal);

        // And set the center of interest.
        const double centerOfInterest = currentDirection.length();
        setValue(
            *it,
            "geometry.centerOfInterest",
            FnAttribute::DoubleAttribute(centerOfInterest),
            isFinal);
    }
}

void GLTransformManipulator::applyOrientationToLocation(
    const std::string& locationPath,
    const Imath::M44d& rotateMatrix,
    bool isFinal)
{
    // Get the transform from component space to orientation space:
    //     Component -> World -> Orientation
    const Imath::M44d compToWorld = toImathMatrix44d(
        getViewerDelegate()->getPartialXform(locationPath, "interactive",
            "rotate?", false));
    const Imath::M44d orientationXform =
        getInitialOrientationXform(locationPath, FnKat::ViewerUtils::WORLD);
    const Imath::M44d worldToOrient = orientationXform.inverse();
    const Imath::M44d compToOrient = compToWorld * worldToOrient;
    const Imath::M44d compToOrientInverse = compToOrient.inverse();

    // Get the initial rotate matrix.
    const Imath::M44d initialRotateMatrix =
        getInitialRotateXformFromLocation(locationPath);

    // Apply the rotation.
    const Imath::M44d resultMatrix =
        initialRotateMatrix * compToOrient * rotateMatrix * compToOrientInverse;

    // Get the euler angles in degrees and XYZ order
    const Imath::Eulerd euler(resultMatrix, getRotationOrder(locationPath));
    const Imath::V3d angles = toDegrees(euler.toXYZVector());

    // Prepare the 4 values per axis (angle, axisX, axisY, axisZ). The
    // nodes that affect interactive transforms will always set the axis
    // to their natural values (e.g. X will be [1, 0, 0]), even if they were
    // tweaked upstream. Because of that we can simply hard-code them here.
    const Imath::V4d rotateX(angles.x, 1.0, 0.0, 0.0);
    const Imath::V4d rotateY(angles.y, 0.0, 1.0, 0.0);
    const Imath::V4d rotateZ(angles.z, 0.0, 0.0, 1.0);

    // Set the rotation and center of interest back into the node graph.
    setValue(locationPath, "xform.interactive.rotateX",
        FnAttribute::DoubleAttribute(&rotateX[0], 4, 4), isFinal);
    setValue(locationPath, "xform.interactive.rotateY",
        FnAttribute::DoubleAttribute(&rotateY[0], 4, 4), isFinal);
    setValue(locationPath, "xform.interactive.rotateZ",
        FnAttribute::DoubleAttribute(&rotateZ[0], 4, 4), isFinal);
}

void GLTransformManipulator::applyTranslationToLocation(
    const std::string& locationPath,
    const Imath::V3d& deltaWorldSpace,
    Orientation orientation,
    bool isFinal)
{
    // Get the transform from component space to orientation space:
    //     Component -> World -> Orientation
    const Imath::M44d compToWorld = toImathMatrix44d(
        getViewerDelegate()->getPartialXform(locationPath, "interactive",
            "translate", false));
    const Imath::M44d compToWorldInv = compToWorld.inverse();

    // And finally apply the translation to the object.
    Imath::M44d translateMatrix;
    translateMatrix.setTranslation(deltaWorldSpace);

    const Imath::M44d initialTranslateMatrix =
        getInitialTranslateXformFromLocation(locationPath);

    // Apply the manipulation transform to the current translate value
    const Imath::M44d resultTranslateMatrix =
        initialTranslateMatrix * compToWorld * translateMatrix * compToWorldInv;

    // Extract the resulting translation values
    const Imath::V3d translate = resultTranslateMatrix.translation();

    // Set the translation back in the node graph
    setValue(locationPath, "xform.interactive.translate",
        FnAttribute::DoubleAttribute(&translate[0], 3, 3), isFinal);
}

Imath::Eulerd::Order GLTransformManipulator::getRotationOrder(
    const std::string& locationPath)
{
    FnAttribute::GroupAttribute xformAttr =
        getViewerDelegate()->getAttributes(locationPath).getChildByName(
            "xform.interactive");
    if (!xformAttr.isValid()) { return Imath::Eulerd::XYZ; /* default */ }

    std::string order = "   "; // 3 characters
    unsigned int o = 3; // Xform attrs are in inverse order, start at the end.
    for(int i = 0; o > 0 && i < xformAttr.getNumberOfChildren(); ++i)
    {
        std::string attrName = xformAttr.getChildName(i);

        if (attrName == "rotateX") { order[--o] = 'X'; }
        else if (attrName == "rotateY") { order[--o] = 'Y'; }
        else if (attrName == "rotateZ") { order[--o] = 'Z'; }
    }

    if (order == "XYZ")
    {
        return Imath::Eulerd::XYZ;
    }
    else if (order == "XZY")
    {
        return Imath::Eulerd::XZY;
    }
    else if (order == "YXZ")
    {
        return Imath::Eulerd::YXZ;
    }
    else if (order == "YZX")
    {
        return Imath::Eulerd::YZX;
    }
    else if (order == "ZXY")
    {
        return Imath::Eulerd::ZXY;
    }
    else if (order == "ZYX")
    {
        return Imath::Eulerd::ZYX;
    }
    return Imath::Eulerd::XYZ;  // default
}

std::string GLTransformManipulator::getLastLocationPath()
{
    std::vector<std::string> locations;
    getLocationPaths(locations);
    if (locations.empty())
    {
        return "";
    }
    return locations.back();
}

void GLTransformManipulator::setOption(
    Foundry::Katana::ViewerAPI::OptionIdGenerator::value_type optionId,
    FnAttribute::Attribute attr)
{
    GLManipulator::setOption(optionId, attr);

    using Foundry::Katana::ViewerAPI::OptionIdGenerator;

    static const OptionIdGenerator::value_type s_globalScaleId =
            OptionIdGenerator::GenerateId("Manipulator.Scale");

    if (s_globalScaleId == optionId)
    {
        // For safety, all the multipliers will be ranged in [0.01, 10.0].
        double value = FnAttribute::DoubleAttribute(attr).getValue(1, false);
        value = std::max(0.01, std::min(10.0, value));

        if (m_globalScale != value)
        {
            m_globalScale = value;
            getViewport()->setDirty(true);
        }
    }
}

////////////////////////////
// GLTransformManipulatorHandle implementation
////////////////////////////
GLTransformManipulatorHandle::GLTransformManipulatorHandle(
    bool alwaysAtLocationOrigin) :
    m_handlePriority(0),
    m_alwaysAtObjectOrigin(alwaysAtLocationOrigin),
    m_manipScale(1),
    m_placeOnCoi(false),
    m_transformMode(kObject)
{}

GLTransformManipulatorHandle::~GLTransformManipulatorHandle()
{}

void GLTransformManipulatorHandle::cancelManipulation()
{
    // If it was dragging when it was cancelled, restore the
    // transform of every location.
    if (isDragging())
    {
        // Get manipulated locations
        std::vector<std::string> locationPaths;
        getManipulator()->getLocationPaths(locationPaths);

        // Iterate over all of the manipulated locations to restore the
        // transform.
        std::vector<std::string>::const_iterator it;
        for (it = locationPaths.begin(); it != locationPaths.end(); ++it)
        {
            restoreLocationXform(*it);
        }
    }
}

void GLTransformManipulatorHandle::draw()
{
    std::string location = getGLTransformManipulator()->getLastLocationPath();
    if (!location.empty())
    {
        calculateAndSetLocalXform(location);
        GLManipulatorHandle::draw();
    }
}

void GLTransformManipulatorHandle::pickerDraw(int64_t pickerOffset)
{
    std::string location = getGLTransformManipulator()->getLastLocationPath();
    if (!location.empty())
    {
        calculateAndSetLocalXform(location);
        GLManipulatorHandle::pickerDraw(pickerOffset);
    }
}

Imath::M44d GLTransformManipulatorHandle::calculateXform(
    const std::string& locationPath)
{
    // Get the translate matrix.
    if (m_placeOnCoi)
    {
        const FnAttribute::GroupAttribute attr =
            getViewerDelegate()->getAttributes(locationPath);
        const FnAttribute::DoubleAttribute coiAttr =
            attr.getChildByName("geometry.centerOfInterest");
        const double centerOfInterest = coiAttr.getValue(0, false);

        Imath::V3d objectDirection;
        const Imath::M44d xform =
            toImathMatrix44d(getViewerDelegate()->getWorldXform(locationPath));
        xform.multDirMatrix(Imath::V3d(0.0, 0.0, -1.0), objectDirection);
        objectDirection.normalize();

        m_manipOrigin = xform.translation() +
            objectDirection * centerOfInterest;
    }
    else if (m_alwaysAtObjectOrigin)
    {
        // Place the manip origin at the object origin.
        m_manipOrigin =
            toImathMatrix44d(
                getViewerDelegate()->getWorldXform(locationPath)).translation();
    }
    else
    {
        // Place the origin at the pivot, which is given by the component space
        // (example: rotate and scale)
        Imath::M44d xform =
            toImathMatrix44d(getViewerDelegate()->getPartialXform(
                locationPath, "interactive", getComponentName(), false));
        m_manipOrigin = xform.translation();
    }

    Imath::M44d translateMatrix;
    translateMatrix.setTranslation(m_manipOrigin);

    // Get the scaling matrix.
    // The scaling matrix is defined by the global size and the compensation
    // for the projection matrix, which guarantees that the manipulator will
    // have always the same size on the viewport, no matter what the camera is
    Imath::M44d scaleMatrix;
    m_manipScale =
        getGLTransformManipulator()->getFixedSizeScale(m_manipOrigin);
    scaleMatrix.setScale(m_manipScale);

    return scaleMatrix * translateMatrix;
}

void GLTransformManipulatorHandle::calculateAndSetLocalXform(
    const std::string& locationPath)
{
    setLocalXform(toMatrix44d(
        getOrientationXform(locationPath) * calculateXform(locationPath)));
}

Orientation GLTransformManipulatorHandle::getOrientation()
{
    // Get the transform space set from the Viewport.
    static const OptionIdGenerator::value_type kManipulatorOrientation =
        OptionIdGenerator::GenerateId("ManipulatorOrientation");

    const FnAttribute::IntAttribute spaceAttr =
        getViewport()->getOption(kManipulatorOrientation);
    return static_cast<Orientation>(spaceAttr.getValue(OBJECT, false));
}

Vec4f GLTransformManipulatorHandle::getDisplayColor(const Vec4f& color)
{
    Vec4f result;

    if (!getGLManipulator()->isInteractive())
    {
        result = FnManipulators::HandleColorNonInteractive;
    }
    else if (isDragging())
    {
        result = FnManipulators::HandleColorDrag;
    }
    else if (isActive())
    {
        result = FnManipulators::HandleColorActive;
    }
    else if (isHovered())
    {
        result = FnManipulators::HandleColorHover;
    }
    else
    {
        result = color;
    }

    return result;
}

GLTransformManipulator* GLTransformManipulatorHandle::getGLTransformManipulator()
{
    return dynamic_cast<GLTransformManipulator*>(getGLManipulator());
}

Imath::V3d GLTransformManipulatorHandle::getCameraRayDirection()
{
    const ViewportCameraWrapperPtr camera = getViewport()->getActiveCamera();
    const Imath::V3d manipOrigin(getOrigin());

    const Vec3d winPos = camera->projectObjectIntoWindow(toVec3d(manipOrigin));
    Vec3d rayPos, rayDir;
    camera->getRay(winPos.x, winPos.y, rayPos, rayDir);
    return toImathV3d(rayDir);
}

bool GLTransformManipulatorHandle::isParallelToCamera(
    const Imath::V3d& axis)
{
    // Get the axis direction in world space.
    Imath::V3d axisDirWorldSpace;
    const Imath::M44d handleXform = toImathMatrix44d(getXform());
    handleXform.multDirMatrix(axis, axisDirWorldSpace);
    axisDirWorldSpace.normalize();

    // Assume parallel if the angle is 10 degrees or less.
    const double cosAngle = axisDirWorldSpace.dot(getCameraRayDirection());
    const bool parallel = std::abs(cosAngle) >= kParallelMinAbsCosAngle;

    return parallel;
}

bool GLTransformManipulatorHandle::isPerpendicularToCamera(
    const Imath::V3d& axis)
{
    // Get the axis direction in world space.
    Imath::V3d axisDirWorldSpace;
    const Imath::M44d handleXform = toImathMatrix44d(getXform());
    handleXform.multDirMatrix(axis, axisDirWorldSpace);
    axisDirWorldSpace.normalize();

    // Assume perpendicular if the angle is 80 degrees or more.
    const double cosAngle = axisDirWorldSpace.dot(getCameraRayDirection());
    const bool perpendicular =
        std::abs(cosAngle) <= kPerpendicularMaxAbsCosAngle;

    return perpendicular;
}

void GLTransformManipulatorHandle::startDrag(const Vec3d& initialPointOnPlane,
    const Vec2i& initialMousePosition)
{
    // Get manipulated locations
    std::vector<std::string> locationPaths;
    getManipulator()->getLocationPaths(locationPaths);
    GLTransformManipulator* manipulator = getGLTransformManipulator();

    // Iterate over all of the manipulated locations and store the initial
    // scaling values.
    std::vector<std::string>::const_iterator it;
    for (it = locationPaths.begin(); it != locationPaths.end(); it++)
    {
        // Store the initial transform in the manipulator, so that
        // we can restore it if required.
        manipulator->setValue(*it, "xform.interactive.initialTranslate",
            manipulator->getValue(*it, "xform.interactive.translate"),
            false);

        manipulator->setValue(*it, "xform.interactive.initialScale",
            manipulator->getValue(*it, "xform.interactive.scale"),
            false);

        manipulator->setValue(*it, "xform.interactive.initialRotateX",
            manipulator->getValue(*it, "xform.interactive.rotateX"),
            false);

        manipulator->setValue(*it, "xform.interactive.initialRotateY",
            manipulator->getValue(*it, "xform.interactive.rotateY"),
            false);

        manipulator->setValue(*it, "xform.interactive.initialRotateZ",
            manipulator->getValue(*it, "xform.interactive.rotateZ"),
            false);

        manipulator->setValue(*it, "geometry.initialCenterOfInterest",
            manipulator->getValue(*it, "geometry.centerOfInterest"),
            false);

        // To speed things up, let's also cache some pre-calculated values
        // such as the position or the center of interest:

        // Position.
        const Imath::M44d xform = toImathMatrix44d(
            getViewerDelegate()->getWorldXform(*it));
        const Imath::V3d position = xform.translation();

        // Direction.
        Imath::V3d direction;
        xform.multDirMatrix(Imath::V3d(0.0, 0.0, -1.0), direction);
        direction.normalize();

        // Center of Interest.
        const FnAttribute::GroupAttribute attr =
            getViewerDelegate()->getAttributes(*it);
        const FnAttribute::DoubleAttribute coiAttr =
            attr.getChildByName("geometry.centerOfInterest");
        const Imath::V3d centerOfInterest =
            position + direction * coiAttr.getValue(0, false);

        manipulator->setValue(*it, "_precalculated.initialWorldXform",
            FnAttribute::DoubleAttribute(&xform[0][0], 16, 1), false);
        manipulator->setValue(*it, "_precalculated.initialPosition",
            FnAttribute::DoubleAttribute(&position[0], 3, 3), false);
        manipulator->setValue(*it, "_precalculated.initialCenterOfInterest",
            FnAttribute::DoubleAttribute(&centerOfInterest[0], 3, 3), false);
    }
}

void GLTransformManipulatorHandle::endDrag()
{
}

void GLTransformManipulatorHandle::applyXformToAllLocations(
    const Imath::M44d& xform, bool isFinal)
{
    // Get manipulated locations
    std::vector<std::string> locationPaths;
    getManipulator()->getLocationPaths(locationPaths);

    // Iterate over all of the manipulated locations to update the translation.
    // The resulting values that are set on the node-graph are in each of the
    // locations' parent space.
    std::vector<std::string>::const_iterator it;
    size_t i = 0;
    for (it = locationPaths.begin(); it != locationPaths.end(); ++it, ++i)
    {
        // Get the manipulation xform for the location
        Imath::M44d manipXform = applyManipulationXform(xform, *it);

        // Get the initial translate matrix (in component space)
        applyXformToLocation(*it, manipXform, isFinal);
    }
}

Imath::M44d GLTransformManipulatorHandle::getOrientationXform(
    const std::string& locationPath)
{
    return getGLTransformManipulator()->getOrientationXform(
        locationPath, getOrientation());
}

Imath::M44d GLTransformManipulatorHandle::applyManipulationXform(
    const Imath::M44d& xform,
    const std::string& locationPath)
{
    // Get the transform from component space to orientation space:
    //     Component -> World -> Orientation
    Imath::M44d compToWorld = toImathMatrix44d(
        getViewerDelegate()->getPartialXform(locationPath, "interactive",
            getComponentName(), false));

    Imath::M44d worldToOrient = getOrientationXform(locationPath).inverse();

    Imath::M44d compToOrient = compToWorld * worldToOrient;

    // Return a matrix that transforms to orientation space, apply the
    // orientation space transform and go back to component space.
    return compToOrient * xform * compToOrient.inverse();
}

void GLTransformManipulatorHandle::copyLocationAttr(
           const std::string& locationPath,
           const char* attrSrc,
           const char* attrDst)
{
    FnAttribute::Attribute attr =
        getManipulator()->getValue(locationPath, attrSrc);

    if (attr.isValid())
        getManipulator()->setValue(locationPath, attrDst, attr, false);
}

void GLTransformManipulatorHandle::restoreLocationXform(
    const std::string& locationPath)
{
    copyLocationAttr(locationPath,
            "xform.interactive.initialTranslate",
            "xform.interactive.translate");

    copyLocationAttr(locationPath,
            "xform.interactive.initialScale",
            "xform.interactive.scale");

    copyLocationAttr(locationPath,
            "xform.interactive.initialRotateX",
            "xform.interactive.rotateX");
    copyLocationAttr(locationPath,
            "xform.interactive.initialRotateY",
            "xform.interactive.rotateY");
    copyLocationAttr(locationPath,
            "xform.interactive.initialRotateZ",
            "xform.interactive.rotateZ");

    copyLocationAttr(locationPath,
            "geometry.initialCenterOfInterest",
            "geometry.centerOfInterest");
}

}
}
}
