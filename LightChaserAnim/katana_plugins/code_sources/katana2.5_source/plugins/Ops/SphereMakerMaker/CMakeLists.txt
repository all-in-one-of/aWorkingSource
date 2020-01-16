add_library(SphereMakerMaker MODULE src/op.cpp)

target_link_libraries(SphereMakerMaker
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
)

set_target_properties(SphereMakerMaker PROPERTIES PREFIX "")

install(FILES RegisterSphereMakerMakerNode.py DESTINATION Plugins)
install(TARGETS SphereMakerMaker DESTINATION Ops)
