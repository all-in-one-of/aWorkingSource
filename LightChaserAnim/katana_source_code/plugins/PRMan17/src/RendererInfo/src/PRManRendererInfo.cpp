// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX_PATHS_BEFORE_ERROR 1024

#include <PRManRendererInfo.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#ifndef _WIN32
#include <libgen.h>
#else
#include <iterator>
#include <windowshelpers.h>
#endif
#include <cassert>
#include <algorithm>

#include <iostream>
#include <pystring/pystring.h>

#include <slo.h>
#include <rx.h>

#include <FnAttribute/FnDataBuilder.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnLogging/FnLogging.h>


#include <FnGeolib/util/Mutex.h>
namespace FnGeolibUtil = FNGEOLIBUTIL_NAMESPACE;

// Keep the logger info internal to this file
namespace
{
FnLogSetup("PRManRendererInfo");
}


namespace // anonymous
{
    // Every use of g_cache should acquire read or write lock, as appropriate.
    FnKat::RendererInfo::ShaderInfoCache<PRManRendererInfo::DirFilePair> g_cache;
    FnGeolibUtil::ReadWriteLock g_readWriteLock;
}

// This wraps a std::set<std::string>
// It is for keeping track of directory paths
// that we have already visited.
//
// addPath will fail and return false if TrackPaths
// already contains 1024 paths.
//
// std::map could be replaced with tr1::unordered_set
// to be quicker.
// Or if the directory tree under
// RMAN_SHADERPATH is quite large a custom allocator could be written to
// avoid allocating for every set insertion.

class TrackPaths
{
    typedef std::set<std::string> StringSet;

    public:
        bool addPath(const char * path);

        // hasPath is non const because we populate the internal buffer m_path
        bool hasPath(const char * path) const;

    private:
        StringSet m_paths;
};

bool TrackPaths::addPath(const char * path)
{
    // hasPath should be false
    // before any call to addPath.

    assert(!hasPath(path));

    // Issue a warning if we are traversing too deeply

    if(m_paths.size() >= MAX_PATHS_BEFORE_ERROR)
    {
        return false;
    }

    // Explicitly create our temporary std:string

    std::string temporaryPath = path;
    m_paths.insert(temporaryPath);

    return true;
}

bool TrackPaths::hasPath(const char * path) const
{
    // Explicitly create our temporary std:string

    std::string temporaryPath = path;
    StringSet::iterator it = m_paths.find(temporaryPath);
    if(it == m_paths.end())
    {
        return false;
    }
    return true;
}

PRManRendererInfo::PRManRendererInfo()
{
}

PRManRendererInfo::~PRManRendererInfo()
{
}

FnKat::RendererInfo::RendererInfoBase* PRManRendererInfo::create()
{
    return new PRManRendererInfo();
}

void PRManRendererInfo::configureBatchRenderMethod(Foundry::Katana::RendererInfo::DiskRenderMethod& batchRenderMethod) const
{
    batchRenderMethod.setDebugOutputSupported(true);
    batchRenderMethod.setDebugOutputFileType("rib");
}

void PRManRendererInfo::fillRenderMethods(std::vector<Foundry::Katana::RendererInfo::RenderMethod*>& renderMethods) const
{
    FnKat::RendererInfo::DiskRenderMethod*    diskRenderMethod;
    FnKat::RendererInfo::PreviewRenderMethod* previewRenderMethod;
    FnKat::RendererInfo::LiveRenderMethod*    liveRenderMethod;

    diskRenderMethod = new FnKat::RendererInfo::DiskRenderMethod();
    previewRenderMethod = new FnKat::RendererInfo::PreviewRenderMethod();
    liveRenderMethod = new FnKat::RendererInfo::LiveRenderMethod();

    previewRenderMethod->setDebugOutputSupported(true);
    previewRenderMethod->setSceneGraphDebugOutputSupported(true);
    previewRenderMethod->setDebugOutputFileType("rib");

    renderMethods.push_back(diskRenderMethod);
    renderMethods.push_back(previewRenderMethod);
    renderMethods.push_back(liveRenderMethod);
}

void  PRManRendererInfo::fillLiveRenderTerminalOps(
        FnKat::RendererInfo::RendererInfoBase::OpDefinitionQueue& terminalOps,
        const FnAttribute::GroupAttribute& stateArgs) const
{
    {
        // Add a LocalizeAttribute to ensure that renderSettings is always sent
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("attributeName", FnAttribute::StringAttribute("renderSettings.cameraName"));
        opArgs.set("CEL", FnAttribute::StringAttribute("/root"));
        terminalOps.push_back(std::make_pair("LocalizeAttribute", opArgs.build()));
    }

    {
        FnAttribute::GroupBuilder opArgs;
        terminalOps.push_back(std::make_pair("LiveAttribute", opArgs.build()));
    }

    {
        // Add a material hash since so that both light material and xforms are sent at once
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("light"));
        opArgs.set("addMaterialHash", FnAttribute::IntAttribute(1));
        terminalOps.push_back(std::make_pair("LocalizeLiveAttributeXform", opArgs.build()));
    }

    {
        // Add a material hash since so that both light material and xforms are sent at once
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("light"));
        opArgs.set("addMaterialHash", FnAttribute::IntAttribute(1));
        terminalOps.push_back(std::make_pair("LocalizeXform", opArgs.build()));
    }

    {
        // Watch for changes to renderer specific Global Statements
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("globals"));
        opArgs.set("attributeNames", FnAttribute::StringAttribute("prmanGlobalStatements"));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes any locations that are cameras
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("camera"));
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));

        std::string attributes[] = {"xform", "geometry"};
        opArgs.set("attributeNames", FnAttribute::StringAttribute(attributes, 2, 1));

        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes any locations that are lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("light"));
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));
        opArgs.set("collectUpstream", FnAttribute::IntAttribute(1));

        std::string attributes[] = {"xform", "material", "geometry", "mute", "solo"};
        opArgs.set("attributeNames", FnAttribute::StringAttribute(attributes, 5, 1));

        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes to light list attributes on any locations that are not lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute(""));
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("lightLink"));
        opArgs.set("exclusions", FnAttribute::StringAttribute("light"));

        terminalOps.push_back(std::make_pair("LocalizeLightListLiveRenderFilter",
                opArgs.build()));
    }

    {
        // Watch for changes to material attributes on any locations that are not lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute(""));
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("geoMaterial"));
        opArgs.set("exclusions", FnAttribute::StringAttribute("light"));
        opArgs.set("collectUpstream", FnAttribute::IntAttribute(1));

        std::string attributes[] = {"info", "material", "geometry.faces",};
        opArgs.set("attributeNames", FnAttribute::StringAttribute(attributes, 3, 1));

        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch /root for changes to renderSettings
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("renderSettings"));
        opArgs.set("attributeNames", FnAttribute::StringAttribute("renderSettings"));

        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for coordinate system updates
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("coordinateSystem"));
        opArgs.set("attributeNames", FnAttribute::StringAttribute("xform"));
        std::string exclusions[] = {"light", "camera"};
        opArgs.set("exclusions", FnAttribute::StringAttribute(exclusions, 2, 1));

        terminalOps.push_back(std::make_pair("CoordinateSystemLiveRenderFilter", opArgs.build()));
    }

}

