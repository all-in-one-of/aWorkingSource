// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_COPYRENDERACTION_H
#define FNRENDER_COPYRENDERACTION_H

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
     * @brief A render action which renders to a temporary location and
     *        then copies the rendered file to the target location.
     *
     * \note
     * The temporary file is deleted unless the
     * KATANA_KEEP_TEMP_RENDER_FILES environment variable is set.
     */
    class CopyRenderAction : public RenderAction
    {
    public:
        /**
         * @param renderTargetLocation The file location which read and
         *                             displayed in the monitor.
         * @param tempRenderLocation   The file location which is
         *                             rendered to and then copied
         *                             to the target location.
         */
        CopyRenderAction(const std::string& renderTargetLocation,
                         const std::string& tempRenderLocation);
        virtual ~CopyRenderAction() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

    protected:
        std::string _tempRenderLocation;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
