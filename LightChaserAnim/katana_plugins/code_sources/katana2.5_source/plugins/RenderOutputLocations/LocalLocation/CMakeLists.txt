add_library(LocalLocation MODULE src/LocalLocation.cpp)

target_include_directories(LocalLocation PRIVATE include)

target_link_libraries(LocalLocation
    PRIVATE
    Katana::FnAttribute
    Katana::FnRenderOutputLocationPlugin
    Katana::pystring
)

set_target_properties(LocalLocation PROPERTIES PREFIX "")
install(TARGETS LocalLocation DESTINATION Libs)