void PRManRendererInfo::fillRendererObjectNames(std::vector<std::string>& rendererObjectNames,
                                                const std::string& type,
                                                const std::vector<std::string>& typeTags) const
{
    if (type == kFnRendererObjectTypeShader)
    {
        _cacheShaderInfo();
        FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);

        for (CacheIterator iter = g_cache.begin(); iter!=g_cache.end(); ++iter)
        {
            std::set<std::string> shaderTypeTags;
            getTypeTagsUsingNameFilters((*iter).first, shaderTypeTags);

            // if name filters don't tell us the typeTags, load from shader
            if (shaderTypeTags.empty())
            {
                _fillTypeTagsFromSloFile(shaderTypeTags, (*iter).second.first, (*iter).first);
            }

            if (findTypeTagsInObjectTypeTags(typeTags, shaderTypeTags))
            {
                rendererObjectNames.push_back((*iter).first);
            }
        }

        const std::vector<std::string> &additionalLocations =
            getAdditionalObjectLocations(kFnRendererObjectTypeShader);
        for (std::vector<std::string>::const_iterator iter=additionalLocations.begin();
                iter!=additionalLocations.end(); ++iter)
        {
            const std::string &shaderLocation = (*iter);
            std::set<std::string> shaderTypeTags;
            getTypeTagsUsingNameFilters(shaderLocation, shaderTypeTags);

            // if name filters don't tell us the typeTags, load from shader
            if (shaderTypeTags.empty())
            {
                DirFilePair dirFilePair;
                std::string objectName;
                bool success = _getDirAndFileFromLocation(dirFilePair, objectName, shaderLocation);
                if (!success) continue;

                _fillTypeTagsFromSloFile(shaderTypeTags, dirFilePair.first, objectName);
            }

            if (findTypeTagsInObjectTypeTags(typeTags, shaderTypeTags))
            {
                rendererObjectNames.push_back(shaderLocation);
            }
        }
    }
}

void PRManRendererInfo::fillRendererObjectTypes(std::vector<std::string>& renderObjectTypes,
                                                const std::string& type) const
{
    renderObjectTypes.clear();

    if (type == kFnRendererObjectTypeShader)
    {
        renderObjectTypes.push_back("surface");
        renderObjectTypes.push_back("displacement");
        renderObjectTypes.push_back("atmosphere");
        renderObjectTypes.push_back("interior");
        renderObjectTypes.push_back("exterior");
        renderObjectTypes.push_back("light");
    }
    else if (type == kFnRendererObjectTypeRenderOutput)
    {
        renderObjectTypes.push_back(kFnRendererOutputTypeColor);
        renderObjectTypes.push_back(kFnRendererOutputTypeDeep);
        renderObjectTypes.push_back(kFnRendererOutputTypeShadow);
        renderObjectTypes.push_back(kFnRendererOutputTypeRaw);
        renderObjectTypes.push_back(OutputTypePtc);
        renderObjectTypes.push_back(kFnRendererOutputTypeScript);
        renderObjectTypes.push_back(kFnRendererOutputTypePreScript);
        renderObjectTypes.push_back(kFnRendererOutputTypeMerge);
        renderObjectTypes.push_back(kFnRendererOutputTypeForceNone);
    }
    else if (type == kFnRendererObjectTypeOutputChannelCustomParam)
    {
        renderObjectTypes.push_back("opacity");
        renderObjectTypes.push_back("remap");
        renderObjectTypes.push_back("interpretation");
        renderObjectTypes.push_back("quantize");
        renderObjectTypes.push_back("matte");
        renderObjectTypes.push_back("fill");
        renderObjectTypes.push_back("source");
        renderObjectTypes.push_back("filter");
        renderObjectTypes.push_back("filterwidth");
        renderObjectTypes.push_back("exposure");
        renderObjectTypes.push_back("dither");
    }
}

void PRManRendererInfo::fillRendererShaderTypeTags(std::vector<std::string>& shaderTypeTags,
                                                   const std::string& shaderType) const
{
    // The tag will be equivalent to the actual shader type, except for the
    // "coshader" case, for which no tags will be added to allow any shading
    // node to be connected to it.
    shaderTypeTags.clear();

    if (shaderType != "coshader")
    {
        shaderTypeTags.push_back(shaderType);
    }
}

std::string PRManRendererInfo::getRendererCoshaderType() const
{
    return "class";
}

std::string PRManRendererInfo::getRegisteredRendererName() const
{
    return "prman";
}

std::string PRManRendererInfo::getRegisteredRendererVersion() const
{
    char* versionstring;
    RxInfoType_t type;
    int count;
    RxRendererInfo( "versionstring", &versionstring, sizeof(char*), &type, &count );
    return versionstring;
}

bool PRManRendererInfo::isPresetLocalFileNeeded(const std::string &outputType) const
{
    if (outputType == OutputTypePtc)
    {
        return true;
    }

    return FnKat::RendererInfo::RendererInfoBase::isPresetLocalFileNeeded(outputType);
}

bool PRManRendererInfo::isPolymeshFacesetSplittingEnabled() const
{
    return true;
}

bool PRManRendererInfo::isNodeTypeSupported(const std::string &nodeType) const
{
    if (nodeType == "ShadingNode" || nodeType == "OutputChannelDefine")
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PRManRendererInfo::fillShaderInputNames(std::vector<std::string>& shaderInputNames,
                                             const std::string& shaderName) const
{
    // Return only the shader parameters of the type "shader".
    shaderInputNames.clear();

    // Get the shader info GroupAttribute and then get the sub-attribute:
    // "param.[paramName].type to check if it is of the type "shader".
    FnAttribute::GroupAttribute dummy;
    FnAttribute::GroupBuilder gb;
    if (!buildRendererObjectInfo(gb, shaderName, kFnRendererObjectTypeShader, dummy))
    {
        return;
    }

    FnAttribute::GroupAttribute shaderInfoAttr = gb.build();
    if (!shaderInfoAttr.isValid())
    {
        return;
    }

    FnAttribute::GroupAttribute paramsAttr = shaderInfoAttr.getChildByName("param");
    if (!paramsAttr.isValid())
    {
        return;
    }

    for(int i = 0 ; i < paramsAttr.getNumberOfChildren(); ++i)
    {

        FnAttribute::GroupAttribute paramAttr = paramsAttr.getChildByIndex(i);
        if (!paramAttr.isValid())
        {
            continue;
        }

        FnAttribute::IntAttribute typeAttr = paramAttr.getChildByName("type");
        if (!typeAttr.isValid())
        {
            continue;
        }

        if (typeAttr.getValue() == kFnRendererObjectValueTypeShader)
        {
            shaderInputNames.push_back(paramsAttr.getChildName(i));
        }
    }
}

void PRManRendererInfo::fillShaderInputTags(std::vector<std::string>& shaderInputTags,
                                            const std::string& shaderName,
                                            const std::string& inputName) const
{
    // The tags for the input ports will be defined only by the .args files,
    // which is taken care automatically by Katana.
    shaderInputTags.clear();
}

void PRManRendererInfo::fillShaderOutputNames(std::vector<std::string>& shaderOutputNames,
                                              const std::string& shaderName) const
{
    shaderOutputNames.clear();
    shaderOutputNames.push_back("out");
}

void PRManRendererInfo::fillShaderOutputTags(std::vector<std::string>& shaderOutputTags,
                                             const std::string& shaderName,
                                             const std::string& outputName) const
{

    // In order for the output tags to match the expected tags on the
    // NetworkMaterialNode's ports we will use fillRendererShaderTypeTags()
    // and pass it the shader type.
    _cacheShaderInfo();

    // Get the Slo file/dir
    DirFilePair shaderInfo;
    {
        FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);
        shaderInfo = g_cache.getShaderInfo(shaderName);
    }
    std::string dirName;
    std::string fileName;
    std::string objectName;

    if (shaderInfo.second.empty())
    {
        DirFilePair dirFilePair;
        bool success = _getDirAndFileFromLocation(dirFilePair,
                                                  objectName,
                                                  shaderName);

        if (!success) return;
        dirName = dirFilePair.first;
        fileName = dirFilePair.second;
    }
    else
    {
        dirName = shaderInfo.first;
        fileName = shaderInfo.second;
        objectName = shaderName;
    }

    // Get the tags associated with the shader (surface, light, class, etc.)
    std::set<std::string> typeTags;
    _fillTypeTagsFromSloFile(typeTags, dirName, objectName);

    // If it is a coshader then it type will contain "class", for which we want
    // to leave with no tags. The tags of a coshader should always be defined by
    // the .args files only.
    std::set<std::string>::iterator it = typeTags.find("class");
    if (it != typeTags.end())
    {
        typeTags.erase(it);
    }

    // Copy the typeTags set into the shaderOutputTags vector
    shaderOutputTags.clear();
    std::copy(typeTags.begin(), typeTags.end(), std::back_inserter(shaderOutputTags));
}

