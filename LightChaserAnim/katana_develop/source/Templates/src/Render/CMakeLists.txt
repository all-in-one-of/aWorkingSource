add_library(TemplateRenderPlugin MODULE src/TemplateRenderPlugin)

target_include_directories(TemplateRenderPlugin PRIVATE include)

target_link_libraries(TemplateRenderPlugin
    PRIVATE
    Katana::FnRenderOutputUtils
    Katana::FnRenderPlugin
    Katana::FnScenegraphIterator
)

set_target_properties(TemplateRenderPlugin PROPERTIES PREFIX "")
install(TARGETS TemplateRenderPlugin DESTINATION Libs)
