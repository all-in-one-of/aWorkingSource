// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "coi/FnGLCoiHandle.h"

#include "translate/FnGLTranslateManipulator.h"
#include "FnManipulatorsConfig.h"
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include <FnViewer/plugin/FnViewportCamera.h>

#include <algorithm>
#include <math.h>

#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>

using Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr;
using Foundry::Katana::ViewerAPI::ViewportWrapperPtr;
using Foundry::Katana::ViewerUtils::GLStateRestore;
using Foundry::Katana::ViewerUtils::Matrix44d;
using Foundry::Katana::ViewerUtils::toImathMatrix44d;
using Foundry::Katana::ViewerUtils::toImathV3d;
using Foundry::Katana::ViewerUtils::toMatrix44d;


// --- GLCoiHandle --------------------

FnLogSetup("Viewer.GLCoiHandle");

GLCoiHandle::GLCoiHandle() :
   GLTransformManipulatorHandle(false)
{
    std::vector<Vec3f> vertices(4);
    vertices[0] = Vec3f(FnManipulators::CoiCrossLineLength, 0, 0);
    vertices[1] = Vec3f(-FnManipulators::CoiCrossLineLength, 0, 0);
    vertices[2] = Vec3f(0, FnManipulators::CoiCrossLineLength, 0);
    vertices[3] = Vec3f(0, -FnManipulators::CoiCrossLineLength, 0);

    const std::vector<Vec3f> normals(4, Vec3f(0, 0, 1));

    std::vector<unsigned int> indices(4);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 3;

    m_crossMesh.setup(vertices, normals, indices);
    m_crossMesh.setLineWidth(FnManipulators::CoiCrossLineWidth);

    m_linesMesh.setLineWidth(FnManipulators::CoiLineWidth);
}

GLCoiHandle::~GLCoiHandle()
{
}

void GLCoiHandle::draw()
{
    GLTransformManipulator* const manip = getGLTransformManipulator();

    // Get all the selected locations.
    std::vector<std::string> paths;
    manip->getLocationPaths(paths);
    if (!paths.size())
    {
        return;
    }

    const unsigned int linesCount = paths.size();
    const unsigned int linesVerticesCount = linesCount * 2;

    // If we have a different number of lines, reconstruct the mesh.
    if (linesVerticesCount != m_linesVertices.size())
    {
        m_linesVertices.resize(linesVerticesCount);
        const std::vector<Vec3f> normals(linesVerticesCount);
        std::vector<unsigned int> indices(linesVerticesCount);
        for (unsigned int i = 0; i < linesVerticesCount; ++i)
        {
            indices[i] = i;
        }
        m_linesMesh = Foundry::Katana::ViewerUtils::Drawable();
        m_linesMesh.setup(m_linesVertices, normals, indices);

        m_centerOfInterests.resize(linesCount);
    }

    // Update the lines vertices now.
    for (unsigned int i = 0; i < linesCount; ++i)
    {
        const std::string& path = paths[i];

        // Position.
        const Imath::M44d xform = toImathMatrix44d(
            getViewerDelegate()->getWorldXform(path));
        const Imath::V3d position = xform.translation();

        // Direction.
        Imath::V3d direction;
        xform.multDirMatrix(Imath::V3d(0, 0, -1), direction);
        direction.normalize();

        // Center of Interest.
        const FnAttribute::GroupAttribute attr =
            getViewerDelegate()->getAttributes(path);
        const FnAttribute::DoubleAttribute coiAttr =
            attr.getChildByName("geometry.centerOfInterest");
        const Imath::V3d centerOfInterest =
            position + direction * coiAttr.getValue(0, false);

        // Set in the vector, eventually we'll update and draw the mesh.
        m_linesVertices[i * 2] = Vec3f(position.x, position.y, position.z);
        m_linesVertices[i * 2 + 1] =
            Vec3f(centerOfInterest.x, centerOfInterest.y, centerOfInterest.z);

        // Set the center of interest.
        m_centerOfInterests[i] = centerOfInterest;
    }

    // We do not wish to write on the depth buffer.
    GLStateRestore glStateRestore(Foundry::Katana::ViewerUtils::DepthBuffer);
    glDepthMask(false);

    // And now update the vertices and draw the lines.
    useLineStippleDrawingShader(
        Matrix44d(),
        FnManipulators::CoiLineColor,
        FnManipulators::CoiLinePattern,
        FnManipulators::CoiLinePatternFactor);
    m_linesMesh.updateVertices(m_linesVertices);
    m_linesMesh.drawLines(false);

    // Get the active camera, as we're going to need the view matrix to keep
    // the crosses oriented towards the observer.
    ViewportWrapperPtr viewport = getViewport();
    ViewportCameraWrapperPtr camera = viewport->getActiveCamera();
    const Imath::V3d cameraPosition = toImathV3d(camera->getOrigin());
    const Imath::V3d cameraUpDirection = toImathV3d(camera->getUp());

    // Render the cross for all the lines (except the last one).
    const unsigned int crossCount = linesCount - 1;
    for (unsigned int i = 0; i < crossCount; ++i)
    {
        drawCross(cameraPosition, cameraUpDirection, m_centerOfInterests[i]);
    }
}

void GLCoiHandle::drawCross(
    const Imath::V3d& cameraPosition,
    const Imath::V3d& cameraUpDirection,
    const Imath::V3d& centerOfInterest)
{
    // Calculate the rotate matrix so that the cross is pointing to the observer
    // at all times.
    const Imath::V3d direction(cameraPosition - centerOfInterest);
    const Imath::M44d rotateMatrix = Imath::rotationMatrixWithUpDir(
        Imath::V3d(0, 0, 1),  // Because the cross mesh is pointing to Z+.
        direction,
        cameraUpDirection);

    // Calculate scale matrix, so that the cross is rendered at a fixed size.
    const double fixedScale =
        getGLTransformManipulator()->getFixedSizeScale(centerOfInterest);
    Imath::M44d scaleMatrix;
    scaleMatrix.setScale(fixedScale);

    // Calculate the translate matrix to render the cross on the given position.
    Imath::M44d translateMatrix;
    translateMatrix.setTranslation(centerOfInterest);

    // And do the draw.
    const Matrix44d xform =
        toMatrix44d(rotateMatrix * scaleMatrix * translateMatrix);
    useDrawingShader(xform, FnManipulators::CoiCrossColor, true);
    m_crossMesh.drawLines(false);
}
