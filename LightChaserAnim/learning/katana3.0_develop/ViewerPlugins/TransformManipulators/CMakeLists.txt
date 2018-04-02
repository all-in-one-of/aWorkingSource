cmake_minimum_required (VERSION 3.2)

add_library(TransformManipulators MODULE
    src/Main.cpp
    src/FnGLTransformManipulator.cpp
    src/FnMeshGeneration.cpp
    src/coi/FnGLCoiManipulator.cpp
    src/coi/FnGLCoiHandle.cpp
    src/translate/FnGLTranslateManipulator.cpp
    src/translate/FnGLTranslateAxisHandle.cpp
    src/translate/FnGLTranslatePlaneHandle.cpp
    src/translate/FnGLTranslateScreenPlaneHandle.cpp
    src/rotate/FnGLRotateManipulator.cpp
    src/rotate/FnGLRotateAxisHandle.cpp
    src/rotate/FnGLRotateBallHandle.cpp
    src/scale/FnGLScaleManipulator.cpp
    src/scale/FnGLScaleAxisHandle.cpp
    src/scale/FnGLScalePlaneHandle.cpp
    src/scale/FnGLScaleUniformHandle.cpp

    ${KATANA_HOME}/plugin_apis/src/FnViewer/utils/FnGLManipulator.cpp
    ${KATANA_HOME}/plugin_apis/src/FnViewer/utils/FnGLShaderProgram.cpp
)

add_definitions(-DFNVIEWER_STATIC=1 -D_USE_MATH_DEFINES -DNOMINMAX -DFNLOGGING_STATIC)

target_include_directories(TransformManipulators
    PRIVATE
        include
        ${OPENGL_INCLUDE_DIR}
        ${KATANA_HOME}/plugin_apis/include
)

target_link_libraries(TransformManipulators
    PRIVATE
        Katana::FnAttribute
        Katana::FnViewerAPI
        Katana::pystring
        ${OPENGL_LIBRARIES}
        OpenEXR::OpenEXR
        GLEW::GLEW)

set_target_properties(TransformManipulators PROPERTIES PREFIX "")
install(TARGETS TransformManipulators DESTINATION Libs)
