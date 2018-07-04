// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "coi/FnGLCoiManipulator.h"

#include "coi/FnGLCoiHandle.h"
#include "rotate/FnGLRotateAxisHandle.h"
#include "rotate/FnGLRotateBallHandle.h"
#include "translate/FnGLTranslateAxisHandle.h"
#include "translate/FnGLTranslatePlaneHandle.h"
#include "translate/FnGLTranslateScreenPlaneHandle.h"


// --- GLCoiManipulator --------------------

GLCoiManipulator::GLCoiManipulator() {}

GLCoiManipulator::~GLCoiManipulator() {}

void GLCoiManipulator::setup()
{
    using FnKat::ViewerUtils::TransformMode;

    // COI handle. This is what renders the lines and crosses to join objects
    // and their center of interests.
    Manipulator::addManipulatorHandle("GLCoiHandle", "CoiHandle");

    // Rotate around COI manipulators.
    {
        // Central Ball handle
        addManipulatorHandle<GLRotateBallHandle>(
            "GLRotateBallHandle",
            "RotateBall",
            true,
            kAroundCenterOfInterest)->setup();

        // X axis handle
        addManipulatorHandle<GLRotateAxisHandle>(
            "GLRotateAxisHandle",
            "RotateAxisX",
            true,
            kAroundCenterOfInterest)->setup(Imath::V3d(1.0, 0.0, 0.0));

        // Y axis handle
        addManipulatorHandle<GLRotateAxisHandle>(
            "GLRotateAxisHandle",
            "RotateAxisY",
            true,
            kAroundCenterOfInterest)->setup(Imath::V3d(0.0, 1.0, 0.0));

        // Z axis handle
        addManipulatorHandle<GLRotateAxisHandle>(
            "GLRotateAxisHandle",
            "RotateAxisZ",
            true,
            kAroundCenterOfInterest)->setup(Imath::V3d(0.0, 0.0, 1.0));

        // View handle
        addManipulatorHandle<GLRotateAxisHandle>(
            "GLRotateAxisHandle",
            "RotateAxisV",
            true,
            kAroundCenterOfInterest)->setupViewHandle();
    }

    // Translate around COI manipulators.
    {
        // X axis handle
        addManipulatorHandle<GLTranslateAxisHandle>(
            "GLTranslateAxisHandle",
            "TranslateAxisX",
            false,
            kAroundCenterOfInterest)->setup(Imath::V3d(1.0, 0.0, 0.0));

        // Y axis handle
        addManipulatorHandle<GLTranslateAxisHandle>(
            "GLTranslateAxisHandle",
            "TranslateAxisY",
            false,
            kAroundCenterOfInterest)->setup(Imath::V3d(0.0, 1.0, 0.0));

        // Z axis handle
        addManipulatorHandle<GLTranslateAxisHandle>(
            "GLTranslateAxisHandle",
            "TranslateAxisZ",
            false,
            kAroundCenterOfInterest)->setup(Imath::V3d(0.0, 0.0, 1.0));

        // Free handle
        addManipulatorHandle<GLTranslateScreenPlaneHandle>(
            "GLTranslateScreenPlaneHandle",
            "TranslateScreenPlane",
            false,
            kAroundCenterOfInterest)->setup();

        // YZ plane handle
        addManipulatorHandle<GLTranslatePlaneHandle>(
            "GLTranslatePlaneHandle",
            "TranslatePlaneYZ",
            false,
            kAroundCenterOfInterest)->setup(Imath::V3d(1.0, 0.0, 0.0));

        // XZ plane handle
        addManipulatorHandle<GLTranslatePlaneHandle>(
            "GLTranslatePlaneHandle",
            "TranslatePlaneXZ",
            false,
            kAroundCenterOfInterest)->setup(Imath::V3d(0.0, 1.0, 0.0));

        // XY plane handle
        addManipulatorHandle<GLTranslatePlaneHandle>(
            "GLTranslatePlaneHandle",
            "TranslatePlaneXY",
            false,
            kAroundCenterOfInterest)->setup(Imath::V3d(0.0, 0.0, 1.0));
    }

    // Translate COI manipulators.
    {
        // X axis handle
        addManipulatorHandle<GLTranslateAxisHandle>(
            "GLTranslateAxisHandle",
            "TranslateCoiAxisX",
            true,
            kCenterOfInterest)->setup(Imath::V3d(1.0, 0.0, 0.0));

        // Y axis handle
        addManipulatorHandle<GLTranslateAxisHandle>(
            "GLTranslateAxisHandle",
            "TranslateCoiAxisY",
            true,
            kCenterOfInterest)->setup(Imath::V3d(0.0, 1.0, 0.0));

        // Z axis handle
        addManipulatorHandle<GLTranslateAxisHandle>(
            "GLTranslateAxisHandle",
            "TranslateCoiAxisZ",
            true,
            kCenterOfInterest)->setup(Imath::V3d(0.0, 0.0, 1.0));

        // Free handle
        addManipulatorHandle<GLTranslateScreenPlaneHandle>(
            "GLTranslateScreenPlaneHandle",
            "TranslateCoiScreenPlane",
            true,
            kCenterOfInterest)->setup();

        // YZ plane handle
        addManipulatorHandle<GLTranslatePlaneHandle>(
            "GLTranslatePlaneHandle",
            "TranslateCoiPlaneYZ",
            true,
            kCenterOfInterest)->setup(Imath::V3d(1.0, 0.0, 0.0));

        // XZ plane handle
        addManipulatorHandle<GLTranslatePlaneHandle>(
            "GLTranslatePlaneHandle",
            "TranslateCoiPlaneXZ",
            true,
            kCenterOfInterest)->setup(Imath::V3d(0.0, 1.0, 0.0));

        // XY plane handle
        addManipulatorHandle<GLTranslatePlaneHandle>(
            "GLTranslatePlaneHandle",
            "TranslateCoiPlaneXY",
            true,
            kCenterOfInterest)->setup(Imath::V3d(0.0, 0.0, 1.0));
    }
}
