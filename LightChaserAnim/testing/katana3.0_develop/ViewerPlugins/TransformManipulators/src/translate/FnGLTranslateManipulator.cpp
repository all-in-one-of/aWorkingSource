// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "translate/FnGLTranslateManipulator.h"
#include "translate/FnGLTranslateAxisHandle.h"
#include "translate/FnGLTranslatePlaneHandle.h"
#include "translate/FnGLTranslateScreenPlaneHandle.h"

using Foundry::Katana::ViewerAPI::ManipulatorHandleWrapperPtr;

// --- GLTranslateManipulator --------------------

GLTranslateManipulator::GLTranslateManipulator()
{}

GLTranslateManipulator::~GLTranslateManipulator() {}

void GLTranslateManipulator::setup()
{
    const std::string handleType = "GLTranslateAxisHandle";

    // X axis handle
    ManipulatorHandleWrapperPtr xHandleWrapper =
        Manipulator::addManipulatorHandle( handleType, "TranslateAxisX");
    GLTranslateAxisHandle* xHandle =
        xHandleWrapper->getPluginInstance<GLTranslateAxisHandle>();
    xHandle->setup(Imath::V3d(1.0, 0.0, 0.0));

    // Y axis handle
    ManipulatorHandleWrapperPtr yHandleWrapper =
        Manipulator::addManipulatorHandle( handleType, "TranslateAxisY");
    GLTranslateAxisHandle* yHandle =
        yHandleWrapper->getPluginInstance<GLTranslateAxisHandle>();
    yHandle->setup(Imath::V3d(0.0, 1.0, 0.0));

    // Z axis handle
    ManipulatorHandleWrapperPtr zHandleWrapper =
        Manipulator::addManipulatorHandle( handleType, "TranslateAxisZ");
    GLTranslateAxisHandle* zHandle =
        zHandleWrapper->getPluginInstance<GLTranslateAxisHandle>();
    zHandle->setup(Imath::V3d(0.0, 0.0, 1.0));

    // Free handle
    ManipulatorHandleWrapperPtr cHandleWrapper =
        Manipulator::addManipulatorHandle("GLTranslateScreenPlaneHandle",
                "TranslateScreenPlane");
    GLTranslateScreenPlaneHandle* cHandle =
        cHandleWrapper->getPluginInstance<GLTranslateScreenPlaneHandle>();
    cHandle->setup();

    // YZ plane handle
    ManipulatorHandleWrapperPtr yzHandleWrapper =
        Manipulator::addManipulatorHandle("GLTranslatePlaneHandle",
                "TranslatePlaneYZ");
    GLTranslatePlaneHandle* yzHandle =
            yzHandleWrapper->getPluginInstance<GLTranslatePlaneHandle>();
    yzHandle->setup(Imath::V3d(1.0, 0.0, 0.0));

    // XZ plane handle
    ManipulatorHandleWrapperPtr xzHandleWrapper =
        Manipulator::addManipulatorHandle("GLTranslatePlaneHandle",
                "TranslatePlaneXZ");
    GLTranslatePlaneHandle* xzHandle =
        xzHandleWrapper->getPluginInstance<GLTranslatePlaneHandle>();
    xzHandle->setup(Imath::V3d(0.0, 1.0, 0.0));

    // XY plane handle
    ManipulatorHandleWrapperPtr xyHandleWrapper =
        Manipulator::addManipulatorHandle("GLTranslatePlaneHandle",
                "TranslatePlaneXY");
    GLTranslatePlaneHandle* xyHandle =
        xyHandleWrapper->getPluginInstance<GLTranslatePlaneHandle>();
    xyHandle->setup(Imath::V3d(0.0, 0.0, 1.0));
}
