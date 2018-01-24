// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef COMPANYNAMEKATANA_TEMPLATERENDERERINFOPLUGIN_H
#define COMPANYNAMEKATANA_TEMPLATERENDERERINFOPLUGIN_H

#include "FnRender/plugin/RenderBase.h"


namespace CompanyName
{
namespace Katana
{
    /**
     * \ingroup TemplateRenderPlugin
     */

    /**
     * @brief Template Render Plugin
     */
    class TemplateRenderPlugin : public FnKat::Render::RenderBase
    {
    public:

        TemplateRenderPlugin(FnKat::FnScenegraphIterator rootIterator,
                             FnKat::GroupAttribute arguments);
        ~TemplateRenderPlugin();

        // Render Control

        int start();

        int pause();

        int resume();

        int stop();

        // Interactive live updates

        int startLiveEditing();

        int stopLiveEditing();

        int processControlCommand(const std::string& command);

        int queueDataUpdates(FnKat::GroupAttribute updateAttribute);

        int applyPendingDataUpdates();

        bool hasPendingDataUpdates() const;

        // Disk Render

        void configureDiskRenderOutputProcess(FnKat::Render::DiskRenderOutputProcess& diskRenderOutputProcess,
                                              const std::string& outputName,
                                              const std::string& outputPath,
                                              const std::string& renderMethodName,
                                              const float& frameTime) const;

        // Plugin Interface

        static Foundry::Katana::Render::RenderBase* create(FnKat::FnScenegraphIterator rootIterator, FnKat::GroupAttribute args)
        {
            return new TemplateRenderPlugin(rootIterator, args);
        }

        static void flush()
        {

        }
    };

    /**
     * @}
     */
}
}

#endif
