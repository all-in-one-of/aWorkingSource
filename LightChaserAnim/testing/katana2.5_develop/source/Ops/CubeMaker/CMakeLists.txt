add_library(CubeMaker MODULE src/op.cpp)

target_link_libraries(CubeMaker
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
    Katana::pystring
)

set_target_properties(CubeMaker PROPERTIES PREFIX "")
install(TARGETS CubeMaker DESTINATION Ops)
