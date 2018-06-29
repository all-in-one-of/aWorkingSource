add_library(LightShapeModifier MODULE src/LightViewerModifier.cpp)

target_include_directories(LightShapeModifier PRIVATE include)

target_link_libraries(LightShapeModifier
    PRIVATE
    Katana::FnViewerModifierPlugin
    Katana::FnAttribute
    Katana::FnPlatform
)

target_include_directories(LightShapeModifier PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(LightShapeModifier PRIVATE ${OPENGL_LIBRARIES})

set_target_properties(LightShapeModifier PROPERTIES PREFIX "")
install(TARGETS LightShapeModifier DESTINATION Libs)
