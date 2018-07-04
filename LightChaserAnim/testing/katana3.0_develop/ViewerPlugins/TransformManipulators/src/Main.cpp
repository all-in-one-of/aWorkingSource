// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
    #include <FnPlatform/Windows.h>
#endif
#include <FnViewer/plugin/FnViewerDelegate.h>
#include <FnViewer/plugin/FnViewportLayer.h>
#include <FnViewer/plugin/FnViewport.h>
#include <FnViewer/plugin/FnManipulator.h>
#include <FnViewer/plugin/FnManipulatorHandle.h>

#include "coi/FnGLCoiManipulator.h"
#include "coi/FnGLCoiHandle.h"
#include "translate/FnGLTranslateManipulator.h"
#include "translate/FnGLTranslateAxisHandle.h"
#include "translate/FnGLTranslatePlaneHandle.h"
#include "translate/FnGLTranslateScreenPlaneHandle.h"
#include "rotate/FnGLRotateManipulator.h"
#include "rotate/FnGLRotateAxisHandle.h"
#include "rotate/FnGLRotateBallHandle.h"
#include "scale/FnGLScaleManipulator.h"
#include "scale/FnGLScaleAxisHandle.h"
#include "scale/FnGLScalePlaneHandle.h"
#include "scale/FnGLScaleUniformHandle.h"


namespace // anonymous
{
    // Manipulators and ManipulatorHandles
    DEFINE_MANIPULATOR_PLUGIN(GLTranslateManipulator)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLTranslateAxisHandle)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLTranslatePlaneHandle)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLTranslateScreenPlaneHandle)

    DEFINE_MANIPULATOR_PLUGIN(GLRotateManipulator)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLRotateAxisHandle)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLRotateBallHandle)

    DEFINE_MANIPULATOR_PLUGIN(GLScaleManipulator)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLScaleAxisHandle)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLScalePlaneHandle)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLScaleUniformHandle)

    DEFINE_MANIPULATOR_PLUGIN(GLCoiManipulator)
    DEFINE_MANIPULATOR_HANDLE_PLUGIN(GLCoiHandle)


} // namespace anonymous

void registerPlugins()
{
    // Manipulators and ManipulatorHandles
    REGISTER_PLUGIN(GLTranslateManipulator, "GLTranslateManipulator", 0, 1);
    REGISTER_PLUGIN(GLTranslateAxisHandle, "GLTranslateAxisHandle", 0, 1);
    REGISTER_PLUGIN(GLTranslatePlaneHandle, "GLTranslatePlaneHandle", 0, 1);
    REGISTER_PLUGIN(GLTranslateScreenPlaneHandle, "GLTranslateScreenPlaneHandle", 0, 1);

    REGISTER_PLUGIN(GLRotateManipulator, "GLRotateManipulator", 0, 1);
    REGISTER_PLUGIN(GLRotateAxisHandle, "GLRotateAxisHandle", 0, 1);
    REGISTER_PLUGIN(GLRotateBallHandle, "GLRotateBallHandle", 0, 1);

    REGISTER_PLUGIN(GLScaleManipulator, "GLScaleManipulator", 0, 1);
    REGISTER_PLUGIN(GLScaleAxisHandle, "GLScaleAxisHandle", 0, 1);
    REGISTER_PLUGIN(GLScalePlaneHandle, "GLScalePlaneHandle", 0, 1);
    REGISTER_PLUGIN(GLScaleUniformHandle, "GLScaleUniformHandle", 0, 1);

    REGISTER_PLUGIN(GLCoiManipulator, "GLCoiManipulator", 0, 1);
    REGISTER_PLUGIN(GLCoiHandle, "GLCoiHandle", 0, 1);
}

