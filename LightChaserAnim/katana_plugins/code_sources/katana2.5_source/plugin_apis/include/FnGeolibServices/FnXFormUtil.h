#ifndef FnGeolibServicesXformUtil_H
#define FnGeolibServicesXformUtil_H

#include <stdint.h>
#include <limits>
#include <string>
#include <utility>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnGeolibServices/FnGeolibServicesAPI.h>
#include <FnGeolibServices/ns.h>
#include <FnGeolibServices/suite/FnXFormUtilSuite.h>

#include <FnPluginSystem/FnPluginSystem.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FNGEOLIBSERVICES_API FnXFormUtil
    {
    public:
        // Calc 4x4 from "xform" attr
        // Matrix ordering matches Imath convention:
        // FnAttribute::DoubleAttribute::array_type m = attr.getNearestSample(0.f);
        // Imath::M44d(m[0], m[1], m[2], m[3],
        //               m[4], m[5], m[6], m[7],
        //               m[8], m[9], m[10], m[11],
        //               m[12], m[13], m[14], m[15]);
        // or...
        // Imath::M44d( (double(*)[4]) m.data());

        // return (4x4 matrix attr, isAbsolute)
        // This will do xform interpolation as needed
        static std::pair<FnAttribute::DoubleAttribute, bool>
            CalcTransformMatrixAtTime(
                const FnAttribute::GroupAttribute & groupAttr,
                float time);

        // return (4x4 matrix attr, isAbsolute)
        // This will do xform interpolation as needed
        static std::pair<FnAttribute::DoubleAttribute, bool>
            CalcTransformMatrixAtTimes(
                const FnAttribute::GroupAttribute & groupAttr,
                const float * time, int numSamples);

        // return (4x4 matrix attr, isAbsolute)
        // This will do xform interpolation as needed
        static std::pair<FnAttribute::DoubleAttribute, bool>
            CalcTransformMatrixAtExistingTimes(
                const FnAttribute::GroupAttribute & groupAttr);

        //
        // Bounds handling
        //

        // conventional attrname: "bound"
        // FnAttribute::DoubleAttribute
        // tuple-size: 2
        // len 6, [xMin, xMax, yMin, yMax, zMin, zMax]

        static FnAttribute::DoubleAttribute CreateBoundsAttr(
            double xMin, double xMax,
            double yMin, double yMax,
            double zMin, double zMax);

        // If the boundsAttrs are multi-sampled, the output bounds
        // will be up-sampled to the union of incoming sample times.
        // bounds data at sample-times not provided (but needed) will
        // be synthesized using fillInterpSample

        static FnAttribute::DoubleAttribute MergeBounds(
            const FnAttribute::DoubleAttribute & boundAttr1,
            const FnAttribute::DoubleAttribute & boundAttr2);

        // This will return a new bound attribute which is the new bounds
        // after baking in the specified coordinate system.
        //
        // The input xform can either be specified as a 4x4 DoubleAttr (matrix)
        // or as an unbaked GroupAttribute xform hierarchy. All other types
        // will result in failure. (an invalid attr being returned)
        //
        // The computed attr will contain time samples at the union of existing
        // samples in the incoming xform and bound attrs.
        //
        // If a user requires a non-multisampled bound attr, this call should
        // be followed up with CollapseBoundTimeSamples(...)

        static FnAttribute::DoubleAttribute
            CalcTransformedBoundsAtExistingTimes(
                const FnAttribute::Attribute & xform,
                const FnAttribute::DoubleAttribute & boundAttr);

        // For bound attrs with multi-samples, this will create
        // a single-sampled attr with the merged union
        static FnAttribute::DoubleAttribute CollapseBoundsTimeSamples(
            const FnAttribute::DoubleAttribute & boundAttr);

        // Authoring "xform"
        // GroupInherit = False
        // All children either DoubleAttributes or GroupAttributes

        // For the specified group builder, set group inherit to false
        // (as all xforms should be) and then do a shallow update with the
        // specified attribute, if provided.
        //
        // Note that if one's command sequence utilizes only
        // Init(gb, ...)
        // PushXXX(...)
        // PushXXX(...)
        // PushXXX(...)
        // etc,
        // it is ok to construct the group builder with the
        // GroupBuilder::BuilderModeStrict argument

        static void InitXForm(FnAttribute::GroupBuilder & gb);

        static void InitXForm(FnAttribute::GroupBuilder & gb,
            const FnAttribute::Attribute & xform);

        static void PushRotateAttr(FnAttribute::GroupBuilder & gb,
            double angle, double x, double y, double z);

        static FnAttribute::Attribute PushRotateAttr(
            const FnAttribute::Attribute & xform,
            double angle, double x, double y, double z);

        static void PushTranslateAttr(FnAttribute::GroupBuilder & gb,
            double x, double y, double z);

        static FnAttribute::Attribute PushTranslateAttr(
            const FnAttribute::Attribute & xform,
            double x, double y, double z);

        static void PushScaleAttr(
            FnAttribute::GroupBuilder & gb,
            double x, double y, double z);

        static FnAttribute::Attribute PushScaleAttr(
            const FnAttribute::Attribute & xform,
            double x, double y, double z);

        static void PushMatrixAttr(
            FnAttribute::GroupBuilder & gb,
            const double * mtx16);

        static FnAttribute::Attribute PushMatrixAttr(
            const FnAttribute::Attribute & xform,
            const double * mtx16);

        static void PushOriginAttr(FnAttribute::GroupBuilder & gb);

        static FnAttribute::Attribute PushOriginAttr(
            const FnAttribute::Attribute & xform);

    private:
        FnXFormUtil();

        static const FnXFormUtilHostSuite_v1 *_getSuite();
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT


#endif // FnGeolibServicesXformUtil_H
