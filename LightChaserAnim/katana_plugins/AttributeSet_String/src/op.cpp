#include <iostream>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/op/FnGeolibOp.h>

#include <FnGeolib/util/Path.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>
#include <FnGeolibServices/FnMaterialResolveUtil.h>

namespace { //anonymous

// This creates scenegraph from the attr representation used by
// RuntimeUtils.GetScenegraphAsAttr

class AttributeSetStringOp : public Foundry::Katana::GeolibOp
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
        FnAttribute::StringAttribute nameAttr = interface.getOpArg("stringAttributeName");
        FnAttribute::StringAttribute valueAttr = interface.getOpArg("stringAttributeValue");
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
        FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,interface,celAttr);

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
        //std::string inputNname = interface.getInputName();
        //FnAttribute::StringAttribute tagName = inputNname;
        interface.setAttr(nameAttr.getValue(), valueAttr, false);
    }
};

DEFINE_GEOLIBOP_PLUGIN(AttributeSetStringOp)

///////////////////////////////////////////////////////////////////////////////

bool matchesLocation(Foundry::Katana::GeolibCookInterface &interface,
        bool & outCanMatchChildren)
{
    bool localMatch = false;
    bool canMatchChildren = false;

    FnAttribute::StringAttribute locationsAttr =
            interface.getOpArg("locationPaths");

    FnAttribute::StringAttribute celAttr =
            interface.getOpArg("CEL");

    if (!locationsAttr.isValid() && !celAttr.isValid())
    {
        outCanMatchChildren = false;
        return false;
    }

    if (locationsAttr.isValid())
    {
        FnGeolibUtil::Path::FnMatchInfo matchInfo;
        FnAttribute::StringConstVector locationPaths =
            locationsAttr.getNearestSample(0.0);
        for(unsigned int i=0; i<locationPaths.size(); ++i)
        {
            FnGeolibUtil::Path::FnMatch(matchInfo,
                interface.getOutputLocationPath(), locationPaths[i]);
            if(matchInfo.match) localMatch = true;
            if(matchInfo.canMatchChildren) canMatchChildren = true;
            if(localMatch && canMatchChildren) break;
        }
    }
    else
    {
        FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo matchInfo;
        FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(
                matchInfo, interface, celAttr);

        if(matchInfo.matches) localMatch = true;
        if(matchInfo.canMatchChildren) canMatchChildren = true;
    }

    outCanMatchChildren = canMatchChildren;
    return localMatch;
}

///////////////////////////////////////////////////////////////////////////////


} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(AttributeSetStringOp, "AttributeSetString", 0, 1);
}

