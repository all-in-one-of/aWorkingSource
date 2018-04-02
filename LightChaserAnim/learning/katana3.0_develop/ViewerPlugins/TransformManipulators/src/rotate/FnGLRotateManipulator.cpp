// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "rotate/FnGLRotateManipulator.h"
#include "rotate/FnGLRotateAxisHandle.h"
#include "rotate/FnGLRotateBallHandle.h"

using Foundry::Katana::ViewerAPI::ManipulatorHandleWrapperPtr;

// --- GLRotateManipulator --------------------

GLRotateManipulator::GLRotateManipulator() {}

GLRotateManipulator::~GLRotateManipulator() {}

void GLRotateManipulator::setup()
{
    // Central Ball handle
    ManipulatorHandleWrapperPtr ballHandleWrapper =
        Manipulator::addManipulatorHandle("GLRotateBallHandle", "RotateBall");
    GLRotateBallHandle* ballHandle =
        ballHandleWrapper->getPluginInstance<GLRotateBallHandle>();
    ballHandle->setup();

    // X axis handle
    ManipulatorHandleWrapperPtr xHandleWrapper =
        Manipulator::addManipulatorHandle( "GLRotateAxisHandle", "RotateAxisX");
    GLRotateAxisHandle* xHandle =
        xHandleWrapper->getPluginInstance<GLRotateAxisHandle>();
    xHandle->setup(Imath::V3d(1.0, 0.0, 0.0));

    // Y axis handle
    ManipulatorHandleWrapperPtr yHandleWrapper =
        Manipulator::addManipulatorHandle( "GLRotateAxisHandle", "RotateAxisY");
    GLRotateAxisHandle* yHandle =
        yHandleWrapper->getPluginInstance<GLRotateAxisHandle>();
    yHandle->setup(Imath::V3d(0.0, 1.0, 0.0));

    // Z axis handle
    ManipulatorHandleWrapperPtr zHandleWrapper =
        Manipulator::addManipulatorHandle( "GLRotateAxisHandle", "RotateAxisZ");
    GLRotateAxisHandle* zHandle =
        zHandleWrapper->getPluginInstance<GLRotateAxisHandle>();
    zHandle->setup(Imath::V3d(0.0, 0.0, 1.0));

    // View handle
    ManipulatorHandleWrapperPtr vHandleWrapper =
        Manipulator::addManipulatorHandle("GLRotateAxisHandle", "RotateAxisV");
    GLRotateAxisHandle* vHandle =
        vHandleWrapper->getPluginInstance<GLRotateAxisHandle>();
    vHandle->setupViewHandle();
}

