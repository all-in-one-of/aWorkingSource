add_library(SkinEnvelope MODULE
    src/SkinEnvelopeOp.cpp)

set_target_properties(SkinEnvelope PROPERTIES PREFIX "")

target_link_libraries(SkinEnvelope
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::pystring
    OpenEXR::OpenEXR)

install(TARGETS SkinEnvelope DESTINATION Ops)
