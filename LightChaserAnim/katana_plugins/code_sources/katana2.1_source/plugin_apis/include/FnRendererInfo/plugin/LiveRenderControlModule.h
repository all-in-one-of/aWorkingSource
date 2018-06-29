// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDERERINFO_LIVERENDERCONTROLMODULE_H
#define FNRENDERERINFO_LIVERENDERCONTROLMODULE_H

#include <FnRendererInfo/plugin/LiveRenderFilter.h>

#include <map>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{
    /**
     * \ingroup RenderAPI
     */

    enum LiveRenderFilterMode
    {
        kLiveRenderFilterMode_Append,
        kLiveRenderFilterMode_Replace
    };

    enum LiveRenderRecookTiming
    {
        kLiveRenderRecookTiming_BeforeUpdate,
        kLiveRenderRecookTiming_AfterUpdate
    };

    class LiveRenderControlModule
    {
    public:
        LiveRenderControlModule(const std::string& name);
        virtual ~LiveRenderControlModule() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const = 0;
        virtual void setLabel(const std::string& label);

    protected:
        std::string _name;
        std::string _label;

        std::string getModulePrefix() const;
    };

    class LiveRenderButtonWidget
    {
    public:
        LiveRenderButtonWidget() {}
        virtual ~LiveRenderButtonWidget() {}

        virtual void buildWidgetAttribute(FnAttribute::GroupBuilder& builder, const std::string& prefix) const;
        void setIconFilename(const std::string& iconFilename);

    private:
        std::string _iconFilename;
    };

    class LiveRenderUserInterfacePolicy
    {
    public:
        LiveRenderUserInterfacePolicy(const std::string& policy);
        virtual ~LiveRenderUserInterfacePolicy() {}

        virtual void buildPolicyAttribute(FnAttribute::GroupBuilder& builder, const std::string& prefix) const;

    private:
        std::string _policy;
    };

    // Concrete implementations for Live Render Control Modules
    class RenderConfigurationPolicyLRCM : public LiveRenderControlModule,
                                          public LiveRenderUserInterfacePolicy
    {
    public:
        RenderConfigurationPolicyLRCM(const std::string& name,
                                      const std::string& policy);

        virtual ~RenderConfigurationPolicyLRCM() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;
    };

    class RecookLRCM : public LiveRenderControlModule
    {
    public:
        RecookLRCM(const std::string& name,
                   LiveRenderRecookTiming recookTiming);

        virtual ~RecookLRCM() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const = 0;

    protected:
        LiveRenderRecookTiming _recookTiming;
    };

    class RecookPolicyLRCM : public RecookLRCM, public LiveRenderUserInterfacePolicy
    {
    public:
        RecookPolicyLRCM(const std::string& name,
                         LiveRenderRecookTiming recookTiming,
                         const std::string& policy);

        virtual ~RecookPolicyLRCM() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;
    };

    class RecookButtonLRCM : public RecookLRCM, public LiveRenderButtonWidget
    {
    public:
        RecookButtonLRCM(const std::string& name,
                         LiveRenderRecookTiming recookTiming,
                         const std::string& updateType);

        virtual ~RecookButtonLRCM() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

    protected:
        std::string _updateType;
    };

    class FilteredRecookButtonLRCM : public RecookButtonLRCM
    {
    public:
        FilteredRecookButtonLRCM(const std::string& name,
                                 LiveRenderRecookTiming recookTiming);

        virtual ~FilteredRecookButtonLRCM() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        void setLiveRenderFilterMode(LiveRenderFilterMode filterMode);

        void addLiveRenderFilter(const std::string& name,
                                 const std::string& updateType,
                                 const std::string& sgLocationType,
                                 const std::string& sgLocationOrigin,
                                 SceneGraphTraversalMode sgTraversalMode);

        void addAttributeToRenderFilter(const std::string& name,
                                        const std::string& attributeName);

    protected:
        LiveRenderFilterMode                    _filterMode;
        std::map<std::string, LiveRenderFilter> _filters;
    };

    class CommandButtonLRCM : public LiveRenderControlModule, public LiveRenderButtonWidget
    {
    public:
        CommandButtonLRCM(const std::string& name,
                          const std::string& command);

        virtual ~CommandButtonLRCM() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

    protected:
        std::string _command;
    };

/**
 * @}
 */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
