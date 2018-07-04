add_library(SphereRender MODULE src/SphereRender.cpp)

target_include_directories(SphereRender PRIVATE include)

target_link_libraries(SphereRender
    PRIVATE
    Katana::FnAttribute
    Katana::FnPlatform
    Katana::FnViewerModifierPlugin
)

target_include_directories(SphereRender PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(SphereRender PRIVATE ${OPENGL_LIBRARIES})

set_target_properties(SphereRender PROPERTIES PREFIX "")
install(TARGETS SphereRender DESTINATION Libs)
