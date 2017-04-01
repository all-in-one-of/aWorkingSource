#ifndef FnGeolibOp_AlembicIn_ArbitraryGeomParamUtils_H
#define FnGeolibOp_AlembicIn_ArbitraryGeomParamUtils_H

#include <Alembic/Abc/All.h>
#include <FnAttribute/FnGroupBuilder.h>
#include "AbcCook.h"

namespace AlembicIn
{

void indexedParamToAttr(IndexedGeomParamPair & iProp,
    const OpArgs & iArgs,
    Foundry::Katana::GroupBuilder & oGb);

void processArbitraryGeomParam(AbcCookPtr ioCook,
    Alembic::Abc::ICompoundProperty & iParent,
    const Alembic::AbcCoreAbstract::PropertyHeader & iPropHeader,
    Foundry::Katana::GroupBuilder & oStaticGb,
    const std::string & iAttrPath);

void processArbGeomParams(AbcCookPtr ioCook,
    Alembic::Abc::ICompoundProperty & iParent,
    Foundry::Katana::GroupBuilder & oStaticGb);
}

#endif
