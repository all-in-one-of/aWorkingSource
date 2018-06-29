add_library(AttributeCopy MODULE src/op.cpp)

target_include_directories(AttributeCopy PRIVATE include)

target_link_libraries(AttributeCopy
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
    Katana::pystring
)

set_target_properties(AttributeCopy PROPERTIES PREFIX "")
install(TARGETS AttributeCopy DESTINATION Ops)
