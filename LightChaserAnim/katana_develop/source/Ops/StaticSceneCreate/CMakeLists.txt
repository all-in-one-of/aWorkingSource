add_library(StaticSceneCreate MODULE src/op.cpp)

target_link_libraries(StaticSceneCreate
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
    Katana::pystring
)

set_target_properties(StaticSceneCreate PROPERTIES PREFIX "")
install(TARGETS StaticSceneCreate DESTINATION Ops)
