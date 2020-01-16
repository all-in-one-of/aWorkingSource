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

class MesserOp : public Foundry::Katana::GeolibOp
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

        FnAttribute::FloatAttribute displacementAttr =
            interface.getOpArg("displacement");

        const float displacement =
            displacementAttr.getValue(DEFAULT_DISPLACEMENT, false);

        FnAttribute::FloatAttribute currPointsAttr =
            interface.getAttr("geometry.point.P");

        if (currPointsAttr.isValid())
        {
            // Initialize the pseudo-random number generator
            srand(2501);

            const int64_t tupleSize = currPointsAttr.getTupleSize();
            // Only vectors with tuple size = 3 are supported
            if (tupleSize > 4)
            {
                // If the tuple size is not supported we stop the Ops execution
                // and notify the user
                Foundry::Katana::ReportError(interface,
                    "Unsupported tuple size for 'geometry.point.P'.");
                interface.stopChildTraversal();
                return;
            }

            FnAttribute::FloatConstVector currPointsVec =
                currPointsAttr.getNearestSample(0.0f);
            const int64_t numValues = currPointsVec.size();
            const float * currPoints = currPointsVec.data();

            float * newPoints = new float[numValues];
            for (int64_t i = 0; i < numValues; i += tupleSize)
            {
                makeNewPoint(&newPoints[i], &currPoints[i], tupleSize,
                             displacement);
            }

            // Create and set the new point positions attribute
            FnAttribute::FloatAttribute newPointsAttr(newPoints, numValues,
                                                      tupleSize);
            interface.setAttr("geometry.point.P", newPointsAttr, false);

            // Clean-up
            delete [] newPoints;
        }
    }

protected:

    static void makeNewPoint(float* dest, const float* src,
                             int64_t tupleSize, float displacement)
    {
        // Max supported tuple size is 4
        assert(tupleSize >= 0 && tupleSize <= 4);
        float randVec[4];
        float squareLen = 0.0f;
        for (int64_t i = 0; i < tupleSize; ++i)
        {
            randVec[i] = static_cast<float>(rand()) / FLOAT_RAND_MAX;
            squareLen += randVec[i] * randVec[i];
        }

        const float length = std::sqrt(squareLen);

        for (int64_t i = 0; i < tupleSize; ++i)
        {
            const float delta = randVec[i] * length * displacement;
            dest[i] = src[i] + delta;
        }
    }
};

DEFINE_GEOLIBOP_PLUGIN(MesserOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(MesserOp, "Messer", 0, 1);
}
