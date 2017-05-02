add_library(AttributeSet MODULE src/op.cpp)

target_link_libraries(AttributeSet
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
    Katana::FnGeolibServices
    Katana::pystring
)

set_target_properties(AttributeSet PROPERTIES PREFIX "")
install(TARGETS AttributeSet DESTINATION Ops)
