// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef RENDEREROBJECTDEFINITIONS_H_
#define RENDEREROBJECTDEFINITIONS_H_

/**
 * \defgroup RendererObjectDefinitions RendererObjectDefinitions
 * \ingroup RenderAPI
 * @{
 *
 * @brief Supported renderer object definitions such as object and parameter types,
 *        renderer object key properties, and parameter value types.
 */
#define kFnRendererObjectTypeShader                    "shader"                   /**< Used to populate shaders and their
                                                                                       parameters in the Material
                                                                                       and [Renderer]ShadingNode nodes */
#define kFnRendererObjectTypeFilter                    "filter"                   /**< Used to populate filters and their
                                                                                       parameters in the
                                                                                       [Renderer]OutputChannel node */
#define kFnRendererObjectTypeDriver                    "driver"                   /**< Used to populate drivers and their
                                                                                       parameters in the
                                                                                       [Renderer]OutputChannel node */
#define kFnRendererObjectTypeRenderOutput              "renderOutput"             /**< Used to populate the type drop-down
                                                                                       parameter in the RenderOutputDefine
                                                                                       node */
#define kFnRendererObjectTypeOutputChannel             "outputChannel"            /**< Used to populate the type drop-down
                                                                                       parameter in the
                                                                                       [Renderer]OutputChannel node */
#define kFnRendererObjectTypeOutputChannelAttrHints    "outputChannelAttrHints"   /**< Used to declare UI hints for the
                                                                                       generated output channel attributes */
#define kFnRendererObjectTypeOutputChannelCustomParam  "outputChannelCustomParam" /**< Used to populate an 'Add' drop-down
                                                                                       list of channel parameter types in
                                                                                       the [Renderer]OutputChannel node */
#define kFnRendererObjectTypeRendererProcedural        "rendererProcedural"       /**< This is currently not being used */
#define kFnRendererObjectTypeRendererPrimitive         "rendererPrimitive"        /**< This is currently not being used */

#define kFnRendererObjectInfoKeyProtocolVersion "protocolVersion"   /**< The protocol version used to parse the attributes
                                                                         describing the renderer object information */
#define kFnRendererObjectInfoKeyName            "name"              /**< The name of the renderer object (e.g. a shader name) */
#define kFnRendererObjectInfoKeyType            "type"              /**< The type of the renderer object (e.g. a shader) */
#define kFnRendererObjectInfoKeyTypeTags        "typeTags"          /**< The tags associated with the type (e.g. shader types) */
#define kFnRendererObjectInfoKeyLocation        "location"          /**< The directory or assetId for the renderer object */
#define kFnRendererObjectInfoKeyFullPath        "fullPath"          /**< Absolute path to the renderer object */
#define kFnRendererObjectInfoKeyParam           "param"             /**< The grouped parameters for the renderer object
                                                                         (e.g. shader parameters) */
#define kFnRendererObjectInfoKeyOutputType      "outputType"        /**< An output type if applicable, otherwise use
                                                                         kFnRendererObjectValueTypeUnknown */
#define kFnRendererObjectInfoKeyContainerHints  "containerHints"    /**< Container level UI hints such as page properties
                                                                         (open, help, hide, etc.) */

#define kFnRendererObjectInfoKeyParamType       "type"              /**< The type of a renderer object parameter */
#define kFnRendererObjectInfoKeyParamArraySize  "arraySize"         /**< The array size of a renderer object parameter */
#define kFnRendererObjectInfoKeyParamDefault    "default"           /**< The default value of a renderer object parameter */
#define kFnRendererObjectInfoKeyParamHints      "hints"             /**< UI hints for a renderer object parameter */
#define kFnRendererObjectInfoKeyParamEnums      "enums"             /**< Enumerated values for a renderer object parameter */

#define kFnRendererObjectValueTypeNull       -2                     /**< Null parameter type */
#define kFnRendererObjectValueTypeUnknown    -1                     /**< Unknown parameter type */
#define kFnRendererObjectValueTypeByte        0                     /**< Byte parameter type */
#define kFnRendererObjectValueTypeInt         1                     /**< Int parameter type */
#define kFnRendererObjectValueTypeUint        2                     /**< Unsigned int parameter type */
#define kFnRendererObjectValueTypeBoolean     3                     /**< Boolean parameter type */
#define kFnRendererObjectValueTypeFloat       4                     /**< Float parameter type */
#define kFnRendererObjectValueTypeColor3      5                     /**< 3 color values parameter type (RBG) */
#define kFnRendererObjectValueTypeColor4      6                     /**< 4 color values parameter type (RBGA) */
#define kFnRendererObjectValueTypeVector2     7                     /**< 2-dimensional vector parameter type */
#define kFnRendererObjectValueTypeVector3     8                     /**< 3-dimensional vector parameter type */
#define kFnRendererObjectValueTypeVector4     9                     /**< 4-dimensional vector parameter type */
#define kFnRendererObjectValueTypePoint2      10                    /**< 2-dimensional point parameter type */
#define kFnRendererObjectValueTypePoint3      11                    /**< 3-dimensional point parameter type */
#define kFnRendererObjectValueTypePoint4      12                    /**< 4-dimensional homogeneous point parameter type */
#define kFnRendererObjectValueTypeString      13                    /**< String parameter type */
#define kFnRendererObjectValueTypeMatrix      14                    /**< Matrix parameter type */
#define kFnRendererObjectValueTypeEnum        15                    /**< Enumerated parameter type */
#define kFnRendererObjectValueTypeNormal      16                    /**< Normal vector parameter type */
#define kFnRendererObjectValueTypePointer     17                    /**< Pointer parameter type */
#define kFnRendererObjectValueTypeShader      18                    /**< Shader parameter type */
#define kFnRendererObjectValueTypeLocation    19                    /**< Location parameter type */

#define kFnRendererOutputTypeColor      "color"                     /**< Color output type (RenderOutputDefine) */
#define kFnRendererOutputTypeDeep       "deep"                      /**< Deep output type (RenderOutputDefine) */
#define kFnRendererOutputTypeShadow     "shadow"                    /**< Shadow output type (RenderOutputDefine) */
#define kFnRendererOutputTypeRaw        "raw"                       /**< Raw output type (RenderOutputDefine) */
#define kFnRendererOutputTypeScript     "script"                    /**< Script (post) output type (RenderOutputDefine) */
#define kFnRendererOutputTypePreScript  "prescript"                 /**< Pre-script output type (RenderOutputDefine) */
#define kFnRendererOutputTypeMerge      "merge"                     /**< Merge outputs (RenderOutputDefine) */
#define kFnRendererOutputTypeForceNone  "none"                      /**< No output (RenderOutputDefine) */
/**
 * @}
 */

#endif /* RENDEREROBJECTDEFINITIONS_H_ */
