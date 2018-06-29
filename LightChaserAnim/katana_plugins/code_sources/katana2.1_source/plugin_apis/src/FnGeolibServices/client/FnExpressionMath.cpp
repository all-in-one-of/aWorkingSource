// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnGeolibServices/FnExpressionMath.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    int FnExpressionMath::clamp(int value, int a, int b)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0;

        return suite->clampint(value, a, b);
    }

    float FnExpressionMath::clamp(float value, float a, float b)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->clampfloat(value, a, b);
    }

    double FnExpressionMath::clamp(double value, double a, double b)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->clampdouble(value, a, b);
    }

    float FnExpressionMath::lerp(float mix, float a, float b)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->lerpfloat(mix, a, b);
    }

    double FnExpressionMath::lerp(double mix, double a, double b)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->lerpdouble(mix, a, b);
    }

    float FnExpressionMath::smoothstep(float t)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->smoothstepfloat(t);
    }

    double FnExpressionMath::smoothstep(double t)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->smoothstepdouble(t);
    }

    float FnExpressionMath::fit(float value, float oldmin, float oldmax,
            float newmin, float newmax)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->fitfloat(value, oldmin, oldmax, newmin, newmax);
    }

    double FnExpressionMath::fit(double value, double oldmin, double oldmax,
            double newmin, double newmax)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->fitdouble(value, oldmin, oldmax, newmin, newmax);
    }

    float FnExpressionMath::cfit(float value, float oldmin, float oldmax,
            float newmin, float newmax)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->cfitfloat(value, oldmin, oldmax, newmin, newmax);
    }

    double FnExpressionMath::cfit(double value, double oldmin, double oldmax,
            double newmin, double newmax)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->cfitdouble(value, oldmin, oldmax, newmin, newmax);
    }

    float FnExpressionMath::softcfit(float value, float oldmin, float oldmax,
            float newmin, float newmax)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->softcfitfloat(value, oldmin, oldmax, newmin, newmax);
    }

    double FnExpressionMath::softcfit(double value, double oldmin, double oldmax,
            double newmin, double newmax)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->softcfitdouble(value, oldmin, oldmax, newmin, newmax);
    }

    double FnExpressionMath::retime(double frame, double start, double end,
            RetimeHoldMode inMode, RetimeHoldMode outMode)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->retime(frame, start, end, inMode, outMode);
    }

    float FnExpressionMath::randval(float min, float max, int seed)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->randvalfloat(min, max, seed);
    }

    double FnExpressionMath::randval(double min, double max, int seed)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0;

        return suite->randvaldouble(min, max, seed);
    }

    float FnExpressionMath::noise(float x)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->noise1D(x);
    }

    float FnExpressionMath::noise(float x, float y)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->noise2D(x, y);
    }

    float FnExpressionMath::noise(float x, float y, float z)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->noise3D(x, y, z);
    }

    float FnExpressionMath::noise(float x, float y, float z, float w)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->noise4D(x, y, z, w);
    }

    float FnExpressionMath::snoise(float x)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->snoise1D(x);
    }

    float FnExpressionMath::snoise(float x, float y)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->snoise2D(x, y);
    }

    float FnExpressionMath::snoise(float x, float y, float z)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->snoise3D(x, y, z);
    }

    float FnExpressionMath::snoise(float x, float y, float z, float w)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0.0f;

        return suite->snoise4D(x, y, z, w);
    }

    int32_t FnExpressionMath::stablehash(const std::string &cacheID)
    {
        const FnExpressionMathHostSuite_v1 * suite = _getSuite();
        if (!suite) return 0;

        return suite->stablehash(cacheID.c_str(),
                static_cast<int32_t>(cacheID.size()));
    }


    ////////////////////////////////////////////////////////////////////////////

    const FnExpressionMathHostSuite_v1 * FnExpressionMath::_getSuite()
    {
        static const FnExpressionMathHostSuite_v1 * suite = 0x0;

        if (!suite)
        {
            suite = reinterpret_cast<const FnExpressionMathHostSuite_v1 *>(
                    FnPluginManager::PluginManager::getHostSuite(
                            "ExpressionMathHost", 1));
        }

        return suite;
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