bool PRManRendererInfo::buildRendererObjectInfo(FnAttribute::GroupBuilder& rendererObjectInfo,
                                                const std::string& name,
                                                const std::string& type,
                                                const FnAttribute::GroupAttribute inputAttr) const
{
    if (type == kFnRendererObjectTypeOutputChannelCustomParam)
    {
        return _buildChannelCustomParam(rendererObjectInfo, name);
    }
    else if (type == kFnRendererObjectTypeOutputChannel)
    {
        return _buildOutputChannel(rendererObjectInfo, name);
    }
    else if (type == kFnRendererObjectTypeRenderOutput)
    {
        // Check for valid input attributes before using them
        if (!inputAttr.isValid())
        {
            // Invalid inputAttr
            const char warning_message[] = "Invalid input attribute passed to buildRendererObjectInfo";
            FnLogWarn(warning_message);

            return false;
        }

        return _buildRenderOutput(rendererObjectInfo, name, inputAttr);
    }
    else
    {
        // TODO For now, this holds all the items. We need to take the type into
        // consideration here as well.
        _cacheShaderInfo();
        DirFilePair shaderInfo;
        {
            FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);
            shaderInfo = g_cache.getShaderInfo(name);
        }
        std::string dirName;
        std::string fileName;
        std::string objectName;
        std::string location;

        if (shaderInfo.second.empty())
        {
            DirFilePair dirFilePair;
            bool success = _getDirAndFileFromLocation(dirFilePair, objectName, name);
            if (!success) return false;
            dirName = dirFilePair.first;
            fileName = dirFilePair.second;
            location = name;
        }
        else
        {
            dirName = shaderInfo.first;
            fileName = shaderInfo.second;
            objectName = name;
            location = dirName;
        }

        std::set<std::string> typeTags;
        _fillTypeTagsFromSloFile(typeTags, dirName, objectName);

        FnKat::Attribute containerHintsAttr;

        std::string fullPath = dirName + "/" + fileName;

        configureBasicRenderObjectInfo(rendererObjectInfo,
                                       kFnRendererObjectTypeShader,                                /* type      */
                                       std::vector<std::string>(typeTags.begin(), typeTags.end()), /* type tags */
                                       location,                                                   /* location  */
                                       fullPath,                                                   /* full path */
                                       kFnRendererObjectValueTypeUnknown,                          /* outputType (unsupported) */
                                       containerHintsAttr);

        _fillParamsFromSloFile(rendererObjectInfo, dirName, objectName);

        return true;
    }

    return false;
}

void PRManRendererInfo::_configureBasicRenderObjectInfo(FnAttribute::GroupBuilder &gb,
                                                  const std::string &name,
                                                  const std::string &type) const
{
    std::vector<std::string> typeTags;
    std::string location = name;
    std::string fullPath = "";
    FnKat::Attribute containerHintsAttr;

    configureBasicRenderObjectInfo(gb,
                                   type,                                                       /* type      */
                                   std::vector<std::string>(typeTags.begin(), typeTags.end()), /* type tags */
                                   location,                                                   /* location  */
                                   fullPath,                                                   /* full path */
                                   kFnRendererObjectValueTypeUnknown,                          /* outputType (unsupported) */
                                   containerHintsAttr);
}

