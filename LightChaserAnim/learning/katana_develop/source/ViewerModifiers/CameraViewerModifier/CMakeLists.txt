add_library(CameraViewerModifier MODULE src/CameraViewerModifier.cpp)

target_include_directories(CameraViewerModifier PRIVATE include)

target_link_libraries(CameraViewerModifier
    PRIVATE
    Katana::FnViewerModifierPlugin
    Katana::FnAttribute
    Katana::FnPlatform
)

target_include_directories(CameraViewerModifier PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(CameraViewerModifier PRIVATE ${OPENGL_LIBRARIES})

set_target_properties(CameraViewerModifier PROPERTIES PREFIX "")
install(TARGETS CameraViewerModifier DESTINATION Libs)
