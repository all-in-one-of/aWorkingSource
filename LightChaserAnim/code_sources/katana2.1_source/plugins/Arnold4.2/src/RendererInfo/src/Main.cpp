// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <ArnoldRendererInfo.h>

namespace // anonymous
{
DEFINE_RENDERERINFO_PLUGIN(ArnoldRendererInfo)
} // namespace anonymous

void registerPlugins()
{
  REGISTER_PLUGIN(ArnoldRendererInfo, "ArnoldRendererInfo", 0, 1);
}