bool PRManRendererInfo::_buildChannelCustomParam(FnAttribute::GroupBuilder &gb,
                                                 const std::string &name) const
{
    _configureBasicRenderObjectInfo(gb, name, kFnRendererObjectTypeOutputChannelCustomParam);

    std::string attrName;
    int arraySize = 0;
    int paramType;
    EnumPairVector enums;
    std::string helpStr;
    std::vector<std::string> attrBuilder;
    FnAttribute::GroupBuilder groupBuilder;

    // getRendererOutputChannelCustomType
    if (name == "opacity")
    {
        paramType = kFnRendererObjectValueTypeString;
        attrBuilder.push_back("");
        helpStr = "Specifies the name of a display channel whose value will be used "
                "to perform alpha compositing, or other transparency operations. "
                "(By default, the renderer will use Oi for these operations.) It "
                "will then be assumed that the shader performs premultiplication "
                "of the specified channels and that the channels are are shader "
                "output variables. The renderer will perform all subsequent "
                "compositing operations based on this assumption.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "remap")
    {
        paramType = kFnRendererObjectValueTypeFloat;
        attrBuilder.push_back("0");
        attrBuilder.push_back("0");
        attrBuilder.push_back("0");
        groupBuilder.set("tupleSize", FnAttribute::StringAttribute("3"));
    }
    else if (name == "interpretation")
    {
        paramType = kFnRendererObjectValueTypeString;
        attrBuilder.push_back("standard");
        groupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        groupBuilder.set("options", FnAttribute::StringAttribute("standard|alpha|*"));
        helpStr = "Specifies alternate meanings for the display channel. The "
                "default interpretation is \"standard\", which means that the "
                "value for the channel is either a known geometric quantity "
                "or a shader output variable. An alternate interpretation "
                "is \"alpha\". When used in conjunction with \"string opacity\", "
                "this means that the value for the channel will be a float "
                "quantity synthesized from the specified opacity channel "
                "(similar to how the built-in display channel \"a\" is "
                "synthesized from \"Oi\").";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "quantize")
    {
        paramType = kFnRendererObjectValueTypeInt;
        attrBuilder.push_back("0");
        attrBuilder.push_back("255");
        attrBuilder.push_back("0");
        attrBuilder.push_back("255");
        groupBuilder.set("tupleSize", FnAttribute::StringAttribute("4"));
        helpStr = "These four values (zeroval, oneval, minval, and maxval) "
                "control how the output display is quantized, in exactly the "
                "same way that RiQuantize works.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "matte")
    {
        paramType = kFnRendererObjectValueTypeInt;
        attrBuilder.push_back("1");
        groupBuilder.set("widget", FnAttribute::StringAttribute("boolean"));
        helpStr = "When set to No, this allows an AOV to entirely ignore "
                "Matte, thus forcing the AOV to show up for that object in "
                "the final image. (By default, \"matte\" [1] is in effect - "
                "the AOV responds to Matte.)";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "fill")
    {
        paramType = kFnRendererObjectValueTypeFloat;
        attrBuilder.push_back("1");
        attrBuilder.push_back("1");
        attrBuilder.push_back("1");
        groupBuilder.set("tupleSize", FnAttribute::StringAttribute("3"));
        groupBuilder.set("widget", FnAttribute::StringAttribute("color"));
        helpStr = "The fill value is used in conjunction with the special "
                "pixel filters min, max, average, zmin, or zmax. The single "
                "value required represents the \"fill\" value used for any "
                "pixel subsamples that miss geometry.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "source")
    {
        paramType = kFnRendererObjectValueTypeString;
        attrBuilder.push_back("");
        helpStr = "Specifies the known geometric quantity or shader output "
                "variable the renderer will use as a source of data in "
                "preference to the channel name (overriding the channel "
                "parameter). This allows the renderer to create multiple "
                "channels, each with unique names, that are copies of the "
                "same source data.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "filter")
    {
        paramType = kFnRendererObjectValueTypeString;
        attrBuilder.push_back("box");
        groupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        std::string options = "box|gaussian|triangle|catmull-rom|sinc|mitchell|"
                "separable-catmull-rom|blackman-harris|min|max|average|zmin|zmax|*";
        groupBuilder.set("options", FnAttribute::StringAttribute(options));
        helpStr = "The name of the pixel filter to be used for the output "
                "display. The names of the standard pixel filters that may "
                "be passed to RiPixelFilter may also be used here (see the "
                "Pixel Filters section below for PRMan extensions). In "
                "addition, five special filters may be used: min, max, "
                "average, zmin, and zmax. The first three filters have the "
                "same meaning as the depthfilter argument to Hider, i.e. "
                "instead of running a convolution filter across all samples, "
                "only a single value (the minimum, maximum, or average of "
                "all pixel samples) is returned and written into the final "
                "pixel value. The zmin and zmax filters operate like the min "
                "and max filters, except that the depth value of the pixel "
                "sample is used for comparison, and not the value implied by "
                "the mode itself. These filters are useful for arbitrary "
                "output variables where standard alpha compositing does not "
                "make sense, or where linear interpolation of values between "
                "disjoint pieces of geometry is nonsensical. Note that when "
                "these filters are used, opacity thresholding is also used "
                "on that output to determine which closest surface to sample.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "filterwidth")
    {
        paramType = kFnRendererObjectValueTypeFloat;
        attrBuilder.push_back("1");
        attrBuilder.push_back("1");
        groupBuilder.set("tupleSize", FnAttribute::StringAttribute("2"));
        groupBuilder.set("int", FnAttribute::StringAttribute("True"));
        helpStr = "The size in X and Y of the pixel filter to be used.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "exposure")
    {
        paramType = kFnRendererObjectValueTypeFloat;
        attrBuilder.push_back("1");
        attrBuilder.push_back("1");
        groupBuilder.set("tupleSize", FnAttribute::StringAttribute("2"));
        helpStr = "The two values required are gain and gamma. These control "
                "the exposure function applied to the pixel values of the "
                "output display in the same manner as RiExposure.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else if (name == "dither")
    {
        paramType = kFnRendererObjectValueTypeFloat;
        attrBuilder.push_back("0.5");
        helpStr = "This single value controls the amplitude of the dither "
                "added to the values of the output display.";
        groupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
    }
    else
    {
        return false;
    }

    addRenderObjectParam(gb, name, paramType, arraySize,
        FnAttribute::StringAttribute(attrBuilder),
            groupBuilder.build(), enums);

    return true;
}

bool PRManRendererInfo::_buildOutputChannel(FnAttribute::GroupBuilder &gb,
                                            const std::string &name) const
{
    _configureBasicRenderObjectInfo(gb, name, kFnRendererObjectTypeOutputChannel);

    std::string attrName;
    int paramType;
    EnumPairVector enums;

    // type
    attrName = "type";
    paramType = kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder typeGroupBuilder;
    const char *typeOptions[] = {"*", "varying color", "varying float",
            "varying point","varying normal","varying vector"};
    typeGroupBuilder.set("options", FnAttribute::StringAttribute(typeOptions, 6, 1));
    typeGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
    addRenderObjectParam(gb, attrName, paramType, 0,
        FnAttribute::StringAttribute("varying float"),
            typeGroupBuilder.build(), enums);

    return true;
}

bool PRManRendererInfo::_buildRenderOutput(FnAttribute::GroupBuilder &gb,
                                           const std::string &name,
                                           const FnAttribute::GroupAttribute inputAttr) const
{
    _configureBasicRenderObjectInfo(gb, name, kFnRendererObjectTypeRenderOutput);

    FnAttribute::GroupAttribute globalStatementsAttr = inputAttr.getChildByName("prmanGlobalStatements");
    FnAttribute::GroupAttribute rendererSettingsAttr = inputAttr.getChildByName("rendererSettings");
    std::string attrName;
    std::string helpStr;
    int arraySize = 0;
    int paramType;
    EnumPairVector enums;

    if (name == "ptc")
    {
        // set default colorSpace
        attrName = "colorSpace";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::StringAttribute defaultColorSpace = inputAttr.getChildByName("OCIODefaultColorSpace");
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            defaultColorSpace,
                FnAttribute::Attribute(), enums);


        // set default fileExtension
        attrName = "fileExtension";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder fileExtensionGroupBuilder;
        const char *fileExtensionOptions[] = {"*", "ptc"};
        fileExtensionGroupBuilder.set("options", FnAttribute::StringAttribute(fileExtensionOptions, 2, 1));
        fileExtensionGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("ptc"),
                fileExtensionGroupBuilder.build(), enums);


        // validFrames
        attrName = "validFrames";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder validFramesGroupBuilder;
        helpStr = "Allows user control over when to expect a ptc file "
                "to be written by the renderer.  If set to <i>All</i> (the "
                "default), a missing ptc file on any rendered frame produces "
                "an error.  If set to <i>Range</i>, an additional "
                "<i>validFrameRange</i> parameter appears to specify which "
                "frames will produce an error for missing ptc files. If set "
                "to <i>None</i>, missing ptc files will never produce an error.";
        validFramesGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        const char *validFramesOptions[] = {"All", "Range", "None"};
        validFramesGroupBuilder.set("options", FnAttribute::StringAttribute(validFramesOptions, 3, 1));
        validFramesGroupBuilder.set("widget", FnAttribute::StringAttribute("capsule"));
        validFramesGroupBuilder.set("exclusive", FnAttribute::StringAttribute("True"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("All"),
                validFramesGroupBuilder.build(), enums);


        // validFrameRange
        attrName = "validFrameRange";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder validFrameRangeGroupBuilder;
        helpStr = "Frame range string specifying which frames should expect "
                "ptc files to be generated by the renderer.  Any frame outside "
                "this range will only warn for missing ptc files.<br/><br/>"
                "<dl>"
                "    <dt>Examples:</dt>"
                "    <dd>1-10</dd>"
                "</dl>";
        validFrameRangeGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(""),
                validFrameRangeGroupBuilder.build(), enums);

    }

    // add in prman-specific options for raw output
    if (name == kFnRendererOutputTypeRaw)
    {
        // displayDriver
        attrName = "displayDriver";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::StringBuilder displayDriverAttrBuilder;
        displayDriverAttrBuilder.push_back("openexr");
        FnAttribute::GroupBuilder displayDriverHintsBuilder;
        addRenderObjectParam(gb, attrName, paramType, arraySize, displayDriverAttrBuilder.build(),
                displayDriverHintsBuilder.build(), enums);

        // displayOptions
        attrName = "displayOptions";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder displayOptionsAttrBuilder;
        FnAttribute::GroupBuilder displayOptionsHintBuilder;
        addRenderObjectParam(gb, attrName, paramType, arraySize, displayOptionsAttrBuilder.build(),
                displayOptionsHintBuilder.build(), enums);
    }

    if (name == kFnRendererOutputTypeColor || name == kFnRendererOutputTypeRaw ||
            name == kFnRendererOutputTypeDeep)
    {
        // Add default output channels to a group
        std::vector<std::string> outputChannelsAttrBuilder;
        outputChannelsAttrBuilder.push_back("rgba");
        outputChannelsAttrBuilder.push_back("P");
        outputChannelsAttrBuilder.push_back("N");
        outputChannelsAttrBuilder.push_back("Ci");

        if (name == "" || name == kFnRendererOutputTypeRaw)
        {
            outputChannelsAttrBuilder.push_back("z");
            outputChannelsAttrBuilder.push_back("deepopacity");
        }

        // Append output channels from global statements
        FnAttribute::GroupAttribute outputGroup = globalStatementsAttr.getChildByName("outputChannels");
        if (outputGroup.isValid())
        {
            for (int i=0; i < outputGroup.getNumberOfChildren(); ++i)
            {
                // We want to get the name from the nested StringAttibute
                // "name", this can correctly contain symbols and punctuation.
                FnAttribute::GroupAttribute child =
                        outputGroup.getChildByIndex(i);

                // If the "name" attribute is not there we can still try to
                // use the groupAttr name.
                FnAttribute::StringAttribute channelNameAttr =
                        child.getChildByName("name");
                const std::string defaultChannelName =
                        outputGroup.getChildName(i);

                outputChannelsAttrBuilder.push_back(
                    channelNameAttr.getValue(defaultChannelName, false));
            }
        }

        attrName = "channel";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder channelGroupBuilder;

        std::string widgetName = "popup";
        if (name == kFnRendererOutputTypeDeep)
        {
            widgetName = "nonexclusiveCheckboxPopup";
        }
        else
        {
            outputChannelsAttrBuilder.push_back("*");
        }
        channelGroupBuilder.set("widget", FnAttribute::StringAttribute(widgetName));
        channelGroupBuilder.set("emptyLabel", FnAttribute::StringAttribute("rgba"));
        channelGroupBuilder.set("options", FnAttribute::StringAttribute(outputChannelsAttrBuilder));

        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("rgba"),
                channelGroupBuilder.build(), enums);
    }

    if (name == kFnRendererOutputTypeDeep)
    {
        // subimage
        attrName = "subimage";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder subimageGroupBuilder;
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(""),
                subimageGroupBuilder.build(), enums);

        // cameraName
        attrName = "cameraName";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder cameraNameGroupBuilder;
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(""),
                cameraNameGroupBuilder.build(), enums);

        // fileExtension
        attrName = "fileExtension";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder fileExtensionGroupBuilder;
        const char *fileExtensionOptions[] = {"*", "dtex", "deepshad"};
        fileExtensionGroupBuilder.set("options", FnAttribute::StringAttribute(fileExtensionOptions, 3, 1));
        fileExtensionGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("dtex"),
                fileExtensionGroupBuilder.build(), enums);

    }

    // prman-specific shadow settings
    if (name == kFnRendererOutputTypeShadow)
    {
        // shadowType
        attrName = "shadowType";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder shadowTypeGroupBuilder;
        const char *shadowTypeOptions[] = {"zfile", "deepshad"};
        shadowTypeGroupBuilder.set("options", FnAttribute::StringAttribute(shadowTypeOptions, 2, 1));
        shadowTypeGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        addRenderObjectParam(gb, attrName, paramType, arraySize, 
            FnAttribute::StringAttribute("zfile"),
                shadowTypeGroupBuilder.build(), enums);

        // deepshadMode
        attrName = "deepshadMode";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder deepshadModeGroupBuilder;
        const char *deepshadModeOptions[] = {"deepopacity", "areashadow"};
        deepshadModeGroupBuilder.set("options", FnAttribute::StringAttribute(deepshadModeOptions, 2, 1));
        deepshadModeGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        deepshadModeGroupBuilder.set("conditionalVisOp", FnAttribute::StringAttribute("equalTo"));
        deepshadModeGroupBuilder.set("conditionalVisPath", FnAttribute::StringAttribute("../shadowType"));
        deepshadModeGroupBuilder.set("conditionalVisValue", FnAttribute::StringAttribute("deepshad"));
        addRenderObjectParam(gb, attrName, paramType, arraySize, 
            FnAttribute::StringAttribute("deepopacity"),
                deepshadModeGroupBuilder.build(), enums);



        std::string fileExtension;
        std::vector<std::string> extOptionsAttrBuilder;
        extOptionsAttrBuilder.push_back("*");

        FnAttribute::StringAttribute shadowTypeAttr = rendererSettingsAttr.getChildByName("shadowType");

        std::string shadowType = "zfile";
        if (shadowTypeAttr.isValid())
        {
            shadowType = shadowTypeAttr.getValue();
        }

        if (shadowType == "deepshad")
        {
            std::string deepshadMode = "deepopacity";
            FnAttribute::StringAttribute deepshadAttr = rendererSettingsAttr.getChildByName("deepshadMode");
            if (deepshadAttr.isValid())
            {
                deepshadMode = deepshadAttr.getValue();
            }

            if (deepshadMode == "deepopacity")
            {
                fileExtension = "dshad";
                extOptionsAttrBuilder.push_back("dshad");
                extOptionsAttrBuilder.push_back("deepshad");
            }
            else if (deepshadMode == "areashadow")
            {
                fileExtension = "tdsm";
                extOptionsAttrBuilder.push_back("tdsm");
                extOptionsAttrBuilder.push_back("deepshad");
            }
            else
            {
                fileExtension = "deepshad";
                extOptionsAttrBuilder.push_back("deepshad");
            }
        }
        else
        {
            fileExtension = "tx";
            extOptionsAttrBuilder.push_back("tx");
        }

        // fileExtension
        attrName = "fileExtension";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder fileExtensionGroupBuilder;
        fileExtensionGroupBuilder.set("options", FnAttribute::StringAttribute(extOptionsAttrBuilder));
        fileExtensionGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(fileExtension),
                fileExtensionGroupBuilder.build(), enums);


        // deepshadVolumeInterpretation
        attrName = "deepshadVolumeInterpretation";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder deepshadVolumeInterpretationGroupBuilder;
        const char *deepshadVolumeInterpretationOptions[] = {"continuous", "discrete"};
        deepshadVolumeInterpretationGroupBuilder.set("options", FnAttribute::StringAttribute(deepshadVolumeInterpretationOptions, 2, 1));
        deepshadVolumeInterpretationGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        helpStr = "When writing deep shadows, the deep texture driver by default "
                "assumes that the data being written to it is volumetric in nature "
                "(the default value of the \"volumeinterpretation\" parameter is "
                "\"continuous\"). The resulting file will contain the best "
                "representation for volumes, and is also well suited for dense "
                "stacks of semi-transparent objects such as hair. If instead the "
                "data being written consists mainly of non-volumetric opaque "
                "objects, it is best to indicate this to the driver by passing "
                "the value of \"discrete\" for the \"volumeinterpretation\" parameter.";
        deepshadVolumeInterpretationGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        deepshadVolumeInterpretationGroupBuilder.set("conditionalVisOp", FnAttribute::StringAttribute("equalTo"));
        deepshadVolumeInterpretationGroupBuilder.set("conditionalVisPath", FnAttribute::StringAttribute("../shadowType"));
        deepshadVolumeInterpretationGroupBuilder.set("conditionalVisValue", FnAttribute::StringAttribute("deepshad"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("continuous"),
                deepshadVolumeInterpretationGroupBuilder.build(), enums);
    }

    if (name == kFnRendererOutputTypeScript)
    {
        // scriptCommand
        attrName = "scriptCommand";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder scriptCommandGroupBuilder;
        const char *scriptCommandOptions[] = {"*",
                                              "txmake -shadow $INPUT $OUTPUT",
                                              "txmake -mode clamp -resize up- $INPUT $OUTPUT",
                                              "txmake -mode clamp -resize up- -format tiff $INPUT $OUTPUT",
                                              "brickmake -maxerror 0.002 $INPUT $OUTPUT"};
        scriptCommandGroupBuilder.set("options", FnAttribute::StringAttribute(scriptCommandOptions, 5, 1));
        scriptCommandGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(""),
                scriptCommandGroupBuilder.build(), enums);

    }

    if (name == kFnRendererOutputTypeColor || name == kFnRendererOutputTypeShadow ||
            name == "ptc" || name == kFnRendererOutputTypeRaw)
    {
        // cameraName
        attrName = "cameraName";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder cameraNameGroupBuilder;
        helpStr = "Scene graph location of camera to render from.  If empty, render from camera "
                "specified at <i>renderSettings.cameraName</i> on <i>/root</i>.</p>";
        cameraNameGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        cameraNameGroupBuilder.set("widget", FnAttribute::StringAttribute("scenegraphLocation"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(""),
                cameraNameGroupBuilder.build(), enums);
    }


    return true;
}

void PRManRendererInfo::flushCaches()
{
    FnGeolibUtil::AutoWriteLock writeLock(g_readWriteLock);
    g_cache.flush();
}

// This will recursively traverse the directory specified by rootPath
// and add any .slo file names discovered.
bool PRManRendererInfo::_recursiveCacheShaderInfo(TrackPaths& trackPaths,
                                                  const std::string& rootPath,
                                                  const std::string& relpath,
                                                  const std::string& fullPath,
                                                  const std::string& d_name) const
{
    // Opt out early

    if(d_name == "")
    {
        return true;
    }

    if(d_name == ".")
    {
        return true;
    }

    if(d_name == "..")
    {
        return true;
    }

    char pathBuffer[PATH_MAX];

    // Find out whether this is a file, symbolic link or a directory

    struct stat dir_statbuf;
    stat(fullPath.c_str(), &dir_statbuf);

    // If this is a shader file then add it to the shader file map

    if( S_ISREG(dir_statbuf.st_mode) )
    {
        // Do the shader stuff
        if (pystring::endswith(relpath, ".slo"))
        {
            std::string shaderName = pystring::slice(relpath, 0, -4);
            // write lock is held by caller (_cacheShaderInfo())
            g_cache.addShaderInfo(shaderName, DirFilePair(rootPath, relpath));
        }

        return true;
    }

    // If this is a directory then recurse into it

    else if( S_ISDIR(dir_statbuf.st_mode) )
    {
        // Resolve any symbolic links
        // in the path name

        realpath(fullPath.c_str(), pathBuffer);

        // Check that we aren't entering the same
        // directory twice

        if( ! trackPaths.hasPath(pathBuffer) )
        {
            if(!trackPaths.addPath(pathBuffer))
            {
                return false;
            }

            DIR *d = opendir(pathBuffer);
            if (d)
            {
                // We can use the same strings for
                // all the child directory entries.
                // Which beats creating and throwing away
                // strings for no good reason.

                std::string entryNameFullPath;
                std::string entryNameRelPath;
                std::string entryName;

                struct dirent *r;
                while ( (r = readdir(d)) )
                {
                    // The name
                    entryName = r->d_name;

                    // The full path
                    entryNameFullPath = fullPath;
                    entryNameFullPath += "/";
                    entryNameFullPath += r->d_name;

                    // The relative path
                    entryNameRelPath = relpath;
                    entryNameRelPath += "/";
                    entryNameRelPath += r->d_name;

                    if(!PRManRendererInfo::_recursiveCacheShaderInfo(trackPaths, rootPath, entryNameRelPath, entryNameFullPath, entryName))
                    {
                        const char warning_message[] = "Have searched "
                                                     TOSTRING(MAX_PATHS_BEFORE_ERROR)
                                                     " directories under the RMAN_SHADERPATH and stopped at that limit."
                                                     " A symbolic link in a directory under the RMAN_SHADERPATH can do this."
                                                     " Not all of the shaders under the RMAN_SHADERPATH will be displayed in the katana UI.";

                        FnLogWarn(warning_message);

                        closedir(d);

                        return false;
                    }
                }
            }
            closedir(d);
        }
    }

    return true;
}

void PRManRendererInfo::_cacheShaderInfo() const
{
    // scope read lock to avoid interfering with write lock below
    {
        FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);
        if (!g_cache.isEmpty()) return;
    }

    FnLogInfo("Loading RenderMan shaders...");

    const char *rmanShaderPaths = ::getenv("RMAN_SHADERPATH");
    std::vector<std::string> shaderPaths;
    if (rmanShaderPaths)
    {
        pystring::split(rmanShaderPaths, shaderPaths, ":");
    }

    // For making sure we
    // don't end up in an infinite loop.
    // This stores an entry for every
    // path we visit. Symbolic links
    // are resolved in to real paths.
    TrackPaths trackPaths;

    // need write lock for remainder of method, since _recursiveCacheShaderInfo
    // modifies g_cache
    FnGeolibUtil::AutoWriteLock writeLock(g_readWriteLock);

    std::string path;
    std::vector<std::string>::reverse_iterator iter;
    for ( iter = shaderPaths.rbegin(); iter != shaderPaths.rend(); ++iter )
    {
        path = (*iter);

        if (path.empty())
        {
            continue;
        }

        FnLogDebug("Loading RenderMan shaders from \"" + path + "\"...");

        DIR *d = opendir(path.c_str());
        if (d)
        {
            struct dirent *r;
            std::string d_name;
            std::string fullPath;
            std::string relativePath;
            while ((r = readdir(d)))
            {
                d_name = r->d_name;
                fullPath = path;
                fullPath += "/";
                fullPath += r->d_name;
                relativePath = r->d_name;
                if(!_recursiveCacheShaderInfo(trackPaths, path, relativePath, fullPath, d_name))
                {
                    break;
                }
            }
            closedir(d);
        }
    }

    FnLogDebug("RenderMan shaders loaded.");
}

