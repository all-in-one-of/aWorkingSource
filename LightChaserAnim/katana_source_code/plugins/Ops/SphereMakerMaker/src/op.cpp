#include <sstream>

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnGeolib/util/Path.h>
#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

namespace { //anonymous

/**
 * SphereMakerMakerOp
 *
 * The SphereMakerMaker Op implements a 'scene graph generator'-like Op, creating
 * a number locations each running the SphereMaker Op.
 *
 * The Op set-up is based on three main parameters:
 *
 * - the number of SphereMakers to create
 * - the base location for all the SphereMakers
 * - the number of spheres each SphereMaker should generate
 *
 * The Op expects the following conventions for its arguments:
 *
 * - The base location is encoded using nested group attributes defining a
 *   hierarchy where the elements in the location paths are interleaved with
 *   group attributes named 'c' (for child).
 *
 *   For example the location '/root/world/geo/sphereMakerMaker' will be encoded as:
 *   'c.world.c.geo.c.sphereMakerMaker' (notice that root has been omitted as the
 *   root location always exists in the scene graph).
 *
 *   The Op will walk the attributes hierarchy building a child location for
 *   each level.
 *   Note: the reason to interleave the 'c' attributes is to allow the Op code
 *   to be extended in the future without changing its arguments convention.
 *   The 'c' group would allow further parameters to be specified for each
 *   level in the hierarchy.
 *
 * - The group attribute representing the last item in the base location path
 *   will contain a group attribute, named 'a', which in turn will hold a
 *   integer attribute defining the number of sphereMakers to be generated.
 *
 * For each sphereMaker the Op will then create a child location and it will
 * set on them a group attribute to specify the location where the spheres will be
 * created.
 */

class SphereMakerMakerOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        if (interface.atRoot())
        {
            interface.stopChildTraversal();
        }

        // Look for a 'c' Op argument, representing an element in the
        // hierarchy leading to the base scene graph location that will
        // contain the spheres
        FnAttribute::GroupAttribute cGrpAttr = interface.getOpArg("c");
        if (cGrpAttr.isValid())
        {
            const int64_t numChildren = cGrpAttr.getNumberOfChildren();
            if (numChildren != 1)
            {
                // We expected exactly one child attribute in 'c', if it's not
                // the case we notify the user with an error
                Foundry::Katana::ReportError(interface,
                    "Unsupported attributes convention.");
                interface.stopChildTraversal();
                return;
            }

            const std::string childName =
                FnAttribute::DelimiterDecode(cGrpAttr.getChildName(0));
            FnAttribute::GroupAttribute childArgs = cGrpAttr.getChildByIndex(0);
            // Create a child location using the attribute name and forwarding
            // the hierarchy information
            interface.createChild(childName, "", childArgs);

            // Ignore other arguments as we've already found the 'c' group
            return;
        }

        // Look for an 'a' Op argument that will contain the attributes needed
        // to generated the spheres
        FnAttribute::GroupAttribute aGrpAttr = interface.getOpArg("a");
        if (aGrpAttr.isValid())
        {
            // Get the number of sphereMakers
            FnAttribute::IntAttribute numberOfSphereMakersAttr =
                aGrpAttr.getChildByName("numberOfSphereMakers");
            int32_t numberOfSphereMakers = numberOfSphereMakersAttr.getValue(0, false);

            // Get the number of sphere
            FnAttribute::IntAttribute numberOfSpheresAttr =
                aGrpAttr.getChildByName("numberOfSpheres");
            int32_t numberOfSpheres = numberOfSpheresAttr.getValue(0, false);

            if (numberOfSphereMakers > 0 && numberOfSpheres >=0)
            {
                for (int32_t i = 0; i < numberOfSphereMakers; ++i)
                {
                    // Build the location name
                    std::ostringstream ss;
                    ss << "sphereMaker_" << i;

                    // create the groupattribute to pass to the SphereMaker Op
                    FnAttribute::GroupBuilder childArgs;
                    childArgs.update(interface.getOpArg());
                    childArgs.set(
                            "a.numberOfSpheres",
                            FnAttribute::IntAttribute(numberOfSpheres));
                    interface.createChild(ss.str(), "SphereMaker", childArgs.build());
                }
            }

            // Ignore other arguments as we've already found the 'a' group
           return;
        }
    }
};

DEFINE_GEOLIBOP_PLUGIN(SphereMakerMakerOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(SphereMakerMakerOp, "SphereMakerMaker", 0, 1);
}

