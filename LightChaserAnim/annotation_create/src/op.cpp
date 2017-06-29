#include <cstdlib>
#include <cmath>
#include <cassert>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolib/util/Path.h>
#include <FnPluginSystem/FnPlugin.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include <iostream>

namespace { //anonymous

const float DEFAULT_DISPLACEMENT = 2.0f;
const float FLOAT_RAND_MAX = static_cast<float>(RAND_MAX);

/**
 * MesserOp
 *
 * The Messer Op modifies the geometry attributes on the given location,
 * displacing the current point positions by a weighted random amount.
 *
 * The Op expects two arguments to be set:
 *
 * - CEL: The CEL expression defining the location(s) for which to modify
 *        the geometry attributes
 *
 *- displacement: the maximum displacement applied to point positions
 */

class TagSetOp : public Foundry::Katana::GeolibOp
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
        FnAttribute::IntAttribute pickableAttr = interface.getOpArg("pickable");
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
        interface.setAttr("viewer.default.annotation.text", tagName, false);
        interface.setAttr("viewer.default.pickable", pickableAttr, false);

/*        std::vector<std::string> deepLocations;
        {
            FnAttribute::StringAttribute pcAttr = 
                interface.getPotentialChildren();
            FnAttribute::StringAttribute::array_type pc = 
                pcAttr.getNearestSample(0.f);
            for(unsigned int i=0; i<pc.size(); ++i)
            {
                interface.prefetch(pc[i]);
                deepLocations.push_back(pc[i]);
            }
        }
        for(unsigned int j=0; j<deepLocations.size(); ++j)
        {
            {
                std::string newLocation = inputLocationPath + "/" +deepLocations[j];
                //std::cout << j << ":" << newLocation << std::endl;
                interface.prefetch(newLocation);
                interface.resetRoot();
                std::cout << interface.getOutputLocationPath() << std::endl;
                FnAttribute::StringAttribute tagName = deepLocations[j];
                interface.setAttr("viewer.default.annotation.text", tagName, false);
                //interface.resetRoot();            
            }
        }*/

        //std::string outputName = interface.getRelativeOutputLocationPath();
        //std::cout << "outputName:" << outputName << std::endl;

    }

protected:

};

DEFINE_GEOLIBOP_PLUGIN(TagSetOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(TagSetOp, "TagSet", 0, 1);
}
