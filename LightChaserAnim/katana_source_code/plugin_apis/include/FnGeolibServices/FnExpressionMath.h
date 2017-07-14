#ifndef FnGeolibServicesExpressionMath_H
#define FnGeolibServicesExpressionMath_H

#include <string>
#include <inttypes.h>
#include <limits>
#include <utility>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnGeolibServices/suite/FnExpressionMathSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FnExpressionMath
    {
    public:
        // Clamp the value between a and b.
        // This function works for a>b and a<b.
        static int    clamp(int value, int a, int b);
        static float  clamp(float value, float a, float b);
        static double clamp(double value, double a, double b);

        // Lerp between the value between a and b, using the specified mix
        // Mix values outside of [0,1] are not clamped.
        // Mix value of zero returns 'a'.
        // This argument order is such to make it similar to fit and cfit
        static float  lerp(float mix, float a, float b);
        static double lerp(double mix, double a, double b);

        // Compute a smoothstep (ease in, ease out) version of t: [0,1]
        // This will clamp the output to between 0 and 1
        static float  smoothstep(float t);
        static double smoothstep(double t);

        // Returns a number between newmin and newmax,
        //  which is relative to val in the range between
        // oldmin and oldmax.
        //
        // example: fit(0.8, 0, 1, 0, 255) = 204.
        //          fit(3,1,4,5,20)=15
        // Implemented for float, double.
        static float  fit(float value, float oldmin, float oldmax,
                float newmin, float newmax);
        static double fit(double value, double oldmin, double oldmax,
                double newmin, double newmax);

        // Same as fit, but clamps to new borders
        // Works on both increasing / decreasing segments
        static float  cfit(float value, float oldmin, float oldmax,
                float newmin, float newmax);
        static double cfit(double value, double oldmin, double oldmax,
                double newmin, double newmax);

        // Just like regular cfit(), only softer!
        // More specifically, uses SmoothStep to ease in and out of the fit
        static float  softcfit(float value, float oldmin, float oldmax,
                float newmin, float newmax);
        static double softcfit(double value, double oldmin, double oldmax,
                double newmin, double newmax);

        enum RetimeHoldMode
        {
            //! Hold the first/last frame of the sequence. 1111 1234 4444
            RETIME_FREEZE = kFnKatExpressionMathRetimeFreeze,

            //! Repeat the sequence.                       1234 1234 1234
            RETIME_REPEAT = kFnKatExpressionMathRetimeRepeat,

            //! Mirror the sequence;                       3432 1234 3212
            RETIME_MIRROR = kFnKatExpressionMathRetimeMirror
        };

        static double retime(double frame, double start, double end,
                RetimeHoldMode inMode, RetimeHoldMode outMode);

        // NOISE FUNCTIONS

        // Return a random value between [min,max]
        // This is equvalent to Splat's randVal3 fcn.
        static float  randval(float min, float max, int seed);
        static double randval(double min, double max, int seed);

        // Improved Perlin noise (Siggraph 2002)
        // Ranges from [0,1]
        static float noise(float x);
        static float noise(float x, float y);
        static float noise(float x, float y, float z);
        static float noise(float x, float y, float z, float w);

        // Signed Improved Perlin noise (Siggraph 2002)
        // Ranges from [-1,1]
        // Results are just a rescaled version of noise
        static float snoise(float x);
        static float snoise(float x, float y);
        static float snoise(float x, float y, float z);
        static float snoise(float x, float y, float z, float w);

        // Hashing Functions
        // We always want to return a 32-bit int, no
        // matter which architecture we're on.
        // Uses md5 internally
        static int32_t stablehash(const std::string &cacheID);

    private:
        FnExpressionMath();

        static const FnExpressionMathHostSuite_v1 *_getSuite();
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT


#endif // FnGeolibServicesExpressionMath_H
