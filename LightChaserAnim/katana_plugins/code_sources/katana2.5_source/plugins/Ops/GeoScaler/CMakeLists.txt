add_library(GeoScaler MODULE src/op.cpp)

target_link_libraries(GeoScaler
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
)

set_target_properties(GeoScaler PROPERTIES PREFIX "")

install(FILES RegisterGeoScalerNode.py DESTINATION Plugins)
install(TARGETS GeoScaler DESTINATION Ops)
