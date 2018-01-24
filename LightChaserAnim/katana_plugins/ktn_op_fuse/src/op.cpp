#include <cstdlib>
#include <cmath>
#include <cassert>
#include <iostream>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolib/util/Path.h>
#include <FnPluginSystem/FnPlugin.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include <iostream>

namespace { //anonymous


const float FLOAT_RAND_MAX = static_cast<float>(RAND_MAX);


class FuseNormalSetOp : public Foundry::Katana::GeolibOp
{
public:

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // In order to run the Op we need a valid CEL statement
        FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
        FnAttribute::StringAttribute fuseAttr = interface.getOpArg("StringAttribute");
        if (!celAttr.isValid())
        {
            interface.stopChildTraversal();
            return;
        }

        // If a CEL attribute was provided (and so it's valid), check
        // our current output location against the CEL. If it doesn't match
        // the CEL, then don't continue cooking at this location.
        // Use the utility function matchesCEL() to populate a
        // MatchesCELInfo object that contains useful attributes we can
        // query
        FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
        FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,
                                                                 interface,
                                                                 celAttr);

        // If there's no chance that the CEL expression matches any child
        // locations, stop the Op from continuing its recursion past this
        // point in the hierarchy. This isn't required, but improves
        // efficiency.
        if (!info.canMatchChildren)
        {
            interface.stopChildTraversal();
        }

        // If the CEL doesn't match the current location, stop cooking
        if (!info.matches)
        {
            return;
        }
        std::string inputNname = interface.getInputName();
        FnAttribute::StringAttribute tagName = inputNname;
        interface.setAttr("geometry.arbitrary.fuse_tag.value", fuseAttr, false);
        interface.setAttr("geometry.arbitrary.fuse_tag.scope", FnAttribute::StringAttribute("primitive"), false);
    }

protected:

};

DEFINE_GEOLIBOP_PLUGIN(FuseNormalSetOp)

} // anonymous

void registerPlugins()
{
    std::cout << "[LCA PLUGIN] Register FuseNormalSet v1.0" << std::endl;
    REGISTER_PLUGIN(FuseNormalSetOp, "FuseNormalSet", 0, 1);
}
