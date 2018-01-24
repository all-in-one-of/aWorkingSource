// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnRendererInfo/suite/FnRendererInfoSuite.h>

#include <FnRendererInfo/plugin/RenderMethod.h>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{

RenderMethod::RenderMethod(const std::string& name, const std::string& label) :
    _name(name),
    _label(label),
    _debugOutputFileType("txt"),
    _createCatalogItem(true),
    _reportRenderMessages(true),
    _registerRender(true),
    _debugOutputSupported(false),
    _sceneGraphDebugOutputSupported(false),
    _visible(true)
{
}

void RenderMethod::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    builder.set("name", FnAttribute::StringAttribute(_name));
    builder.set("label", FnAttribute::StringAttribute(_label));

    builder.set("createCatalogItem", FnAttribute::IntAttribute(_createCatalogItem));
    builder.set("reportRenderMessages", FnAttribute::IntAttribute(_reportRenderMessages));
    builder.set("registerRender", FnAttribute::IntAttribute(_registerRender));
    builder.set("debugOutputSupported", FnAttribute::IntAttribute(_debugOutputSupported));
    builder.set("sceneGraphDebugOutputSupported", FnAttribute::IntAttribute(_sceneGraphDebugOutputSupported));
    builder.set("debugOutputFileType", FnAttribute::StringAttribute(_debugOutputFileType));
    builder.set("isVisible", FnAttribute::IntAttribute(_visible));
}

void RenderMethod::setName(const std::string& name)
{
    _name = name;
}

std::string RenderMethod::getName() const
{
    return _name;
}

void RenderMethod::setLabel(const std::string& label)
{
    _label = label;
}

std::string RenderMethod::getLabel() const
{
    return _label;
}

void RenderMethod::setCreateCatalogItem(bool createCatalogItem)
{
    _createCatalogItem = createCatalogItem;
}

bool RenderMethod::isCreateCatalogItem() const
{
    return _createCatalogItem;
}

void RenderMethod::setReportRenderMessages(bool reportRenderMessages)
{
    _reportRenderMessages = reportRenderMessages;
}

bool RenderMethod::isReportRenderMessages() const
{
    return _reportRenderMessages;
}

void RenderMethod::setRegisterRender(bool registerRender)
{
    _registerRender = registerRender;
}

bool RenderMethod::isRegisterRender() const
{
    return _registerRender;
}

void RenderMethod::setDebugOutputSupported(bool debugOutputSupported)
{
    _debugOutputSupported = debugOutputSupported;
}

bool RenderMethod::isDebugOutputSupported() const
{
    return _debugOutputSupported;
}

void RenderMethod::setSceneGraphDebugOutputSupported(bool sceneGraphDebugOutputSupported)
{
    _sceneGraphDebugOutputSupported = sceneGraphDebugOutputSupported;
}

bool RenderMethod::isSceneGraphDebugOutputSupported() const
{
    return _sceneGraphDebugOutputSupported;
}

void RenderMethod::setDebugOutputFileType(const std::string& debugOutputFileType)
{
    _debugOutputFileType = debugOutputFileType;
}

std::string RenderMethod::getDebugOutputFileType() const
{
    return _debugOutputFileType;
}

void RenderMethod::setVisible(bool visible)
{
    _visible = visible;
}
bool RenderMethod::isVisible() const
{
    return _visible;
}

DiskRenderMethod::DiskRenderMethod() :
    RenderMethod(DiskRenderMethod::kDefaultName, DiskRenderMethod::kDefaultLabel),
    _allowWaitingForRenderCompletion(true)
{
}

DiskRenderMethod::DiskRenderMethod(const std::string& name, const std::string& label) :
    RenderMethod(name, label),
    _allowWaitingForRenderCompletion(true)
{
}

void DiskRenderMethod::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RenderMethod::buildAttribute(builder);
    builder.set("type", FnAttribute::StringAttribute(getType()));
    builder.set("allowWaitingForRenderCompletion", FnAttribute::IntAttribute(_allowWaitingForRenderCompletion));
}

void DiskRenderMethod::setAllowWaitingForRenderCompletion(bool allowWaitingForRenderCompletion)
{
    _allowWaitingForRenderCompletion = allowWaitingForRenderCompletion;
}

bool DiskRenderMethod::isAllowWaitingForRenderCompletion() const
{
    return _allowWaitingForRenderCompletion;
}

const char* DiskRenderMethod::kType = kFnRenderMethodTypeDiskRender;
const char* DiskRenderMethod::kDefaultLabel = "Disk Render";
const char* DiskRenderMethod::kDefaultName = "diskRender";
const char* DiskRenderMethod::kBatchName = "batchRender";

PreviewRenderMethod::PreviewRenderMethod() : RenderMethod(PreviewRenderMethod::kDefaultName, PreviewRenderMethod::kDefaultLabel),
        _allowConcurrentRenders(false)
{
}

PreviewRenderMethod::PreviewRenderMethod(const std::string& name, const std::string& label,
                                         bool allowConcurrentRenders) :
    RenderMethod(name, label),
    _allowConcurrentRenders(allowConcurrentRenders)
{
}

void PreviewRenderMethod::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RenderMethod::buildAttribute(builder);
    builder.set("type", FnAttribute::StringAttribute(getType()));
    builder.set("allowConcurrentRenders", FnAttribute::IntAttribute(_allowConcurrentRenders));
}

void PreviewRenderMethod::setAllowConcurrentRenders(bool allowConcurrentRenders)
{
    _allowConcurrentRenders = allowConcurrentRenders;
}

bool PreviewRenderMethod::isAllowConcurrentRenders() const
{
    return _allowConcurrentRenders;
}

const char* PreviewRenderMethod::kType = kFnRenderMethodTypePreviewRender;
const char* PreviewRenderMethod::kDefaultLabel = "Preview Render";
const char* PreviewRenderMethod::kDefaultName = "previewRender";

LiveRenderMethod::LiveRenderMethod() :
                RenderMethod(LiveRenderMethod::kDefaultName, LiveRenderMethod::kDefaultLabel)
{
}

LiveRenderMethod::LiveRenderMethod(const std::string& name, const std::string& label) :
    RenderMethod(name, label)
{
}

void LiveRenderMethod::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RenderMethod::buildAttribute(builder);
    builder.set("type", FnAttribute::StringAttribute(getType()));
}

const char* LiveRenderMethod::kType = kFnRenderMethodTypeLiveRender;
const char* LiveRenderMethod::kDefaultLabel = "Live Render";
const char* LiveRenderMethod::kDefaultName = "liveRender";

}
}
}
