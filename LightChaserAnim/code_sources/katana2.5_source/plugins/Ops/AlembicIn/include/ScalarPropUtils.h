#ifndef FnGeolibOp_AlembicIn_ScalarPropUtils_H
#define FnGeolibOp_AlembicIn_ScalarPropUtils_H

#include <Alembic/Abc/All.h>
#include <FnAttribute/FnGroupBuilder.h>
#include "AbcCook.h"

namespace AlembicIn
{

void scalarPropertyToAttr(
    Alembic::Abc::ICompoundProperty & iParent,
    const Alembic::AbcCoreAbstract::PropertyHeader & iHeader,
    const std::string & iPropName,
    AbcCookPtr ioCook,
    Foundry::Katana::GroupBuilder & oStaticGb);

void scalarPropertyToAttr(ScalarProp & iProp,
    const OpArgs & iArgs,
    Foundry::Katana::GroupBuilder & oGb);

}

#endif
