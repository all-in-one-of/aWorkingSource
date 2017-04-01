add_library(Messer MODULE src/op.cpp)

target_link_libraries(Messer
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
    Katana::pystring
)

set_target_properties(Messer PROPERTIES PREFIX "")

install(FILES RegisterMesserNode.py DESTINATION Plugins)
install(TARGETS Messer DESTINATION Ops)
