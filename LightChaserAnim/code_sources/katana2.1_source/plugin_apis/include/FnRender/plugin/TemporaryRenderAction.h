// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_TEMPORARYRENDERACTION_H
#define FNRENDER_TEMPORARYRENDERACTION_H

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
     * @brief A render action which renders to a temporary location.
     *
     * This action renders a file to a temporary location which is deleted when the
     * render is complete unless the KATANA_KEEP_TEMP_RENDER_FILES environment
     * variable is set. By default nothing is loaded into the monitor when using
     * this action.
     *
     * This action is useful when e.g. a post process is required where a temporary
     * file is written to disk and then processed using a custom command.
     *
     * A render action needs to declare an output for each port on the render node
     * in order to allow a render process to start.
     * Therefore, a render pass that is not expected to produce a render output
     * still needs to use this temporary render action regardless of whether the
     * temporary render location is used anywhere.
     *
     * @see Render::DiskRenderOutputProcess::addPostCommand
     */
    class TemporaryRenderAction : public RenderAction
    {
    public:
        TemporaryRenderAction(const std::string& tempRenderLocation);
        virtual ~TemporaryRenderAction() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
