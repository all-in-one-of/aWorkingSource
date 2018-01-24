// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_RENDERACTION_H
#define FNRENDER_RENDERACTION_H

#include "FnAttribute/FnGroupBuilder.h"
#include "FnAttribute/FnAttribute.h"
#include "FnRender/plugin/RenderSettings.h"

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
     * @brief A render action which renders directly to the target location.
     */
    class RenderAction
    {
    public:
        /**
         * @param renderTargetLocation The file location which is rendered
         *                             to and then displayed in the monitor.
         */
        RenderAction(const std::string& renderTargetLocation);
        virtual ~RenderAction() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        /**
         * Set the target location of the file which will be rendered to.
         * The rendered file will be read and displayed in the catalog and
         * monitor unless setLoadOutputInMonitor is set to false.
         *
         * @param renderTargetLocation The file location which is rendered
         *                             to and then displayed in the monitor.
         */
        void setRenderTargetLocation(const std::string& renderTargetLocation);

        /**
         * The rendered output is loaded into the monitor and
         * catalog by default unless it is disabled with
         * this function (does not apply to batch rendering).
         *
         * @param loadOutputInMonitor Specifies whether or not
         *        the render output should be loaded in the monitor
         */
        void setLoadOutputInMonitor(bool loadOutputInMonitor);

        /**
         * The rendered output overwrites the target file (even
         * if it is set read-only) unless it is disabled with
         * this function.
         *
         * @param forceOverwriteTarget Specifies whether or not the
         *        render output should forcibly overwrite any existing
         *        file.  Default is true. If set to false, render
         *        will fail if target file is read-only.
         */
        void setForceOverwriteTarget(bool forceOverwriteTarget);

        /**
         * The rendered output creates any necessary directories required
         * to satisfy the path specified for the final render location,
         * unless this feature is disabled with this function.
         *
         * @param createDestinationDirectories Specifies whether or not the
         *        render output should create directories if the destination
         *        path does not exist.  Default is true. If set to false,
         *        render will fail if target path doesn't exist.
         */
        void setCreateDestinationDirectories(bool createDestinationDirectories);

    protected:
        std::string _renderTargetLocation;
        bool        _loadOutputInMonitor;
        bool        _forceOverwriteTarget;
        bool        _createDestinationDirectories;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
