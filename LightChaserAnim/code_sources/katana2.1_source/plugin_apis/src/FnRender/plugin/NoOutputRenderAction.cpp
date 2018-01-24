// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/NoOutputRenderAction.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

NoOutputRenderAction::NoOutputRenderAction() : RenderAction("")
{
    setLoadOutputInMonitor(false);
}

}
}
}

