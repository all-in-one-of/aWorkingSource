// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_NOOUTPUTRENDERACTION_H
#define FNRENDER_NOOUTPUTRENDERACTION_H

#include "FnAttribute/FnAttribute.h"
#include "FnAttribute/FnGroupBuilder.h"
#include "FnRender/FnRenderAPI.h"
#include "FnRender/plugin/RenderAction.h"

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
    class FNRENDER_API NoOutputRenderAction : public RenderAction
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
