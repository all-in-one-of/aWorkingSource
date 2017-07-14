// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManRendererInfo.h>

namespace // anonymous
{
DEFINE_RENDERERINFO_PLUGIN(PRManRendererInfo)
} // namespace anonymous

void registerPlugins()
{
  REGISTER_PLUGIN(PRManRendererInfo, "PRManRendererInfo", 0, 1);
}
