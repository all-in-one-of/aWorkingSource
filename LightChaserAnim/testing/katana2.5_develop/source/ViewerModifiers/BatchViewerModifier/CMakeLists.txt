add_library(BatchViewerModifier MODULE src/BatchViewerModifier.cpp)

target_link_libraries(BatchViewerModifier
    PRIVATE
    Katana::FnAttribute
    Katana::FnViewerModifierPlugin
)

target_include_directories(BatchViewerModifier PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(BatchViewerModifier PRIVATE ${OPENGL_LIBRARIES})

target_link_libraries(BatchViewerModifier PRIVATE GLEW::GLEW)
target_link_libraries(BatchViewerModifier PRIVATE OpenEXR::OpenEXR)

set_target_properties(BatchViewerModifier PROPERTIES PREFIX "")
install(TARGETS BatchViewerModifier DESTINATION Libs)