bool PRManRendererInfo::_getDirAndFileFromLocation(DirFilePair &dirFilePair,
                                                   std::string &objectName,
                                                   const std::string & location) const
{
    if (location.empty()) return false;

    std::string path = location;
    if (FnKat::DefaultAssetPlugin::isAssetId(location))
    {
        path = FnKat::DefaultAssetPlugin::resolveAsset(location);
    }


    struct stat statResult;
    if (::stat(path.c_str(), &statResult) == 0)
    {
        char *tmpPath = ::strdup(path.c_str());
        char *tmpPath2 = ::strdup(path.c_str());
        std::string dir = ::dirname(tmpPath);
        std::string base = ::basename(tmpPath2);
        ::free(tmpPath);
        ::free(tmpPath2);
        dirFilePair = DirFilePair(dir, base);
        objectName = base;
        if (pystring::endswith(objectName, ".slo"))
        {
            objectName = pystring::slice(objectName, 0, -4);
        }

        return true;
    }

    return false;
}


void PRManRendererInfo::_fillTypeTagsFromSloFile(std::set<std::string> &typeTags,
                                                 const std::string &dirName,
                                                 const std::string &shaderName) const
{
    Slo_SetPath(const_cast<char *>(dirName.c_str()));

    Slo_SetShader(const_cast<char *>(shaderName.c_str()));
    int shaderType = Slo_GetType();

    // Check if this is an RSL2.0 shader that uses a class instead of a standard shader
    bool isRsl2 = (shaderType == SLO_TYPE_SHADER);
    if (isRsl2)
    {
        typeTags.insert("class");
    }
    if (shaderType == SLO_TYPE_SURFACE ||
            (isRsl2 && Slo_HasMethod("surface")) ||
            (isRsl2 && Slo_HasMethod("prelighting")) ||
            (isRsl2 && Slo_HasMethod("lighting")) ||
            (isRsl2 && Slo_HasMethod("postlighting")) )
    {
        typeTags.insert("surface");
    }
    if (shaderType == SLO_TYPE_LIGHT || (isRsl2 && Slo_HasMethod("light")))
    {
        typeTags.insert("light");
    }
    if (shaderType == SLO_TYPE_DISPLACEMENT || (isRsl2 && Slo_HasMethod("displacement")))
    {
        typeTags.insert("displacement");
    }
    if (shaderType == SLO_TYPE_VOLUME)
    {
        typeTags.insert("atmosphere");
        typeTags.insert("interior");
        typeTags.insert("exterior");
    }
    else if (isRsl2)
    {
        if (Slo_HasMethod("atmosphere"))
        {
            typeTags.insert("atmosphere");
        }
        if (Slo_HasMethod("interior"))
        {
            typeTags.insert("interior");
        }
        if (Slo_HasMethod("exterior"))
        {
            typeTags.insert("exterior");
        }
    }

    Slo_EndShader();
}

