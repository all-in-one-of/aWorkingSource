add_library(SphereMaker MODULE src/op.cpp)

target_link_libraries(SphereMaker
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
)

set_target_properties(SphereMaker PROPERTIES PREFIX "")

install(FILES RegisterSphereMakerNode.py DESTINATION Plugins)
install(TARGETS SphereMaker DESTINATION Ops)
