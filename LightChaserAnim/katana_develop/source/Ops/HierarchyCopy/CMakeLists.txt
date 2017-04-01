add_library(HierarchyCopy MODULE src/op.cpp)

target_link_libraries(HierarchyCopy
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::pystring
)

set_target_properties(HierarchyCopy PROPERTIES PREFIX "")
install(TARGETS HierarchyCopy DESTINATION Ops)