void PRManRendererInfo::_fillParamsFromSloFile(FnAttribute::GroupBuilder& params, 
                                               const std::string& dirName, 
                                               const std::string& shaderName) const
{
    Slo_SetPath(const_cast<char *>(dirName.c_str()));
    Slo_SetShader(const_cast<char *>(shaderName.c_str()));

    // Set up mapping of shader parameter meta names to actual attribute names
    setShaderParameterMapping(params, "shader", "prmanLightShader");
    const std::string parameterNamePrefix = "prmanLightParams.";

    // Define lists of attribute names of shader parameters to collect names of
    // actual shader parameter attributes for shader parameter mapping after
    // the following loop
    std::vector<std::string> colorParameterNames;
    std::vector<std::string> intensityParameterNames;
    std::vector<std::string> exposureParameterNames;
    std::vector<std::string> innerConeAngleParameterNames;
    std::vector<std::string> outerConeAngleParameterNames;

    for (int i=1; i<=Slo_GetNArgs(); ++i)
    {
        SLO_VISSYMDEF *paramPtr = Slo_GetArgById(i);

        if (!paramPtr) continue;

        int type = kFnRendererObjectValueTypeUnknown;
        switch (paramPtr->svd_type)
        {
            case SLO_TYPE_POINT:  type = kFnRendererObjectValueTypePoint3;  break;
            case SLO_TYPE_COLOR:  type = kFnRendererObjectValueTypeColor3;  break;
            case SLO_TYPE_SCALAR: type = kFnRendererObjectValueTypeFloat;   break;
            case SLO_TYPE_STRING: type = kFnRendererObjectValueTypeString;  break;
            case SLO_TYPE_SHADER: type = kFnRendererObjectValueTypeShader;  break;
            case SLO_TYPE_VECTOR: type = kFnRendererObjectValueTypeVector3; break;
            case SLO_TYPE_NORMAL: type = kFnRendererObjectValueTypeNormal;  break;
            case SLO_TYPE_MATRIX: type = kFnRendererObjectValueTypeMatrix;  break;
            default:              type = kFnRendererObjectValueTypeUnknown; break;
        }

        if (type == kFnRendererObjectValueTypeFloat)
        {
            if (strcmp(paramPtr->svd_name, "Intensity") == 0
                || strcmp(paramPtr->svd_name, "intensity") == 0)
            {
                intensityParameterNames.push_back(parameterNamePrefix
                                                  + paramPtr->svd_name);
            }
            else if (strcmp(paramPtr->svd_name, "Exposure") == 0
                     || strcmp(paramPtr->svd_name, "exposure") == 0)
            {
                exposureParameterNames.push_back(parameterNamePrefix
                                                 + paramPtr->svd_name);
            }
            else if (strcmp(paramPtr->svd_name, "Cone_Inner_Angle") == 0)
            {
                innerConeAngleParameterNames.push_back(parameterNamePrefix
                                                       + paramPtr->svd_name);
            }
            else if (strcmp(paramPtr->svd_name, "Cone_Outer_Angle") == 0)
            {
                outerConeAngleParameterNames.push_back(parameterNamePrefix
                                                       + paramPtr->svd_name);
            }
        }
        else if (type == kFnRendererObjectValueTypeColor3
                 || type == kFnRendererObjectValueTypeColor4)
        {
            colorParameterNames.push_back(parameterNamePrefix
                                          + paramPtr->svd_name);
        }

        int arraySize = paramPtr->svd_arraylen;
        FnKat::Attribute defaultAttr;

        // Check the type of the current shader parameter
        if (type == kFnRendererObjectValueTypeString
            || type == kFnRendererObjectValueTypeShader)
        {
            std::vector<std::string> sb;

            // Check if the parameter provides a single value
            if (arraySize == 0)
            {
                // Check if the parameter provides a string
                if (paramPtr->svd_default.stringval)
                {
                    // Add the string provided to the string attribute builder
                    sb.push_back(paramPtr->svd_default.stringval);
                }
                else
                {
                    // Add an empty string to the string attribute builder
                    sb.push_back("");
                }
            }
            else
            {
                // Iterate over the values provided by the parameter and add
                // them or empty strings to the string builder one by one
                for (int ai = 0; ai < arraySize; ++ai)
                {
                    // Get the array element with the current index and check
                    // if it provides a string
                    SLO_VISSYMDEF *arrayPtr = Slo_GetArrayArgElement(paramPtr, ai);
                    if (arrayPtr && arrayPtr->svd_default.stringval)
                    {
                        // Add the string provided to the string attribute
                        // builder
                        sb.push_back(arrayPtr->svd_default.stringval);
                    }
                    else
                    {
                        // Add an empty string to the string attribute builder
                        sb.push_back("");
                    }
                }
            }

            // Build the string attribute to use as the default attribute
            // through the string attribute builder
            defaultAttr = FnAttribute::StringAttribute(sb);
        }
        else if (type == kFnRendererObjectValueTypePoint3 ||
                type == kFnRendererObjectValueTypeColor3 ||
                type == kFnRendererObjectValueTypeVector3 ||
                type == kFnRendererObjectValueTypeNormal)
        {
            if (arraySize == 0)
            {
                FnAttribute::FloatBuilder fb(3);
                if (paramPtr->svd_default.pointval)
                {
                    fb.push_back(paramPtr->svd_default.pointval->xval);
                    fb.push_back(paramPtr->svd_default.pointval->yval);
                    fb.push_back(paramPtr->svd_default.pointval->zval);
                }
                else
                {
                    fb.push_back(0);
                    fb.push_back(0);
                    fb.push_back(0);
                }

                defaultAttr = fb.build();
            }
            else
            {
                FnAttribute::FloatBuilder fb(3);
                for (int ai=0; ai<arraySize; ++ai)
                {
                    SLO_VISSYMDEF *arrayPtr = Slo_GetArrayArgElement(paramPtr, ai);
                    if (arrayPtr && arrayPtr->svd_default.pointval)
                    {
                        fb.push_back(arrayPtr->svd_default.pointval->xval);
                        fb.push_back(arrayPtr->svd_default.pointval->yval);
                        fb.push_back(arrayPtr->svd_default.pointval->zval);
                    }
                    else
                    {
                        fb.push_back(0);
                        fb.push_back(0);
                        fb.push_back(0);
                    }
                }
                defaultAttr = fb.build();
            }
        }
        else if (type == kFnRendererObjectValueTypeFloat)
        {
            FnAttribute::FloatBuilder fb;

            if (arraySize == 0)
            {
                if(paramPtr->svd_default.scalarval)
                {
                    fb.push_back(*paramPtr->svd_default.scalarval);
                }
                else
                {
                    fb.push_back(0.0f);
                }
            }
            else
            {
                for (int ai=0; ai<arraySize; ++ai)
                {
                    SLO_VISSYMDEF *arrayPtr = Slo_GetArrayArgElement(paramPtr, ai);
                    if (arrayPtr && arrayPtr->svd_default.scalarval)
                    {
                        fb.push_back(*arrayPtr->svd_default.scalarval);
                    }
                    else
                    {
                        fb.push_back(0.0f);
                    }

                }
            }

            defaultAttr = fb.build();
        }
        else if (type == kFnRendererObjectValueTypeMatrix)
        {
            FnAttribute::FloatBuilder fb(4);

            if (arraySize == 0)
            {
                for (int mi=0; mi<16; ++mi)
                {
                    fb.push_back(paramPtr->svd_default.matrixval[mi]);
                }
            }
            else
            {
                for (int ai=0; ai<arraySize; ++ai)
                {
                    SLO_VISSYMDEF *arrayPtr = Slo_GetArrayArgElement(paramPtr, ai);
                    if (arrayPtr && arrayPtr->svd_default.matrixval)
                    {
                        for (int mi=0; mi<16; ++mi)
                        {
                            fb.push_back(arrayPtr->svd_default.matrixval[mi]);
                        }
                    }
                    else
                    {
                        for (int mi=0; mi<16; ++mi)
                        {
                            fb.push_back(0);
                        }
                    }
                }
            }

            defaultAttr = fb.build();
        }

        EnumPairVector enums;

        // Build widget hints based on array size and parameter type
        FnKat::Attribute hintsAttr;
        if (arraySize == -1)
        {
            FnAttribute::GroupBuilder params;

            if (type == kFnRendererObjectValueTypeFloat
                || type == kFnRendererObjectValueTypeString)
            {
                params.set("widget", FnAttribute::StringAttribute("sortableArray"));
            }
            else if (type == kFnRendererObjectValueTypeColor3)
            {
                params.set("widget", FnAttribute::StringAttribute("dynamicArray"));
                params.set("panelWidget", FnAttribute::StringAttribute("color"));
            }
            else if (type == kFnRendererObjectValueTypeMatrix)
            {
                params.set("widget", FnAttribute::StringAttribute("dynamicArray"));
                params.set("tupleGroupSize", FnAttribute::IntAttribute(4));
            }
            else
            {
                params.set("widget", FnAttribute::StringAttribute("dynamicArray"));
            }

            params.set("isDynamicArray", FnAttribute::IntAttribute(1));

            hintsAttr = params.build();
        }
        else if (arraySize > 0)
        {
            FnAttribute::GroupBuilder params;

            if (type == kFnRendererObjectValueTypeColor3)
            {
                params.set("widget", FnAttribute::StringAttribute("colorArray"));
            }

            hintsAttr = params.build();
        }

        addRenderObjectParam(params, paramPtr->svd_name, type, arraySize, defaultAttr, hintsAttr, enums);
    }

    // Set up mapping of shader parameter meta names to actual attribute names
    if (colorParameterNames.size() > 0)
    {
        setShaderParameterMapping(params, "color", colorParameterNames);
    }
    if (intensityParameterNames.size() > 0)
    {
        setShaderParameterMapping(params, "intensity",
                                  intensityParameterNames);
    }
    if (exposureParameterNames.size() > 0)
    {
        setShaderParameterMapping(params, "exposure", exposureParameterNames);
    }
    if (innerConeAngleParameterNames.size() > 0)
    {
        setShaderParameterMapping(params, "innerConeAngle",
                                  innerConeAngleParameterNames);
    }
    if (outerConeAngleParameterNames.size() > 0)
    {
        setShaderParameterMapping(params, "outerConeAngle",
                                  outerConeAngleParameterNames);
    }

    Slo_EndShader();
}

