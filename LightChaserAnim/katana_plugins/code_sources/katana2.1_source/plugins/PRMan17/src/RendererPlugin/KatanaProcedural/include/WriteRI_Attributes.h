// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef WRITERI_ATTRIBUTES_H
#define WRITERI_ATTRIBUTES_H

#include <FnAttribute/FnAttribute.h>
#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <PRManPluginState.h>

namespace PRManProcedural
{
    void WriteRI_Object_Name(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
    void WriteRI_Object_LevelOfDetail(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
    void WriteRI_Object_Transform(FnAttribute::GroupAttribute xformAttr, FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
    void WriteRI_Object_GlobalTransform(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
    void WriteRI_Object_RelativeScopedCoordinateSystems(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
}

#endif
