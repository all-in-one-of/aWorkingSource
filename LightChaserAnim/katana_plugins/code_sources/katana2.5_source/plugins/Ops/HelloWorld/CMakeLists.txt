add_library(HelloWorldOp MODULE src/HelloWorldOp.cpp)

target_link_libraries(HelloWorldOp
    PRIVATE
    Katana::FnAttribute
    Katana::FnGeolibOpPlugin
)

set_target_properties(HelloWorldOp PROPERTIES PREFIX "")

install(TARGETS HelloWorldOp DESTINATION Ops)
