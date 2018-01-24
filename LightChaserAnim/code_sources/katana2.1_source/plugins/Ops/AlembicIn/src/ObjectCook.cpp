#include "AbcCook.h"
#include "ArrayPropUtils.h"
#include "ScalarPropUtils.h"
#include "ArbitraryGeomParamUtils.h"


namespace AlembicIn
{

bool isBoundBox(const Alembic::Abc::PropertyHeader & iPropHeader)
{

    if (iPropHeader.getDataType().getExtent() != 6)
    {
        return false;
    }

    Alembic::Util::PlainOldDataType podType =
        iPropHeader.getDataType().getPod();
    if (podType != Alembic::Util::kFloat64POD &&
        podType != Alembic::Util::kFloat32POD)
    {
        return false;
    }

    std::string interp = iPropHeader.getMetaData().get("interpretation");
    if (interp != "box")
    {
        return false;
    }

    return true;
}

int64_t getTupleSize(const Alembic::AbcCoreAbstract::PropertyHeader & iHeader)
{
    int64_t extent = iHeader.getDataType().getExtent();
    if (iHeader.getMetaData().get("interpretation") == "box")
    {
        if (extent == 6)
        {
            extent = 3;
        }
        else if (extent == 4)
        {
            extent = 2;
        }
    }

    if (extent == 0)
    {
        extent = 1;
    }

    return extent;
}

void evalObject(AbcCookPtr ioCookPtr, const OpArgs & iArgs,
                FnAttribute::GroupBuilder & oGb)
{

    if (ioCookPtr->visProp.valid())
    {
        Alembic::Util::int8_t visValue = -1;
        Alembic::Abc::ISampleSelector ss(iArgs.getAbcFrameTime());
        ioCookPtr->visProp.get(visValue, ss);
        if (visValue > -1)
        {
            oGb.set("visible", FnAttribute::IntAttribute(visValue));
        }
    }

    if (!ioCookPtr->objPtr)
    {
        return;
    }

    std::vector< ArrayProp >::iterator at = ioCookPtr->arrayProps.begin();
    for (; at != ioCookPtr->arrayProps.end(); ++at)
    {
        arrayPropertyToAttr(*at, iArgs, oGb);
    }

    std::vector< ScalarProp >::iterator st = ioCookPtr->scalarProps.begin();
    for (; st != ioCookPtr->scalarProps.end(); ++st)
    {
        scalarPropertyToAttr(*st, iArgs, oGb);
    }

    std::vector< IndexedGeomParamPair >::iterator it =
        ioCookPtr->forcedExpandProps.begin();
    for (; it != ioCookPtr->forcedExpandProps.end(); ++it)
    {
        indexedParamToAttr(*it, iArgs, oGb);
    }

    Alembic::AbcGeom::IXformPtr xformPtr = Alembic::Util::dynamic_pointer_cast<
        Alembic::AbcGeom::IXform, Alembic::Abc::IObject >(ioCookPtr->objPtr);
    if (xformPtr)
    {
        evalXform(xformPtr->getSchema(), iArgs, oGb);
        return;
    }

    Alembic::AbcGeom::INuPatchPtr patchPtr =
        Alembic::Util::dynamic_pointer_cast<Alembic::AbcGeom::INuPatch,
            Alembic::Abc::IObject >(ioCookPtr->objPtr);
    if (patchPtr)
    {
        evalNuPatch(patchPtr->getSchema(), iArgs, false, oGb);
        return;
    }

    Alembic::AbcGeom::ICurvesPtr curvesPtr =
        Alembic::Util::dynamic_pointer_cast<Alembic::AbcGeom::ICurves,
            Alembic::Abc::IObject >(ioCookPtr->objPtr);
    if (curvesPtr)
    {
        evalCurves(curvesPtr->getSchema(), iArgs, oGb);
        return;
    }

    Alembic::AbcGeom::ICameraPtr cameraPtr =
        Alembic::Util::dynamic_pointer_cast<Alembic::AbcGeom::ICamera,
            Alembic::Abc::IObject >(ioCookPtr->objPtr);
    if (cameraPtr)
    {
        evalCamera(cameraPtr->getSchema(), iArgs, oGb);
        return;
    }
}

// fills in the static group, and the animated array and static props
void initAbcCook(AbcCookPtr ioCookPtr,
                 FnAttribute::GroupBuilder & oStaticGb)
{

    const Alembic::AbcCoreAbstract::ObjectHeader & header =
        ioCookPtr->objPtr->getHeader();

    Alembic::AbcGeom::IVisibilityProperty visProp =
        Alembic::AbcGeom::GetVisibilityProperty(*ioCookPtr->objPtr);

    if (visProp.valid() && visProp.isConstant())
    {
        Alembic::Util::int8_t visValue = -1;
        visProp.get(visValue);
        if (visValue > -1)
        {
            oStaticGb.set("visible", FnAttribute::IntAttribute(visValue));
        }
    }
    else
    {
        if (visProp.valid())
            ioCookPtr->animatedSchema = true;
        ioCookPtr->visProp = visProp;
    }

    if (Alembic::AbcGeom::IXform::matches(header))
    {
        cookXform(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::IPolyMesh::matches(header))
    {
        cookPolyMesh(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::ISubD::matches(header))
    {
        cookSubd(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::ICamera::matches(header))
    {
        cookCamera(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::IFaceSet::matches(header))
    {
        cookFaceset(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::ICurves::matches(header))
    {
        cookCurves(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::INuPatch::matches(header))
    {
        cookNuPatch(ioCookPtr, oStaticGb);
    }
    else if (Alembic::AbcGeom::IPoints::matches(header))
    {
        cookPoints(ioCookPtr, oStaticGb);
    }
    else
    {
        // just set this up as a group, we'll ignore all the properties for now
        oStaticGb.set("type", FnAttribute::StringAttribute("group"));
    }
}

namespace
{
    std::string safeAttrName(const std::string & name)
    {
        std::string result = name;

        for (size_t i = 0; i < result.size(); ++i)
        {
            if (result[i] == '.')
            {
                result[i] = '_';
            }
        }

        return result;
    }
}

void processUserProperties(AbcCookPtr ioCook,
    Alembic::Abc::ICompoundProperty & iParent,
    FnAttribute::GroupBuilder & oStaticGb,
    const std::string & iAttrPath)
{

    if (!iParent.valid())
    {
        return;
    }

    for (size_t i = 0; i < iParent.getNumProperties(); ++i)
    {
        const Alembic::AbcCoreAbstract::PropertyHeader &propHeader =
            iParent.getPropertyHeader(i);

        std::string propName = iAttrPath + safeAttrName(propHeader.getName());

        //recurse if it's a compound
        if (propHeader.isCompound())
        {
            Alembic::Abc::ICompoundProperty childCompound(iParent,
                propHeader.getName());

            if (childCompound.valid())
            {
                processUserProperties(ioCook, childCompound, oStaticGb,
                    propName+".");
            }
        }
        else if (propHeader.isScalar())
        {
            scalarPropertyToAttr(iParent, propHeader, propName,
                                 ioCook, oStaticGb);
        }
        else if (propHeader.isArray())
        {
            arrayPropertyToAttr(iParent, propHeader, propName,
                                FnAttribute::NullAttribute::getKatAttributeType(),
                                ioCook, oStaticGb);
        }
    }
}

Alembic::Util::PlainOldDataType FnAttrTypeToPODType(FnKatAttributeType iType)
{
    if (iType == kFnKatAttributeTypeInt)
    {
        return Alembic::Util::kInt32POD;
    }
    else if (iType == kFnKatAttributeTypeFloat)
    {
        return Alembic::Util::kFloat32POD;
    }
    else if (iType == kFnKatAttributeTypeDouble)
    {
        return Alembic::Util::kFloat64POD;
    }
    else if (iType == kFnKatAttributeTypeString)
    {
        return Alembic::Util::kStringPOD;
    }

    return Alembic::Util::kUnknownPOD;
}

} //end of namespace AlembicIn

