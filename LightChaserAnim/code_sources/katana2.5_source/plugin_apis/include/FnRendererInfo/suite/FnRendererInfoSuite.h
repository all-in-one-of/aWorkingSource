// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnRendererInfoSuite_H
#define FnRendererInfoSuite_H

extern "C" {

#include <FnAttribute/suite/FnAttributeSuite.h>

// Render Method constants
#define kFnRenderMethodTypeDiskRender       "diskRender"
#define kFnRenderMethodTypePreviewRender    "previewRender"
#define kFnRenderMethodTypeLiveRender       "liveRender"

// Constants used as keys of the stateArg parameter of:
//   getLiveRenderTerminalOps()
//   getRenderTerminalOps()
//   fillLiveRenderTerminalOps()
//   fillRenderTerminalOps()
#define kFnTerminalOpStateArgRenderMethodType   "renderMethodType"
#define kTerminalOpStateArgSystem               "system"


typedef struct RendererInfoPluginStruct* RendererInfoPluginHandle;

#define RendererInfoPluginSuite_version 2

struct RendererInfoPluginSuite_v1
{
    RendererInfoPluginHandle (*create)();

    void (*destroy)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getRenderMethods)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getBatchRenderMethod)(
        RendererInfoPluginHandle handle);

    // return string attribute containing list of renderer node names
    FnAttributeHandle (*getRendererObjectNames)(
        RendererInfoPluginHandle handle,
        const char *type,
        FnAttributeHandle typeTags);

    // return string attribute containing list of renderer shader type names
    FnAttributeHandle (*getRendererObjectTypes)(
        RendererInfoPluginHandle handle,
        const char *type);

    FnAttributeHandle (*getRendererCoshaderType)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getRegisteredRendererName)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getRegisteredRendererVersion)(
        RendererInfoPluginHandle handle);

    bool (*isPresetLocalFileNeeded)(
        RendererInfoPluginHandle handle,
        const char *outputType);

    bool (*isPolymeshFacesetSplittingEnabled)(
        RendererInfoPluginHandle handle);

    bool (*isNodeTypeSupported)(
        RendererInfoPluginHandle handle,
        const char *nodeType);

    // return string attribute containing the tags associated with a shader type
    FnAttributeHandle (*getRendererShaderTypeTags)(
        RendererInfoPluginHandle handle,
        const char *shaderType);

    // return string attribute containing default output type name
    FnAttributeHandle (*getRendererObjectDefaultType)(
        RendererInfoPluginHandle handle,
        const char *type);

    FnAttributeHandle (*getLiveRenderControlModules)(
        RendererInfoPluginHandle handle);

    // return group attribute containing dict of renderer object info
    FnAttributeHandle (*getRendererObjectInfo)(
        RendererInfoPluginHandle handle,
        const char *name,
        const char *type,
        const FnAttributeHandle inputData);

    // return the list of the names of inputs of a shader
    FnAttributeHandle (*getShaderInputNames)(
        RendererInfoPluginHandle handle,
        const char *shader);

    // return the list of the tags associated with a given input of a shader
    FnAttributeHandle (*getShaderInputTags)(
        RendererInfoPluginHandle handle,
        const char *shader,
        const char *inputName);

    // return the list of the names of outputs of a shader
    FnAttributeHandle (*getShaderOutputNames)(
        RendererInfoPluginHandle handle,
        const char *shader);

    // return the list of the tags associated with a given input of a shader
    FnAttributeHandle (*getShaderOutputTags)(
        RendererInfoPluginHandle handle,
        const char *shader,
        const char *outputName);

    // automatically assign typeTag to shaders whose name matches the filename-style (fnmatch) filter
    void (*setTypeTagNameFilter)(
        RendererInfoPluginHandle handle,
        const char *filter,
        const char *typeTag);

    // Gives the plugin its location path
    void (*setPluginPath)(
        RendererInfoPluginHandle handle,
        const char *pluginPath);

    // Gives the plugin the path to its 'root' directory
    void (*setPluginRootPath)(
        RendererInfoPluginHandle handle,
        const char *pluginRootPath);

    // Tells the plugin what is Katana's path
    void (*setKatanaPath)(
        RendererInfoPluginHandle handle,
        const char *katanaPath);

    // Tells the plugin what is the current Katana session's temporary directory
    void (*setTmpPath)(
        RendererInfoPluginHandle handle,
        const char *tmpPath);

    // add full path or assetId that resolves to a shader location
    //  examples: /my/path/to/shader.slo
    //            {spref:my/shader.slo}
    void (*addObjectLocation)(
        RendererInfoPluginHandle handle,
        const char *type,
        const char *location);

    void (*clearObjectLocations)(
        RendererInfoPluginHandle handle,
        const char *type);

    void (*flushCaches)(
        RendererInfoPluginHandle handle);

    void (*initialiseCaches)(
        RendererInfoPluginHandle handle);
};

