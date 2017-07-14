// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnGeolibServices/FnXFormUtil.h>

#include <FnAttribute/FnAttribute.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    std::pair<FnAttribute::DoubleAttribute, bool>
        FnXFormUtil::CalcTransformMatrixAtTime(
            const FnAttribute::GroupAttribute & groupAttr,
            float time)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return std::make_pair(FnAttribute::Attribute(), false);

        uint8_t isabs = false;
        FnAttributeHandle ret = suite->calcTransformMatrix1(
            &isabs, groupAttr.getHandle(), time);

        return std::make_pair(
            FnAttribute::Attribute::CreateAndSteal(ret), bool(isabs));
    }

    std::pair<FnAttribute::DoubleAttribute, bool>
        FnXFormUtil::CalcTransformMatrixAtTimes(
            const FnAttribute::GroupAttribute & groupAttr,
            const float * times, int numtimes)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return std::make_pair(FnAttribute::Attribute(), false);

        uint8_t isabs = false;
        FnAttributeHandle ret = suite->calcTransformMatrix2(
            &isabs, groupAttr.getHandle(), times, int32_t(numtimes));

        return std::make_pair(
            FnAttribute::Attribute::CreateAndSteal(ret), bool(isabs));
    }

    std::pair<FnAttribute::DoubleAttribute, bool>
        FnXFormUtil::CalcTransformMatrixAtExistingTimes(
            const FnAttribute::GroupAttribute & groupAttr)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return std::make_pair(FnAttribute::Attribute(), false);

        uint8_t isabs = false;
        FnAttributeHandle ret = suite->calcTransformMatrix3(
            &isabs, groupAttr.getHandle());
        return std::make_pair(
            FnAttribute::Attribute::CreateAndSteal(ret), bool(isabs));
    }

    void FnXFormUtil::InitXForm(FnAttribute::GroupBuilder & gb)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->initXForm(gb.getHandle(), 0);
    }

    void FnXFormUtil::InitXForm(FnAttribute::GroupBuilder & gb,
        const FnAttribute::Attribute & xform)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->initXForm(gb.getHandle(), xform.getHandle());
    }

    void FnXFormUtil::PushRotateAttr(FnAttribute::GroupBuilder & gb,
        double angle, double x, double y, double z)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->pushRotateAttr1(gb.getHandle(), angle, x, y, z);
    }

    FnAttribute::Attribute FnXFormUtil::PushRotateAttr(
        const FnAttribute::Attribute & xform,
        double angle, double x, double y, double z)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->pushRotateAttr2(xform.getHandle(), angle, x, y, z));
    }

    void FnXFormUtil::PushTranslateAttr(FnAttribute::GroupBuilder & gb,
        double x, double y, double z)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->pushTranslateAttr1(gb.getHandle(), x, y, z);
    }

    FnAttribute::Attribute FnXFormUtil::PushTranslateAttr(
        const FnAttribute::Attribute & xform,
        double x, double y, double z)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->pushTranslateAttr2(xform.getHandle(), x, y, z));
    }

    void FnXFormUtil::PushScaleAttr(
        FnAttribute::GroupBuilder & gb,
        double x, double y, double z)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->pushScaleAttr1(gb.getHandle(), x, y, z);
    }

    FnAttribute::Attribute FnXFormUtil::PushScaleAttr(
        const FnAttribute::Attribute & xform,
        double x, double y, double z)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->pushScaleAttr2(xform.getHandle(), x, y, z));
    }

    void FnXFormUtil::PushMatrixAttr(
        FnAttribute::GroupBuilder & gb,
        const double * mtx16)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->pushMatrixAttr1(gb.getHandle(), mtx16);
    }

    FnAttribute::Attribute FnXFormUtil::PushMatrixAttr(
        const FnAttribute::Attribute & xform,
        const double * mtx16)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->pushMatrixAttr2(xform.getHandle(), mtx16));
    }

    void FnXFormUtil::PushOriginAttr(FnAttribute::GroupBuilder & gb)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;
        suite->pushOriginAttr1(gb.getHandle());
    }

    FnAttribute::Attribute FnXFormUtil::PushOriginAttr(
        const FnAttribute::Attribute & xform)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->pushOriginAttr2(xform.getHandle()));
    }


    FnAttribute::DoubleAttribute FnXFormUtil::CreateBoundsAttr(
        double xMin, double xMax,
        double yMin, double yMax,
        double zMin, double zMax)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->createBoundsAttr(xMin, xMax, yMin, yMax, zMin, zMax));
    }

    FnAttribute::DoubleAttribute FnXFormUtil::MergeBounds(
        const FnAttribute::DoubleAttribute & boundAttr1,
        const FnAttribute::DoubleAttribute & boundAttr2)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->mergeBounds(boundAttr1.getHandle(), boundAttr2.getHandle()));
    }

    FnAttribute::DoubleAttribute
        FnXFormUtil::CalcTransformedBoundsAtExistingTimes(
            const FnAttribute::Attribute & xform,
            const FnAttribute::DoubleAttribute & boundAttr)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->calcTransformedBoundsAtExistingTimes(xform.getHandle(), boundAttr.getHandle()));
    }

    FnAttribute::DoubleAttribute FnXFormUtil::CollapseBoundsTimeSamples(
        const FnAttribute::DoubleAttribute & boundAttr)
    {
        const FnXFormUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();
        return FnAttribute::Attribute::CreateAndSteal(
            suite->collapseBoundsTimeSamples(boundAttr.getHandle()));
    }

    ////////////////////////////////////////////////////////////////////////////

    const FnXFormUtilHostSuite_v1 * FnXFormUtil::_getSuite()
    {
        static const FnXFormUtilHostSuite_v1 * suite = 0x0;

        if (!suite)
        {
            suite = reinterpret_cast<const FnXFormUtilHostSuite_v1 *>(
                    FnPluginManager::PluginManager::getHostSuite(
                            "XFormUtilHost", 1));
        }

        return suite;
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
