#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnGeolib/util/Path.h>
#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

namespace { //anonymous

class GeoScalerOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // CEL support
        FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
        if (celAttr.isValid())
        {
            FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
            FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,
                                                                     interface,
                                                                     celAttr);
            if (!info.canMatchChildren)
            {
                interface.stopChildTraversal();
            }
            if (!info.matches)
            {
                return;
            }
        } else
        {
            interface.stopChildTraversal();
            return;
        }

        // Get the Geometry FloatAttributes
        FnAttribute::FloatAttribute inputFltAttr =
                interface.getAttr("geometry.point.P");

        // Get scale FloatAttribute
        FnAttribute::FloatAttribute scaleFloatAttr = interface.getOpArg("scale");

        if (inputFltAttr.isValid() && scaleFloatAttr.isValid())
        {
            FnAttribute::FloatConstVector inputPoints =
                    inputFltAttr.getNearestSample(0.0f);

            const float scale = scaleFloatAttr.getValue();

            const size_t numberOfPoints = inputPoints.size();
            float *newPoints = new float[numberOfPoints];
            for (size_t i = 0; i < numberOfPoints; ++i)
            {
                newPoints[i] = inputPoints[i] * scale;
            }
            FnAttribute::FloatAttribute newPointsFltAttr =
                    FnAttribute::FloatAttribute(newPoints, numberOfPoints, 3);
            delete[] newPoints;
            interface.setAttr("geometry.point.P", newPointsFltAttr);
        }
    }
};

DEFINE_GEOLIBOP_PLUGIN(GeoScalerOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(GeoScalerOp, "GeoScaler", 0, 1);
}