struct RendererInfoPluginSuite_v2
{

    RendererInfoPluginHandle (*create)();

    void (*destroy)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getRenderMethods)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getBatchRenderMethod)(
        RendererInfoPluginHandle handle);

    // return string attribute containing list of renderer node names
    FnAttributeHandle (*getRendererObjectNames)(
        RendererInfoPluginHandle handle,
        const char *type,
        FnAttributeHandle typeTags);

    // return string attribute containing list of renderer shader type names
    FnAttributeHandle (*getRendererObjectTypes)(
        RendererInfoPluginHandle handle,
        const char *type);

    FnAttributeHandle (*getRendererCoshaderType)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getRegisteredRendererName)(
        RendererInfoPluginHandle handle);

    FnAttributeHandle (*getRegisteredRendererVersion)(
        RendererInfoPluginHandle handle);

    bool (*isPresetLocalFileNeeded)(
        RendererInfoPluginHandle handle,
        const char *outputType);

    bool (*isPolymeshFacesetSplittingEnabled)(
        RendererInfoPluginHandle handle);

    bool (*isNodeTypeSupported)(
        RendererInfoPluginHandle handle,
        const char *nodeType);

    // return string attribute containing the tags associated with a shader type
    FnAttributeHandle (*getRendererShaderTypeTags)(
        RendererInfoPluginHandle handle,
        const char *shaderType);

    // return string attribute containing default output type name
    FnAttributeHandle (*getRendererObjectDefaultType)(
        RendererInfoPluginHandle handle,
        const char *type);

    FnAttributeHandle (*getLiveRenderControlModules)(
        RendererInfoPluginHandle handle);

    // return group attribute containing dict of renderer object info
    FnAttributeHandle (*getRendererObjectInfo)(
        RendererInfoPluginHandle handle,
        const char *name,
        const char *type,
        const FnAttributeHandle inputData);

    // return the list of the names of inputs of a shader
    FnAttributeHandle (*getShaderInputNames)(
        RendererInfoPluginHandle handle,
        const char *shader);

    // return the list of the tags associated with a given input of a shader
    FnAttributeHandle (*getShaderInputTags)(
        RendererInfoPluginHandle handle,
        const char *shader,
        const char *inputName);

    // return the list of the names of outputs of a shader
    FnAttributeHandle (*getShaderOutputNames)(
        RendererInfoPluginHandle handle,
        const char *shader);

    // return the list of the tags associated with a given input of a shader
    FnAttributeHandle (*getShaderOutputTags)(
        RendererInfoPluginHandle handle,
        const char *shader,
        const char *outputName);

    // automatically assign typeTag to shaders whose name matches the filename-style (fnmatch) filter
    void (*setTypeTagNameFilter)(
        RendererInfoPluginHandle handle,
        const char *filter,
        const char *typeTag);

    // Gives the plugin its location path
    void (*setPluginPath)(
        RendererInfoPluginHandle handle,
        const char *pluginPath);

    // Gives the plugin the path to its 'root' directory
    void (*setPluginRootPath)(
        RendererInfoPluginHandle handle,
        const char *pluginRootPath);

    // Tells the plugin what is Katana's path
    void (*setKatanaPath)(
        RendererInfoPluginHandle handle,
        const char *katanaPath);

    // Tells the plugin what is the current Katana session's temporary directory
    void (*setTmpPath)(
        RendererInfoPluginHandle handle,
        const char *tmpPath);

    // add full path or assetId that resolves to a shader location
    //  examples: /my/path/to/shader.slo
    //            {spref:my/shader.slo}
    void (*addObjectLocation)(
        RendererInfoPluginHandle handle,
        const char *type,
        const char *location);

    void (*clearObjectLocations)(
        RendererInfoPluginHandle handle,
        const char *type);

    void (*flushCaches)(
        RendererInfoPluginHandle handle);

    // Deprecated in 2.0v4, to be removed in 2.1
    void (*initialiseCaches)(
        RendererInfoPluginHandle handle);

    // New in version 2
    // -------------------------------------------------------------------------

    FnAttributeHandle (*getLiveRenderTerminalOps)(
        RendererInfoPluginHandle handle,
        const FnAttributeHandle stateArgs);

    FnAttributeHandle (*getRenderTerminalOps)(
        RendererInfoPluginHandle handle,
        const FnAttributeHandle stateArgs);
};

}
#endif // FnRendererInfoSuite_H
