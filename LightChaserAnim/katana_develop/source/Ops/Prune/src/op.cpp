#include <iostream>
#include <string>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/op/FnGeolibOp.h>

#include <FnGeolib/util/Path.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

namespace { //anonymous

class PruneOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        FnAttribute::StringAttribute pathsAttr = interface.getOpArg("paths");

        bool canMatchChildren = false;
        std::string fullName = interface.getOutputLocationPath();

        if (pathsAttr.isValid())
        {
            FnAttribute::StringConstVector paths = pathsAttr.getNearestSample(0.0f);

            for (FnAttribute::StringConstVector::const_iterator iter=paths.begin();
                 iter!=paths.end(); ++iter)
            {
                FnGeolibUtil::Path::FnMatchInfo fnMatchInfo;
                FnGeolibUtil::Path::FnMatch(fnMatchInfo, fullName, *iter);
                if (fnMatchInfo.match)
                {
                    interface.deleteSelf();
                    return;
                }
                if (fnMatchInfo.canMatchChildren)
                {
                    canMatchChildren = true;
                }
            }
        }

        FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
        if (celAttr.isValid())
        {
            FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
            FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(
                    info, interface, celAttr);
            if (info.matches)
            {
                interface.deleteSelf();
                return;
            }

            if (info.canMatchChildren)
            {
                canMatchChildren = true;
            }
        }

        if (!canMatchChildren) interface.stopChildTraversal();
    }
};
DEFINE_GEOLIBOP_PLUGIN(PruneOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(PruneOp, "Prune", 0, 1);
}

