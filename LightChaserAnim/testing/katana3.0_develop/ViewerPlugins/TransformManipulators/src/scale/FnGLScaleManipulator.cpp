// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "scale/FnGLScaleManipulator.h"
#include "scale/FnGLScaleAxisHandle.h"
#include "scale/FnGLScalePlaneHandle.h"
#include "scale/FnGLScaleUniformHandle.h"

using Foundry::Katana::ViewerAPI::ManipulatorHandleWrapperPtr;

// --- GLScaleManipulator --------------------

GLScaleManipulator::GLScaleManipulator() {}

GLScaleManipulator::~GLScaleManipulator() {}

void GLScaleManipulator::setup()
{
    // X axis handle
    ManipulatorHandleWrapperPtr xHandleWrapper =
        Manipulator::addManipulatorHandle( "GLScaleAxisHandle", "ScaleAxisX");
    GLScaleAxisHandle* xHandle =
        xHandleWrapper->getPluginInstance<GLScaleAxisHandle>();
    xHandle->setup(Imath::V3d(1.0, 0.0, 0.0));

    // Y axis handle
    ManipulatorHandleWrapperPtr yHandleWrapper =
        Manipulator::addManipulatorHandle( "GLScaleAxisHandle", "ScaleAxisY");
    GLScaleAxisHandle* yHandle =
        yHandleWrapper->getPluginInstance<GLScaleAxisHandle>();
    yHandle->setup(Imath::V3d(0.0, 1.0, 0.0));

    // Z axis handle
    ManipulatorHandleWrapperPtr zHandleWrapper =
        Manipulator::addManipulatorHandle( "GLScaleAxisHandle", "ScaleAxisZ");
    GLScaleAxisHandle* zHandle =
        zHandleWrapper->getPluginInstance<GLScaleAxisHandle>();
    zHandle->setup(Imath::V3d(0.0, 0.0, 1.0));

    // Uniform handle
    ManipulatorHandleWrapperPtr uniformHandleWrapper =
        Manipulator::addManipulatorHandle( "GLScaleUniformHandle", "ScaleUniform");
    GLScaleUniformHandle* uniformHandle =
        uniformHandleWrapper->getPluginInstance<GLScaleUniformHandle>();
    uniformHandle->setup();

    // YZ plane handle
    ManipulatorHandleWrapperPtr yzHandleWrapper =
        Manipulator::addManipulatorHandle("GLScalePlaneHandle", "ScalePlaneYZ");
    GLScalePlaneHandle* yzHandle =
        yzHandleWrapper->getPluginInstance<GLScalePlaneHandle>();
    yzHandle->setup(Imath::V3d(1.0, 0.0, 0.0));

    // XZ plane handle
    ManipulatorHandleWrapperPtr xzHandleWrapper =
        Manipulator::addManipulatorHandle("GLScalePlaneHandle", "ScalePlaneXZ");
    GLScalePlaneHandle* xzHandle =
        xzHandleWrapper->getPluginInstance<GLScalePlaneHandle>();
    xzHandle->setup(Imath::V3d(0.0, 1.0, 0.0));

    // XY plane handle
    ManipulatorHandleWrapperPtr xyHandleWrapper =
        Manipulator::addManipulatorHandle("GLScalePlaneHandle", "ScalePlaneXY");
    GLScalePlaneHandle* xyHandle =
        xyHandleWrapper->getPluginInstance<GLScalePlaneHandle>();
    xyHandle->setup(Imath::V3d(0.0, 0.0, 1.0));
}
