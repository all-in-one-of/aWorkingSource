#ifndef FnXFormUtilSuite_H
#define FnXFormUtilSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>

#include <stdint.h>

extern "C" {

#define FnXFormUtilHostSuite_version 1

struct FnXFormUtilHostSuite_v1
{
    FnAttributeHandle (*calcTransformMatrix1)(uint8_t * isabs,
        FnAttributeHandle xform, float time);

    FnAttributeHandle (*calcTransformMatrix2)(uint8_t * isabs,
        FnAttributeHandle xform, const float * times, int32_t numtimes);

    FnAttributeHandle (*calcTransformMatrix3)(uint8_t * isabs,
        FnAttributeHandle xform);

    void (*initXForm)(
        FnGroupBuilderHandle gb, FnAttributeHandle xform);

    void (*pushRotateAttr1)(FnGroupBuilderHandle gb,
        double angle, double x, double y, double z);

    FnAttributeHandle (*pushRotateAttr2)(FnAttributeHandle xform,
        double angle, double x, double y, double z);

    void (*pushTranslateAttr1)(FnGroupBuilderHandle gb,
        double x, double y, double z);

    FnAttributeHandle (*pushTranslateAttr2)(FnAttributeHandle xform,
        double x, double y, double z);

    void (*pushScaleAttr1)(FnGroupBuilderHandle gb,
        double x, double y, double z);

    FnAttributeHandle (*pushScaleAttr2)(FnAttributeHandle xform,
        double x, double y, double z);

    void (*pushMatrixAttr1)(FnGroupBuilderHandle gb,
        const double * mtx16);

    FnAttributeHandle (*pushMatrixAttr2)(FnAttributeHandle xform,
        const double * mtx16);

    void (*pushOriginAttr1)(FnGroupBuilderHandle gb);

    FnAttributeHandle (*pushOriginAttr2)(FnAttributeHandle xform);

    FnAttributeHandle (*createBoundsAttr)(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

    FnAttributeHandle (*mergeBounds)(FnAttributeHandle bounds1, FnAttributeHandle bounds2);

    FnAttributeHandle (*calcTransformedBoundsAtExistingTimes)(FnAttributeHandle xform, FnAttributeHandle bound);

    FnAttributeHandle (*collapseBoundsTimeSamples)(FnAttributeHandle bound);
};

}


#endif
