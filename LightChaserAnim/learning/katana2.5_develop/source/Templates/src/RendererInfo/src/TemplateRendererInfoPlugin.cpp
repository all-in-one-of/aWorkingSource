// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "TemplateRendererInfoPlugin.h"

#include <algorithm>

namespace CompanyName
{
namespace Katana
{

TemplateRendererInfoPlugin::TemplateRendererInfoPlugin()
{
}

TemplateRendererInfoPlugin::~TemplateRendererInfoPlugin()
{
}

void TemplateRendererInfoPlugin::fillRenderMethods(
    std::vector<Foundry::Katana::RendererInfo::RenderMethod*>& renderMethods) const
{
    renderMethods.clear();

    // e.g.
    // Support Preview, Disk and Live Render Methods:

    FnKat::RendererInfo::RenderMethod *diskRenderMethod;
    FnKat::RendererInfo::RenderMethod *previewRenderMethod;
    FnKat::RendererInfo::RenderMethod *liveRenderMethod;

    diskRenderMethod = new FnKat::RendererInfo::DiskRenderMethod();
    previewRenderMethod = new FnKat::RendererInfo::PreviewRenderMethod();
    liveRenderMethod = new FnKat::RendererInfo::LiveRenderMethod();

    renderMethods.push_back(diskRenderMethod);
    renderMethods.push_back(previewRenderMethod);
    renderMethods.push_back(liveRenderMethod);
}

void TemplateRendererInfoPlugin::fillRendererObjectNames(
    std::vector<std::string>& rendererObjectNames,
    const std::string& type,
    const std::vector<std::string>& typeTags) const
{
    rendererObjectNames.clear();

    // TODO: Fill out the names of objects (e.g. shaders) that exist
    // for each of the tags in typeTags.

    // e.g.
    // if (type == kFnRendererObjectTypeShader)
    // {
    //     if (std::find(typeTags.begin(), typeTags.end(), "surface") != typeTags.end())
    //     {
    //         rendererObjectNames.push_back("myWoodShader");
    //     }
    // }
}

void TemplateRendererInfoPlugin::fillRendererObjectTypes(
    std::vector<std::string>& renderObjectTypes,
    const std::string& type) const
{
    renderObjectTypes.clear();

    // TODO: Fill out the full list of object types your renderer
    // supports, of the specified parent 'type'.

    // e.g.
    // if (type == kFnRendererObjectTypeShader)
    // {
    //     renderObjectTypes.push_back("surface");
    //     renderObjectTypes.push_back("light");
    // }
    // else if (type == kFnRendererObjectTypeRenderOutput)
    // {
    //     renderObjectTypes.push_back(kFnRendererOutputTypeColor);
    //     renderObjectTypes.push_back(kFnRendererOutputTypeRaw);
    // }
    // else if (type == kFnRendererObjectTypeOutputChannelCustomParam)
    // {
    //     renderObjectTypes.push_back("opacity");
    // }
}

void TemplateRendererInfoPlugin::configureBatchRenderMethod(
    Foundry::Katana::RendererInfo::DiskRenderMethod& batchRenderMethod) const
{
    // e.g.
    // batchRenderMethod.setDebugOutputFileType("txt");
}

std::string TemplateRendererInfoPlugin::getRegisteredRendererName() const
{
    return "template";
}

std::string TemplateRendererInfoPlugin::getRegisteredRendererVersion() const
{
    return "0.0v0";
}

std::string TemplateRendererInfoPlugin::getRendererObjectDefaultType(
    const std::string& type) const
{
    // e.g.
    // if (type == kFnRendererObjectTypeRenderOutput)
    // {
    //     return kFnRendererOutputTypeColor;
    // }

    // Default:
    return FnKat::RendererInfo::RendererInfoBase::getRendererObjectDefaultType(type);
}

bool TemplateRendererInfoPlugin::isPresetLocalFileNeeded(
    const std::string &outputType) const
{
    // e.g.
    // if (outputType == kSomeOutputTypeWhichRequiresPresetLocalFile)
    // {
    //     return true;
    // }

    // Default:
    return FnKat::RendererInfo::RendererInfoBase::isPresetLocalFileNeeded(outputType);
}

bool TemplateRendererInfoPlugin::isNodeTypeSupported(
    const std::string &nodeType) const
{
    // e.g.
    // if (nodeType == "ShadingNode" || nodeType == "OutputChannelDefine")
    // {
    //     return true;
    // }

    // Default:
    return FnKat::RendererInfo::RendererInfoBase::isNodeTypeSupported(nodeType);
}

bool TemplateRendererInfoPlugin::isPolymeshFacesetSplittingEnabled() const
{
    // If true is returned, Katana will split geometry automatically before
    // calling the Render plug-in.

    return FnKat::RendererInfo::RendererInfoBase::isPolymeshFacesetSplittingEnabled();
}

void TemplateRendererInfoPlugin::fillShaderInputNames(
    std::vector<std::string>& shaderInputNames,
    const std::string& shaderName) const
{
    shaderInputNames.clear();

    // TODO: Fill out the names of inputs supported by the
    // shader specified in 'shaderName'. These are used to
    // connect shading nodes.

    // e.g.
    // if (shaderName == "myWoodShader")
    // {
    //     shaderInputNames.push_back("colour");
    // }
}

void TemplateRendererInfoPlugin::fillShaderInputTags(
    std::vector<std::string>& shaderInputTags,
    const std::string& shaderName,
    const std::string& inputName) const
{
    shaderInputTags.clear();

    // TODO: Fill out the type tags of inputs supported by the
    // shader specified in 'shaderName'. These are used to
    // constrain connections between shading nodes.
    //
    // Type tags are specified using a simple Python-like syntax
    // that supports boolean operations.

    // e.g.
    // if (shaderName == "myWoodShader")
    // {
    //     shaderInputTags.push_back("rgba or float");
    // }
}

void TemplateRendererInfoPlugin::fillShaderOutputNames(
    std::vector<std::string>& shaderOutputNames,
    const std::string& shaderName) const
{
    shaderOutputNames.clear();

    // TODO: Fill out the names of inputs supported by the
    // shader specified in 'shaderName'. These are used to
    // connect shading nodes.

    // e.g.
    // if (shaderName == "myWoodShader")
    // {
    //     shaderOutputNames.push_back("out");
    // }
}

void TemplateRendererInfoPlugin::fillShaderOutputTags(
    std::vector<std::string>& shaderOutputTags,
    const std::string& shaderName,
    const std::string& outputName) const
{
    shaderOutputTags.clear();

    // TODO: Fill out the type tags of outputs supported by the
    // shader specified in 'shaderName'. These are used to
    // constrain connections between shading nodes.
    //
    // Type tags are specified using a simple Python-like syntax
    // that supports boolean operations.

    // e.g.
    // if (shaderName == "myWoodShader")
    // {
    //     shaderOutputTags.push_back("rgba or float");
    // }
}

void TemplateRendererInfoPlugin::fillRendererShaderTypeTags(
    std::vector<std::string>& shaderTypeTags,
    const std::string& shaderType) const
{
    shaderTypeTags.clear();

    // TODO: Fill out any type tags that are compatible with the
    // shader specified in 'shaderType'.  These are used to
    // constrain the connection of shading nodes to Network
    // Material node input terminals.

    // e.g.
    // Allowing only float and rgba to connect to a surface
    // terminal on a NetworkMaterial node:
    //
    // if (shaderType == "surface")
    // {
    //     shaderTypeTags.push_back("float or rgba");
    // }
}

std::string TemplateRendererInfoPlugin::getRendererCoshaderType() const
{
    // e.g.
    // return "class";

    // Default:
    return FnKat::RendererInfo::RendererInfoBase::getRendererCoshaderType();
}

bool TemplateRendererInfoPlugin::buildRendererObjectInfo(
    FnKat::GroupBuilder& rendererObjectInfo,
    const std::string& name,
    const std::string& type,
    const FnKat::GroupAttribute inputAttr /*= 0x0*/) const
{
    // Provide attribute data for objects that are advertised in
    // fillRendererObjectNames/fillRendererObjectTypes.

    // e.g.
    //
    // if(type == kFnRendererObjectTypeShader)
    // {
    //     if (name == "myWoodShader")
    //     {
    //         _buildAttributesForWoodShader(rendererObjectInfo, inputAttr);
    //     }
    // }
    // else if(type == kFnRendererObjectTypeRenderOutput)
    // {
    //     if (name == kFnRendererOutputTypeColor)
    //     {
    //         _buildAttributesForColorOutput(rendererObjectInfo, inputAttr);
    //     }
    // }
    // etc

    return false;
}

void TemplateRendererInfoPlugin::flushCaches()
{

}

DEFINE_RENDERERINFO_PLUGIN(TemplateRendererInfoPlugin)

}
}

void registerPlugins()
{
    REGISTER_PLUGIN(CompanyName::Katana::TemplateRendererInfoPlugin, "TemplateRendererInfoPlugin", 0, 1);
}
