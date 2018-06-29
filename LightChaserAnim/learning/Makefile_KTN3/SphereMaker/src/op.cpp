#include <sstream>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolib/util/Path.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

namespace { //anonymous

/**
 * SphereMakerOp
 *
 * The SphereMaker Op implements a 'scene graph generator'-like Op, creating
 * a number of spheres on a defined location.
 *
 * The Op set-up is based on two main parameters:
 *
 * - the base parent location for all the spheres
 * - the number of spheres to generate
 *
 * The Op expects the following conventions for its arguments:
 *
 * - The base location is encoded using nested group attributes defining a
 *   hierarchy where the elements in the location paths are interleaved with
 *   group attributes named 'c' (for child).
 *
 *   For example the location '/root/world/geo/sphereMaker' will be encoded as:
 *   'c.world.c.geo.c.sphereMaker' (notice that root has been omitted as the
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
 *   integer attribute defining the number of spheres to be generated.
 *
 *   For each sphere the Op will then create a child location and it will set
 *   on them a group attribute, named 'leaf', containing a sphere Id.
 *   When processed, leaf locations will be populated with the 'geometry' and
 *   'xform' group attributes representing the sphere shape and transform.
 *
 */

class SphereMakerOp : public Foundry::Katana::GeolibOp
{
public:

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
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
            FnAttribute::IntAttribute numberOfSpheresAttr =
                aGrpAttr.getChildByName("numberOfSpheres");

            const int numberOfSpheres = numberOfSpheresAttr.getValue(0, false);
            if (numberOfSpheres > 0)
            {
                for (int i = 0; i < numberOfSpheres; ++i)
                {
                    // Build the location name
                    std::ostringstream ss;
                    ss << "sphere_" << i;

                    // Set up and create a leaf location that will be turned
                    FnAttribute::GroupBuilder childArgsBuilder;
                    childArgsBuilder.set("leaf.index", FnAttribute::IntAttribute(i));
                    interface.createChild(ss.str(), "", childArgsBuilder.build());
                }
            }

            // Ignore other arguments as we've already found the 'a' group
           return;
        }

        // Look for a 'leaf' Op argument
        FnAttribute::GroupAttribute leaf = interface.getOpArg("leaf");
        if (leaf.isValid())
        {
            // If leafAttr is valid we'll populate the leaf location with the
            // sphere geometry
            FnAttribute::IntAttribute indexAttr = leaf.getChildByName("index");
            const int index = indexAttr.getValue(0 , false);
            interface.setAttr("geometry", buildGeometry(index));
            interface.setAttr("xform", buildTransform(index));
            interface.setAttr("type", FnAttribute::StringAttribute("sphere"));

            interface.stopChildTraversal();
        }
    }

protected:

    /**
     * Builds and returns a group attribute representing the sphere geometry
     */
    static FnAttribute::Attribute buildGeometry(int index)
    {
        FnAttribute::GroupBuilder gb;
        gb.set("radius", FnKat::DoubleAttribute(1.0));
        gb.set("id", FnKat::IntAttribute(index));

        return gb.build();
    }

    /**
     * Builds and returns a group attribute representing the transform of the
     * i-th sphere
     */
    static FnAttribute::Attribute buildTransform(int index)
    {
        FnKat::GroupBuilder gb;

        // In order to calculate the sphere's translation we have to take into
        // account the increasing scale of the preceding spheres
        const double translate[] = { 0.25 * (index + 2.0) * index, 0.0, 0.0 };
        gb.set("translate", FnKat::DoubleAttribute(translate, 3, 3));

        const double scale = (index + 1.0) * 0.5;
        const double scaleValues[] = { scale, scale, scale };
        gb.set("scale", FnKat::DoubleAttribute(scaleValues, 3, 3));

        gb.setGroupInherit(false);
        return gb.build();
    }
};

DEFINE_GEOLIBOP_PLUGIN(SphereMakerOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(SphereMakerOp, "SphereMaker", 0, 1);
}
