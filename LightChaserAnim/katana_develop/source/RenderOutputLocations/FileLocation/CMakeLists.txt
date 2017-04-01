add_library(FileLocation MODULE src/FileLocation.cpp)

target_include_directories(FileLocation PRIVATE include)

target_link_libraries(FileLocation
    PRIVATE
    Katana::FnAsset
    Katana::FnAttribute
    Katana::FnDefaultAttributeProducerPlugin
    Katana::FnRenderOutputLocationPlugin
    Katana::pystring
)

set_target_properties(FileLocation PROPERTIES PREFIX "")
install(TARGETS FileLocation DESTINATION Libs)
