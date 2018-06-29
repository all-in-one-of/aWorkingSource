// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/CopyAndConvertRenderAction.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

CopyAndConvertRenderAction::CopyAndConvertRenderAction(
        const std::string& renderTargetLocation,
        const std::string& tempRenderLocation,
        bool clampOutput,
        bool colorConvert,
        const std::string & computeStats,
        const RenderSettings::AttributeSettings& convertSettings) :
            CopyRenderAction(renderTargetLocation, tempRenderLocation),
            _clampOutput(clampOutput),
            _colorConvert(colorConvert),
            _computeStats(computeStats),
            _convertSettings(convertSettings),
            _offsetForOverscan(true)
{
}

void CopyAndConvertRenderAction::setAdditionalExrAttrs(
        const RenderSettings::AttributeSettings& additionalExrAttrs)
{
    _additionalExrAttrs = additionalExrAttrs;
}

void CopyAndConvertRenderAction::setOffsetForOverscan(bool value)
{
    _offsetForOverscan = value;
}

void CopyAndConvertRenderAction::buildAttribute(GroupBuilder& builder) const
{
    CopyRenderAction::buildAttribute(builder);
    builder.set("action", FnKat::StringAttribute("renderAndConvert"));
    builder.set("convert.clampFinalImage", FnKat::StringAttribute(_clampOutput ? "true" : "false"));
    builder.set("convert.colorConvertImage", FnKat::StringAttribute(_colorConvert ? "true" : "false"));
    builder.set("convert.computeStats", FnKat::StringAttribute(_computeStats));

    GroupBuilder convertSettingsBuilder;
    RenderSettings::AttributeSettings::const_iterator convertIt;
    for(convertIt = _convertSettings.begin(); convertIt != _convertSettings.end(); ++convertIt)
    {
        convertSettingsBuilder.set((*convertIt).first, (*convertIt).second);
    }
    builder.set("convert.finalImageOptions", convertSettingsBuilder.build());

    GroupBuilder additionalExrBuilder;
    RenderSettings::AttributeSettings::const_iterator exrIt;
    for(exrIt = _additionalExrAttrs.begin(); exrIt != _additionalExrAttrs.end(); ++exrIt)
    {
        additionalExrBuilder.set((*exrIt).first, (*exrIt).second);
    }
    builder.set("convert.additionalExrAttrs", additionalExrBuilder.build());
    builder.set("convert.offsetForOverscan", FnKat::IntAttribute(_offsetForOverscan));
}

}
}
}


