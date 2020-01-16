add_library(SubdividedSpaceOp MODULE src/op.cpp)

target_link_libraries(SubdividedSpaceOp
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
)

set_target_properties(SubdividedSpaceOp PROPERTIES PREFIX "")

install(FILES SubdividedSpaceOp.py DESTINATION Plugins)
install(TARGETS SubdividedSpaceOp DESTINATION Ops)
