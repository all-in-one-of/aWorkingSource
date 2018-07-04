// Copyright (c) 2016 The Foundry Visionmongers, Ltd.
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnAttribute/FnAttribute.h>

namespace
{
// "Hello World"-style op that sets a string attribute at the root location.
class HelloWorldOp : public Foundry::Katana::GeolibOp
{
public:
    // Boilerplate that indicates the Op's cook() function is safe to be called
    // concurrently.
    static void setup(Foundry::Katana::GeolibSetupInterface& interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface& interface)
    {
        if (interface.atRoot())
        {
            interface.setAttr("hello", FnAttribute::StringAttribute("world!"));
        }
        interface.stopChildTraversal();
    }
};
DEFINE_GEOLIBOP_PLUGIN(HelloWorldOp)
}  // namespace

void registerPlugins()
{
    REGISTER_PLUGIN(HelloWorldOp, "HelloWorld", 0, 1);
}
