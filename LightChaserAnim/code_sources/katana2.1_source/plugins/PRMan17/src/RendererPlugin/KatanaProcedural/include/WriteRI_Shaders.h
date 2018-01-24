// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef WRITERI_SHADERS_H
#define WRITERI_SHADERS_H
#include <FnAttribute/FnAttribute.h>
#include <FnScenegraphIterator/FnScenegraphIterator.h>

namespace PRManProcedural
{
    void WriteRI_Object_Material(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
    void WriteRI_Object_Material(const std::string & identifier, FnAttribute::GroupAttribute material_attr, bool flipZ, FnAttribute::GroupAttribute info_attr, PRManPluginState* sharedState);
    void WriteRI_Object_IlluminationList(FnKat::FnScenegraphIterator sgIterator, PRManPluginState* sharedState);
}
#endif
