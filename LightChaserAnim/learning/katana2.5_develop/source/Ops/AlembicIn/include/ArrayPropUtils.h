#ifndef FnGeolibOp_AlembicIn_ArrayPropUtils_H
#define FnGeolibOp_AlembicIn_ArrayPropUtils_H

#include <Alembic/Abc/All.h>
#include <FnAttribute/FnGroupBuilder.h>
#include "AbcCook.h"

namespace AlembicIn
{

void arrayPropertyToAttr(Alembic::Abc::ICompoundProperty & iParent,
    const Alembic::AbcCoreAbstract::PropertyHeader & iPropHeader,
    const std::string & iPropName,
    FnKatAttributeType iType,
    AbcCookPtr ioCook,
    Foundry::Katana::GroupBuilder & oStaticGb);

void arrayPropertyToAttr(ArrayProp & iProp,
    const OpArgs & iArgs,
    Foundry::Katana::GroupBuilder & oGb);

}

#endif
