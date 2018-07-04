cmake_minimum_required (VERSION 3.2)

add_library(SSLighting MODULE
    src/Main.cpp
    src/ExampleViewport.cpp
    src/ExampleViewerDelegate.cpp
    src/DrawableMesh.cpp
    src/SceneNode.cpp
    src/layers/ExampleSceneLayer.cpp
    src/components/IgnoreComponent.cpp
    src/components/LoggingComponent.cpp

    ${KATANA_HOME}/plugin_apis/src/FnViewer/utils/FnGLShaderProgram.cpp
)

add_definitions(-DFNVIEWER_STATIC=1 -D_USE_MATH_DEFINES -DFNLOGGING_STATIC)

target_include_directories(SSLighting
    PRIVATE
        include
        ${OPENGL_INCLUDE_DIR}
        ${KATANA_HOME}/plugin_apis/include
)

target_link_libraries(SSLighting
    PRIVATE
        Katana::FnAttribute
        Katana::pystring
        Katana::FnLogging
        Katana::FnViewerAPI
        ${OPENGL_LIBRARIES}
        OpenEXR::OpenEXR
        GLEW::GLEW)

set_target_properties(SSLighting PROPERTIES PREFIX "")
install(TARGETS SSLighting DESTINATION Libs)
