add_library(RigidBind MODULE
    src/RigidBindOp.cpp)

set_target_properties(RigidBind PROPERTIES PREFIX "")

target_link_libraries(RigidBind
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    OpenEXR::OpenEXR)

install(TARGETS RigidBind DESTINATION Ops)
