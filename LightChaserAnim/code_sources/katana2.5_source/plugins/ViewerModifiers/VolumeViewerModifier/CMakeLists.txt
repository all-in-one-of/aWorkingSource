add_library(VolumeViewerModifier MODULE src/VolumeViewerModifier.cpp)

target_include_directories(VolumeViewerModifier PRIVATE include)

target_link_libraries(VolumeViewerModifier
    PRIVATE
    Katana::FnAttribute
    Katana::FnPlatform
    Katana::FnViewerModifierPlugin
)

target_include_directories(VolumeViewerModifier PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(VolumeViewerModifier PRIVATE ${OPENGL_LIBRARIES})

target_link_libraries(VolumeViewerModifier PRIVATE OpenEXR::OpenEXR)

set_target_properties(VolumeViewerModifier PROPERTIES PREFIX "")
install(TARGETS VolumeViewerModifier DESTINATION Libs)
