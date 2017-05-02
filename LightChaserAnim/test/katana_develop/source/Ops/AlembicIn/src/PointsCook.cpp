#include "AbcCook.h"
#include "ArrayPropUtils.h"
#include "ScalarPropUtils.h"
#include "ArbitraryGeomParamUtils.h"

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnDataBuilder.h>

namespace AlembicIn
{
    void cookPoints(AbcCookPtr ioCook, FnAttribute::GroupBuilder & oStaticGb)
    {
        Alembic::AbcGeom::IPointsPtr objPtr(
            new Alembic::AbcGeom::IPoints(*(ioCook->objPtr),
                                          Alembic::AbcGeom::kWrapExisting));

        Alembic::AbcGeom::IPointsSchema schema = objPtr->getSchema();

        oStaticGb.set("type", FnAttribute::StringAttribute("pointcloud"));

        Alembic::Abc::ICompoundProperty userProp = schema.getUserProperties();
        Alembic::Abc::ICompoundProperty arbGeom = schema.getArbGeomParams();
        std::string abcUser = "abcUser.";
        processUserProperties(ioCook, userProp, oStaticGb, abcUser);
        processArbGeomParams(ioCook, arbGeom, oStaticGb);

        Alembic::Abc::IP3fArrayProperty pointsProp =
            schema.getPositionsProperty();
        if (pointsProp.valid())
        {
            arrayPropertyToAttr(schema, pointsProp.getHeader(),
                "geometry.point.P", kFnKatAttributeTypeFloat,
                ioCook, oStaticGb);
        }

        Alembic::Abc::IV3fArrayProperty velocProp =
            schema.getVelocitiesProperty();
        if (velocProp.valid())
        {
            arrayPropertyToAttr(schema, velocProp.getHeader(),
                "geometry.point.v", kFnKatAttributeTypeFloat,
                ioCook, oStaticGb);
        }

        Alembic::Abc::IBox3dProperty boundsProp =
            schema.getSelfBoundsProperty();
        if (boundsProp.valid())
        {
            scalarPropertyToAttr(schema, boundsProp.getHeader(),
                "bound", ioCook, oStaticGb);
        }

        Alembic::AbcGeom::IFloatGeomParam widthProp = schema.getWidthsParam();
        if (widthProp.valid())
        {
            Alembic::AbcGeom::GeometryScope scope = widthProp.getScope();
            std::string widthName = "geometry.point.";

            if (scope == Alembic::AbcGeom::kVertexScope)
            {
                widthName += "width";
            }
            else
            {
                widthName += "constantwidth";
            }

            processArbitraryGeomParam(ioCook, schema, widthProp.getHeader(),
                oStaticGb, widthName);
        }
    }

} //end of namespace AlembicIn
