// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnRendererInfo/plugin/LiveRenderControlModule.h>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{

LiveRenderControlModule::LiveRenderControlModule(const std::string& name)
    : _name(name)
{
}

void LiveRenderControlModule::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    builder.set(getModulePrefix() + ".name", FnAttribute::StringAttribute(_name));

    if(!_label.empty())
        builder.set(getModulePrefix() + ".label", FnAttribute::StringAttribute(_label));
}

void LiveRenderControlModule::setLabel(const std::string& label)
{
    _label = label;
}

std::string LiveRenderControlModule::getModulePrefix() const
{
    return "modules." + _name;
}

// LiveRenderButtonWidget
void LiveRenderButtonWidget::setIconFilename(const std::string &iconFilename)
{
    _iconFilename = iconFilename;
}

void LiveRenderButtonWidget::buildWidgetAttribute(FnAttribute::GroupBuilder& builder, const std::string& prefix) const
{
    builder.set(prefix + ".type", FnAttribute::StringAttribute("button"));

    if(!_iconFilename.empty())
        builder.set(prefix + ".icon", FnAttribute::StringAttribute(_iconFilename));
}

// LiveRenderUserInterfacePolicy
LiveRenderUserInterfacePolicy::LiveRenderUserInterfacePolicy(const std::string& policy)
    : _policy(policy)
{
}

void LiveRenderUserInterfacePolicy::buildPolicyAttribute(FnAttribute::GroupBuilder& builder, const std::string& prefix) const
{
    builder.set(prefix + ".type", FnAttribute::StringAttribute("policy"));
    builder.set(prefix + ".policy", FnAttribute::StringAttribute(_policy));
}

// RenderConfigurationPolicyLRCM
RenderConfigurationPolicyLRCM::RenderConfigurationPolicyLRCM(const std::string& name,
                                                             const std::string& policy)
    : LiveRenderControlModule(name),
      LiveRenderUserInterfacePolicy(policy)
{
}

void RenderConfigurationPolicyLRCM::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    LiveRenderControlModule::buildAttribute(builder);
    LiveRenderUserInterfacePolicy::buildPolicyAttribute(builder, getModulePrefix());
    builder.set(getModulePrefix() + ".policyType", FnAttribute::StringAttribute("startConfiguration"));
}

// RecookLRCM
RecookLRCM::RecookLRCM(const std::string& name,
                       LiveRenderRecookTiming recookTiming)
    : LiveRenderControlModule(name),
      _recookTiming(recookTiming)
{
}

void RecookLRCM::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    LiveRenderControlModule::buildAttribute(builder);

    if(_recookTiming == kLiveRenderRecookTiming_BeforeUpdate)
        builder.set(getModulePrefix() + ".recookScene", FnAttribute::StringAttribute("beforeUpdate"));
    else if(_recookTiming == kLiveRenderRecookTiming_AfterUpdate)
        builder.set(getModulePrefix() + ".recookScene", FnAttribute::StringAttribute("afterUpdate"));
}

// Add name comment header thing
RecookButtonLRCM::RecookButtonLRCM(const std::string& name,
                                   LiveRenderRecookTiming recookTiming,
                                   const std::string& updateType)
    : RecookLRCM(name, recookTiming),
      _updateType(updateType)
{
}

void RecookButtonLRCM::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RecookLRCM::buildAttribute(builder);
    LiveRenderButtonWidget::buildWidgetAttribute(builder, getModulePrefix());

    if(!_updateType.empty())
        builder.set(getModulePrefix() + ".attribute.type", FnAttribute::StringAttribute(_updateType));
}

// FilteredRecookButtonLRCM
FilteredRecookButtonLRCM::FilteredRecookButtonLRCM(const std::string& name,
                                                   LiveRenderRecookTiming recookTiming)
    : RecookButtonLRCM(name, recookTiming, ""),
      _filterMode(kLiveRenderFilterMode_Append)
{
}

void FilteredRecookButtonLRCM::setLiveRenderFilterMode(LiveRenderFilterMode filterMode)
{
    _filterMode = filterMode;
}

void FilteredRecookButtonLRCM::addLiveRenderFilter(const std::string& name,
                                                   const std::string& updateType,
                                                   const std::string& sgLocationType,
                                                   const std::string& sgLocationOrigin,
                                                   SceneGraphTraversalMode sgTraversalMode)
{
    LiveRenderFilter filter(name, updateType, sgLocationType,
                            sgLocationOrigin, sgTraversalMode);
    _filters[name] = filter;
}

void FilteredRecookButtonLRCM::addAttributeToRenderFilter(const std::string& name,
                                                          const std::string& attributeName)
{
    _filters[name].addAttribute(attributeName);
}

void FilteredRecookButtonLRCM::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RecookButtonLRCM::buildAttribute(builder);

    if(_filterMode == kLiveRenderFilterMode_Replace)
        builder.set(getModulePrefix() + ".filterMode", FnAttribute::StringAttribute("replace"));

    if(_filters.size() > 0)
    {
        std::map<std::string, LiveRenderFilter>::const_iterator filtersIt;
        for(filtersIt = _filters.begin(); filtersIt != _filters.end(); ++filtersIt)
        {
            std::string filtersPrefix = getModulePrefix() + ".recookFilters." + filtersIt->first + ".";
            filtersIt->second.buildAttribute(builder, filtersPrefix);
        }
    }
}

// RecookPolicyLRCM
RecookPolicyLRCM::RecookPolicyLRCM(const std::string& name,
                                   LiveRenderRecookTiming recookTiming,
                                   const std::string& policy)
    : RecookLRCM(name, recookTiming),
      LiveRenderUserInterfacePolicy(policy)
{
}

void RecookPolicyLRCM::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RecookLRCM::buildAttribute(builder);
    LiveRenderUserInterfacePolicy::buildPolicyAttribute(builder, getModulePrefix());
}

// CommandButtonLRCM
CommandButtonLRCM::CommandButtonLRCM(const std::string& name,
                                     const std::string& command)
    : LiveRenderControlModule(name),
      _command(command)
{
}

void CommandButtonLRCM::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    LiveRenderControlModule::buildAttribute(builder);
    LiveRenderButtonWidget::buildWidgetAttribute(builder, getModulePrefix());

    builder.set(getModulePrefix() + ".command", FnAttribute::StringAttribute(_command));
}

}
}
}
