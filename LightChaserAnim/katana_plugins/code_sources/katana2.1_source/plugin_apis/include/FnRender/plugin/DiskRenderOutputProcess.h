// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_DISKRENDEROUTPUTPROCESS_H
#define FNRENDER_DISKRENDEROUTPUTPROCESS_H

#include "FnRender/plugin/RenderAction.h"
#include "FnRender/plugin/NoOutputRenderAction.h"
#include "FnRender/plugin/TemporaryRenderAction.h"
#include "FnRender/plugin/CopyRenderAction.h"
#include "FnRender/plugin/CopyAndConvertRenderAction.h"
#include "FnRender/plugin/PostCommandsRenderAction.h"

#include "FnAttribute/FnGroupBuilder.h"
#include "FnAttribute/FnAttribute.h"

#include <map>
#include <memory>

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
     * @brief Configures the render process for disk renders and defines how
     *        render outputs are managed.
     */
    class DiskRenderOutputProcess
    {
    public:
        DiskRenderOutputProcess();
        virtual ~DiskRenderOutputProcess() {}

        /**
         * A render action declares where the outputs are rendered to and
         * how they are handled after the render is complete.
         * All stages of the render action are executed before the
         * post-render commands.
         *
         * @param renderAction The render action used for this render output
         *                     process.
         */
        void setRenderAction(std::auto_ptr<RenderAction> renderAction);

        /**
         * Add a command line which is executed before the render.
         *
         * @param command Pre-render command line
         */
        void addPreCommand(const std::string & command);

        /**
         * Add a command line which is executed after the render.
         * The post-render commands are always executed after all the
         * stages of a render action but before temporary files are
         * cleaned up.
         *
         * @param command Post-render command line
         */
        void addPostCommand(const std::string & command);

        FnAttribute::GroupAttribute buildRenderOutputAttribute() const;

    private:
        std::auto_ptr<RenderAction> _renderAction;
        std::vector<std::string>    _preCommands;
        std::vector<std::string>    _postCommands;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
