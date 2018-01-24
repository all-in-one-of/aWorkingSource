// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef WRITERI_STATEMENTS_H
#define WRITERI_STATEMENTS_H

#include <FnAttribute/FnAttribute.h>
#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <WriteRI_Util.h>

namespace PRManProcedural
{
    void WriteRI_Object_Statements(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
    void WriteRI_Object_PRManStatements(const std::string & errorIdentifier, FnAttribute::GroupAttribute prmanStatementsAttr, PRManPluginState* sharedState);
    void WriteRI_Object_AttributeStatements(const std::string & errorIdentifier, FnAttribute::GroupAttribute stmtAttr, PRManPluginState* sharedState);
    void WriteRI_Object_ConvertAttrListToStatements(const AttrList& attrList, const std::string& groupAttrName);
}
#endif
