add_library(TemplateRendererInfoPlugin MODULE src/TemplateRendererInfoPlugin)

target_include_directories(TemplateRendererInfoPlugin PRIVATE include)

target_link_libraries(TemplateRendererInfoPlugin
    PRIVATE
    Katana::FnRendererInfoPlugin
)

set_target_properties(TemplateRendererInfoPlugin PROPERTIES PREFIX "")
install(TARGETS TemplateRendererInfoPlugin DESTINATION Libs)
