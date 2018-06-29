// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_POSTCOMMANDRENDERACTION_H
#define FNRENDER_POSTCOMMANDRENDERACTION_H

#include "FnAttribute/FnAttribute.h"
#include "FnAttribute/FnGroupBuilder.h"
#include "FnRender/FnRenderAPI.h"
#include "FnRender/plugin/CopyRenderAction.h"

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
     * @brief A render action which renders to a temporary location and
     *        then runs post commands to process image and copy to
     *         target render location.
     *
     * \note
     * The temporary file is deleted unless the
     * KATANA_KEEP_TEMP_RENDER_FILES environment variable is set.
     */
    class FNRENDER_API PostCommandsRenderAction : public CopyRenderAction
    {
    public:
        /**
         * @param renderTargetLocation The file location which is read and
         *                             displayed in the monitor.
         * @param tempRenderLocation   The file location which is rendered
         *                             to and then copied to the target
         *                             location.
         * @param postCommand          The external command to run to convert
         *                             tempRenderLocation to renderTargetLocation.
         */
        PostCommandsRenderAction(const std::string& renderTargetLocation,
                                   const std::string& tempRenderLocation,
                                   const std::vector<std::string>& postCommands);

        virtual ~PostCommandsRenderAction() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        void addCleanupFile(const std::string & cleanupFile);

    protected:
        std::vector<std::string> _postCommands;
        std::vector<std::string> _cleanupFiles;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
