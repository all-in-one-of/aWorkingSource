add_library(Prune MODULE src/op.cpp)

target_link_libraries(Prune
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
    Katana::pystring
)

set_target_properties(Prune PROPERTIES PREFIX "")
install(TARGETS Prune DESTINATION Ops)
