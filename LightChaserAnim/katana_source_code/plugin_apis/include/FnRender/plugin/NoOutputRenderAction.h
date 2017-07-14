// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_NOOUTPUTRENDERACTION_H
#define FNRENDER_NOOUTPUTRENDERACTION_H

#include "FnRender/plugin/RenderAction.h"
#include "FnAttribute/FnGroupBuilder.h"
#include "FnAttribute/FnAttribute.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
    /**
     * \ingroup RenderAPI
     */

    /**
     * @brief A render action which does not specify any output
     *        locations.
     */
    class NoOutputRenderAction : public RenderAction
    {
    public:
        NoOutputRenderAction();
        virtual ~NoOutputRenderAction() {}
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
