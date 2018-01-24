#ifndef FnExpressionMathSuite_H
#define FnExpressionMathSuite_H

#include <stdint.h>

extern "C" {

#define FnExpressionMathHostSuite_version 1

#define kFnKatExpressionMathRetimeFreeze 0
#define kFnKatExpressionMathRetimeRepeat 1
#define kFnKatExpressionMathRetimeMirror 2

struct FnExpressionMathHostSuite_v1
{
    int32_t (*clampint)(int32_t value, int32_t a, int32_t b);
    float   (*clampfloat)(float value, float a, float b);
    double  (*clampdouble)(double value, double a, double b);

    float   (*lerpfloat)(float mix, float a, float b);
    double  (*lerpdouble)(double mix, double a, double b);

    float   (*smoothstepfloat)(float t);
    double  (*smoothstepdouble)(double t);

    float   (*fitfloat)(float value, float oldmin, float oldmax,
            float newmin, float newmax);
    double  (*fitdouble)(double value, double oldmin, double oldmax,
            double newmin, double newmax);

    float   (*cfitfloat)(float value, float oldmin, float oldmax,
            float newmin, float newmax);
    double  (*cfitdouble)(double value, double oldmin, double oldmax,
            double newmin, double newmax);

    float   (*softcfitfloat)(float value, float oldmin, float oldmax,
            float newmin, float newmax);
    double  (*softcfitdouble)(double value, double oldmin, double oldmax,
            double newmin, double newmax);

    double  (*retime)(double frame, double start, double end,
            int32_t inMode, int32_t outMode);

    float   (*randvalfloat)(float min, float max, int32_t seed);
    double  (*randvaldouble)(double min, double max, int32_t seed);

    float   (*noise1D)(float x);
    float   (*noise2D)(float x, float y);
    float   (*noise3D)(float x, float y, float z);
    float   (*noise4D)(float x, float y, float z, float w);

    float   (*snoise1D)(float x);
    float   (*snoise2D)(float x, float y);
    float   (*snoise3D)(float x, float y, float z);
    float   (*snoise4D)(float x, float y, float z, float w);

    int32_t (*stablehash)(const char *cacheID, int32_t cacheIDSize);
};

} // extern "C"

#endif
