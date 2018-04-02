cmake_minimum_required (VERSION 3.2)

set(PYTHON_FILES python/BallViewerPluginExtension.py)

add_library(BallLocationType MODULE
    src/BallLocationType.cpp
    include/BallLocationType.h
    ${PYTHON_FILES}

    ${KATANA_HOME}/plugin_apis/src/FnViewer/utils/FnGLShaderProgram.cpp
    ${KATANA_HOME}/plugin_apis/include/FnViewer/utils/FnGLShaderProgram.h
)

add_definitions(-DFNVIEWER_STATIC=1 -D_USE_MATH_DEFINES -DNOMINMAX -DFNLOGGING_STATIC)

target_include_directories(BallLocationType
    PRIVATE
        include
        ${OPENGL_INCLUDE_DIR}
        ${KATANA_HOME}/plugin_apis/include
)

target_link_libraries(BallLocationType
    PRIVATE
        Katana::FnAttribute
        Katana::FnViewerAPI
        ${OPENGL_LIBRARIES}
        GLEW::GLEW
        OpenEXR::OpenEXR)

set_target_properties(BallLocationType PROPERTIES PREFIX "")

install(TARGETS BallLocationType DESTINATION Libs)
install(FILES ${PYTHON_FILES} DESTINATION Plugins)
