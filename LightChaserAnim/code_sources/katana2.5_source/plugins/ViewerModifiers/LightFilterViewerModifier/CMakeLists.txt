add_library(LightFilterViewerModifier MODULE src/LightFilterViewerModifier.cpp)

target_link_libraries(LightFilterViewerModifier
    PRIVATE
    Katana::FnViewerModifierPlugin
    Katana::FnPlatform
    Katana::FnAttribute
)

target_include_directories(LightFilterViewerModifier PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(LightFilterViewerModifier PRIVATE ${OPENGL_LIBRARIES})

set_target_properties(LightFilterViewerModifier PROPERTIES PREFIX "")
install(TARGETS LightFilterViewerModifier DESTINATION Libs)
