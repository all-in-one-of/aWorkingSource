# katana-config.cmake.in
#
# Sample usage:
# find_package(Katana REQUIRED PATH /path/to/katana/plugin_apis/cmake)
#
# This will generate CMake targets for each of Katana's plug-in API modules of
# the form Katana::MODULE_NAME. For example, Katana::FnAttribute.
#
# See the CMake projects in $KATANA_HOME/plugins for sample usage.
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.2)

get_filename_component(KATANA_PLUGINAPIS_DIR
    "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

find_package(Threads REQUIRED)


# -----------------------------------------------------------------------------
# Katana::FnAsset
if (NOT TARGET foundry.katana.FnAsset)
    add_library(foundry.katana.FnAsset STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnAsset/client/FnDefaultAssetPlugin.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnAsset/client/FnDefaultFileSequencePlugin.cpp"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY COMPILE_DEFINITIONS
        "FNASSET_STATIC"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNASSET_STATIC"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAsset
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnAsset)
    add_library(Katana::FnAsset ALIAS foundry.katana.FnAsset)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnAssetPlugin
if (NOT TARGET foundry.katana.FnAssetPlugin)
    add_library(foundry.katana.FnAssetPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnAsset/plugin/FnAsset.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnAsset/plugin/FnFileSequence.cpp"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNASSET_STATIC"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNASSET_STATIC"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAssetPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnAssetPlugin)
    add_library(Katana::FnAssetPlugin ALIAS foundry.katana.FnAssetPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnAttribute
if (NOT TARGET foundry.katana.FnAttribute)
    add_library(foundry.katana.FnAttribute STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnAttribute/client/FnAttribute.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnAttribute/client/FnAttributeUtils.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnAttribute/client/FnDataBuilder.cpp"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY COMPILE_DEFINITIONS
        "FNATTRIBUTE_STATIC"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNATTRIBUTE_STATIC"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
        "foundry.katana.pystring"
        "dl"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
        "$<LINK_ONLY:foundry.katana.pystring>"
        "$<LINK_ONLY:dl>"
    )
    set_property(TARGET foundry.katana.FnAttribute
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnAttribute)
    add_library(Katana::FnAttribute ALIAS foundry.katana.FnAttribute)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnAttributeFunctionPlugin
if (NOT TARGET foundry.katana.FnAttributeFunctionPlugin)
    add_library(foundry.katana.FnAttributeFunctionPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnAttributeFunction/plugin/FnAttributeFunctionPlugin.cpp"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNATTRIBUTEFUNCTION_STATIC"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNATTRIBUTEFUNCTION_STATIC"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAttributeFunctionPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnAttributeFunctionPlugin)
    add_library(Katana::FnAttributeFunctionPlugin ALIAS foundry.katana.FnAttributeFunctionPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnAttributeModifierPlugin
if (NOT TARGET foundry.katana.FnAttributeModifierPlugin)
    add_library(foundry.katana.FnAttributeModifierPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnAttributeModifier/plugin/FnAttributeModifier.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnAttributeModifier/plugin/FnAttributeModifierInput.cpp"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNATTRIBUTEMODIFIER_STATIC"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNATTRIBUTEMODIFIER_STATIC"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnAttributeModifierPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnAttributeModifierPlugin)
    add_library(Katana::FnAttributeModifierPlugin ALIAS foundry.katana.FnAttributeModifierPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnConfig
if (NOT TARGET foundry.katana.FnConfig)
    add_library(foundry.katana.FnConfig STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnConfig/FnConfig.cpp"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY COMPILE_DEFINITIONS
        "FNCONFIG_STATIC"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNCONFIG_STATIC"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnConfig
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnConfig)
    add_library(Katana::FnConfig ALIAS foundry.katana.FnConfig)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnDefaultAttributeProducerPlugin
if (NOT TARGET foundry.katana.FnDefaultAttributeProducerPlugin)
    add_library(foundry.katana.FnDefaultAttributeProducerPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnDefaultAttributeProducer/plugin/FnDefaultAttributeProducerPlugin.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDefaultAttributeProducer/plugin/FnDefaultAttributeProducerUtil.cpp"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNDEFAULTATTRIBUTEPRODUCER_STATIC"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNDEFAULTATTRIBUTEPRODUCER_STATIC"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
        "foundry.katana.FnGeolibServices"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnPluginSystem"
        "foundry.katana.pystring"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
        "foundry.katana.FnGeolibServices"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnPluginSystem"
        "foundry.katana.pystring"
    )
    set_property(TARGET foundry.katana.FnDefaultAttributeProducerPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnDefaultAttributeProducerPlugin)
    add_library(Katana::FnDefaultAttributeProducerPlugin ALIAS foundry.katana.FnDefaultAttributeProducerPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnGeolibOpPlugin
if (NOT TARGET foundry.katana.FnGeolibOpPlugin)
    add_library(foundry.katana.FnGeolibOpPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolib/op/FnGeolibCookInterface.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolib/op/FnGeolibCookInterfaceUtils.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolib/op/FnGeolibOp.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolib/op/FnGeolibSetupInterface.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolib/util/Path.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolib/util/PowerNap.cpp"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNGEOLIB_STATIC"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNGEOLIB_STATIC"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
        "foundry.katana.FnAsset"
        "foundry.katana.FnPluginManager"
        "foundry.katana.pystring"
        "Threads::Threads"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
        "$<LINK_ONLY:foundry.katana.FnAsset>"
        "$<LINK_ONLY:foundry.katana.FnPluginManager>"
        "$<LINK_ONLY:foundry.katana.pystring>"
        "$<LINK_ONLY:Threads::Threads>"
    )
    set_property(TARGET foundry.katana.FnGeolibOpPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnGeolibOpPlugin)
    add_library(Katana::FnGeolibOpPlugin ALIAS foundry.katana.FnGeolibOpPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnGeolibServices
if (NOT TARGET foundry.katana.FnGeolibServices)
    add_library(foundry.katana.FnGeolibServices STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnArbitraryOutputAttr.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnArgsFile.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnAttributeFunctionUtil.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnBuiltInOpArgsUtil.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnExpressionMath.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnGeolibCookInterfaceUtilsService.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnHintUtil.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnLookFile.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnMaterialResolveUtil.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnResolutionTable.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnResourceFiles.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnGeolibServices/client/FnXFormUtil.cpp"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY COMPILE_DEFINITIONS
        "FNGEOLIBSERVICES_STATIC"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNGEOLIBSERVICES_STATIC"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
        "foundry.katana.FnPluginSystem"
        "foundry.katana.FnPluginManager"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
        "foundry.katana.FnPluginSystem"
        "$<LINK_ONLY:foundry.katana.FnPluginManager>"
    )
    set_property(TARGET foundry.katana.FnGeolibServices
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnGeolibServices)
    add_library(Katana::FnGeolibServices ALIAS foundry.katana.FnGeolibServices)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnLogging
if (NOT TARGET foundry.katana.FnLogging)
    add_library(foundry.katana.FnLogging STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnLogging/client/FnLogging.cpp"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY COMPILE_DEFINITIONS
        "FNLOGGING_STATIC"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNLOGGING_STATIC"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnLogging
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnLogging)
    add_library(Katana::FnLogging ALIAS foundry.katana.FnLogging)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnPlatform
if (NOT TARGET foundry.katana.FnPlatform)
    add_library(foundry.katana.FnPlatform INTERFACE)
    set_property(TARGET foundry.katana.FnPlatform
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNPLATFORM_STATIC"
    )
    set_property(TARGET foundry.katana.FnPlatform
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
endif ()
if (NOT TARGET Katana::FnPlatform)
    add_library(Katana::FnPlatform ALIAS foundry.katana.FnPlatform)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnPluginManager
if (NOT TARGET foundry.katana.FnPluginManager)
    add_library(foundry.katana.FnPluginManager STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnPluginManager/client/FnPluginManager.cpp"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY COMPILE_DEFINITIONS
        "FNPLUGINMANAGER_STATIC"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNPLUGINMANAGER_STATIC"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnPluginManager
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnPluginManager)
    add_library(Katana::FnPluginManager ALIAS foundry.katana.FnPluginManager)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnPluginSystem
if (NOT TARGET foundry.katana.FnPluginSystem)
    add_library(foundry.katana.FnPluginSystem STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnPluginSystem/FnPlugin.cpp"
    )
    set_property(TARGET foundry.katana.FnPluginSystem
        PROPERTY COMPILE_DEFINITIONS
        "FNPLUGINSYSTEM_STATIC"
    )
    set_property(TARGET foundry.katana.FnPluginSystem
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNPLUGINSYSTEM_STATIC"
    )
    set_property(TARGET foundry.katana.FnPluginSystem
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnPluginSystem
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnPluginSystem
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnPluginSystem)
    add_library(Katana::FnPluginSystem ALIAS foundry.katana.FnPluginSystem)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnRenderOutputLocation
if (NOT TARGET foundry.katana.FnRenderOutputLocation)
    add_library(foundry.katana.FnRenderOutputLocation STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputLocation/client/FnRenderOutputLocationPluginClient.cpp"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY COMPILE_DEFINITIONS
        "FNRENDEROUTPUTLOCATION_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNRENDEROUTPUTLOCATION_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocation
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnRenderOutputLocation)
    add_library(Katana::FnRenderOutputLocation ALIAS foundry.katana.FnRenderOutputLocation)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnRenderOutputLocationPlugin
if (NOT TARGET foundry.katana.FnRenderOutputLocationPlugin)
    add_library(foundry.katana.FnRenderOutputLocationPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputLocation/plugin/FnRenderOutputLocationPlugin.cpp"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNRENDEROUTPUTLOCATION_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNRENDEROUTPUTLOCATION_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnRenderOutputLocationPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnRenderOutputLocationPlugin)
    add_library(Katana::FnRenderOutputLocationPlugin ALIAS foundry.katana.FnRenderOutputLocationPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnRenderOutputUtils
if (NOT TARGET foundry.katana.FnRenderOutputUtils)
    add_library(foundry.katana.FnRenderOutputUtils STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputUtils/client/CameraInfo.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputUtils/client/FnRenderOutputUtils.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputUtils/client/ShadingNodeConnectionDescription.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputUtils/client/ShadingNodeDescription.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputUtils/client/ShadingNodeDescriptionMap.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRenderOutputUtils/client/XFormMatrix.cpp"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY COMPILE_DEFINITIONS
        "FNRENDEROUTPUTUTILS_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNRENDEROUTPUTUTILS_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
        "foundry.katana.FnScenegraphIterator"
        "dl"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
        "foundry.katana.FnScenegraphIterator"
        "$<LINK_ONLY:dl>"
    )
    set_property(TARGET foundry.katana.FnRenderOutputUtils
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnRenderOutputUtils)
    add_library(Katana::FnRenderOutputUtils ALIAS foundry.katana.FnRenderOutputUtils)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnRenderPlugin
if (NOT TARGET foundry.katana.FnRenderPlugin)
    add_library(foundry.katana.FnRenderPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/CameraSettings.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/CommandLineTools.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/CopyAndConvertRenderAction.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/CopyRenderAction.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/DiskRenderOutputProcess.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/GlobalSettings.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/IdSenderFactory.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/NoOutputRenderAction.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/PostCommandsRenderAction.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/RenderAction.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/RenderBase.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/RenderSettings.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/ScenegraphLocationDelegate.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/SocketIdSender.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/TemporaryRenderAction.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRender/plugin/SocketConnection.cpp"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNRENDER_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNRENDER_STATIC"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnRenderOutputUtils"
        "foundry.katana.FnScenegraphIterator"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnRenderOutputUtils"
        "foundry.katana.FnScenegraphIterator"
    )
    set_property(TARGET foundry.katana.FnRenderPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnRenderPlugin)
    add_library(Katana::FnRenderPlugin ALIAS foundry.katana.FnRenderPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnRendererInfo
if (NOT TARGET foundry.katana.FnRendererInfo)
    add_library(foundry.katana.FnRendererInfo STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnRendererInfo/client/FnRendererInfoPluginClient.cpp"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY COMPILE_DEFINITIONS
        "FNRENDERERINFO_STATIC"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNRENDERERINFO_STATIC"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnGeolibOpPlugin"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "$<LINK_ONLY:foundry.katana.FnGeolibOpPlugin>"
    )
    set_property(TARGET foundry.katana.FnRendererInfo
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnRendererInfo)
    add_library(Katana::FnRendererInfo ALIAS foundry.katana.FnRendererInfo)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnRendererInfoPlugin
if (NOT TARGET foundry.katana.FnRendererInfoPlugin)
    add_library(foundry.katana.FnRendererInfoPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnRendererInfo/plugin/LiveRenderControlModule.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRendererInfo/plugin/LiveRenderFilter.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRendererInfo/plugin/RenderMethod.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRendererInfo/plugin/RendererInfoBase.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnRendererInfo/plugin/ShaderInfoCache.cpp"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNRENDERERINFO_STATIC"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNRENDERERINFO_STATIC"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginManager"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnRendererInfoPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnRendererInfoPlugin)
    add_library(Katana::FnRendererInfoPlugin ALIAS foundry.katana.FnRendererInfoPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnScenegraphGeneratorPlugin
if (NOT TARGET foundry.katana.FnScenegraphGeneratorPlugin)
    add_library(foundry.katana.FnScenegraphGeneratorPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnScenegraphGenerator/plugin/FnScenegraphGenerator.cpp"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNSCENEGRAPHGENERATOR_STATIC"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNSCENEGRAPHGENERATOR_STATIC"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnScenegraphGeneratorPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnScenegraphGeneratorPlugin)
    add_library(Katana::FnScenegraphGeneratorPlugin ALIAS foundry.katana.FnScenegraphGeneratorPlugin)
endif ()

# -----------------------------------------------------------------------------
# Katana::FnScenegraphIterator
if (NOT TARGET foundry.katana.FnScenegraphIterator)
    add_library(foundry.katana.FnScenegraphIterator STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnScenegraphIterator/client/FnScenegraphIterator.cpp"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY COMPILE_DEFINITIONS
        "FNSCENEGRAPHITERATOR_STATIC"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNSCENEGRAPHITERATOR_STATIC"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY INTERFACE_LINK_LIBRARIES
        "foundry.katana.FnAttribute"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnScenegraphIterator
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnScenegraphIterator)
    add_library(Katana::FnScenegraphIterator ALIAS foundry.katana.FnScenegraphIterator)
endif ()

# -----------------------------------------------------------------------------
# Katana::pystring
if (NOT TARGET foundry.katana.pystring)
    add_library(foundry.katana.pystring STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/pystring/pystring.cpp"
    )
    set_property(TARGET foundry.katana.pystring
        PROPERTY COMPILE_DEFINITIONS
        "PYSTRING_STATIC"
    )
    set_property(TARGET foundry.katana.pystring
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "PYSTRING_STATIC"
    )
    set_property(TARGET foundry.katana.pystring
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.pystring
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.pystring
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::pystring)
    add_library(Katana::pystring ALIAS foundry.katana.pystring)
endif ()


# ------------------------------------------------------------------------------
# Katana::FnDisplayDriver
if (NOT TARGET foundry.katana.FnDisplayDriver)
    add_library(foundry.katana.FnDisplayDriver STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/KatanaPipe.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/NewChannelMessage.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/NewFrameMessage.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/EndOfRenderMessage.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/Message.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/DataMessage.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/CloseMessage.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/client/FnDisplayDriver.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnDisplayDriver/KatanaPipeSingleton.cpp"
    )
    set_property(TARGET foundry.katana.FnDisplayDriver
        PROPERTY COMPILE_DEFINITIONS
        "FNDISPLAYDRIVER_STATIC"
    )
    set_property(TARGET foundry.katana.FnDisplayDriver
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNDISPLAYDRIVER_STATIC"
    )
    set_property(TARGET foundry.katana.FnDisplayDriver
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnDisplayDriver
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnDisplayDriver
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnDisplayDriver)
    add_library(Katana::FnDisplayDriver ALIAS foundry.katana.FnDisplayDriver)
endif ()

# ------------------------------------------------------------------------------
# Katana::FnViewerModifierPlugin
if (NOT TARGET foundry.katana.FnViewerModifierPlugin)
    add_library(foundry.katana.FnViewerModifierPlugin STATIC
        EXCLUDE_FROM_ALL
        "${KATANA_PLUGINAPIS_DIR}/src/FnViewerModifier/plugin/FnViewerModifierInput.cpp"
        "${KATANA_PLUGINAPIS_DIR}/src/FnViewerModifier/plugin/FnViewerModifier.cpp"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY COMPILE_DEFINITIONS
        "FNVIEWERMODIFIER_STATIC"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY INTERFACE_COMPILE_DEFINITIONS
        "FNVIEWERMODIFIER_STATIC"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "${KATANA_PLUGINAPIS_DIR}/include"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY LINK_LIBRARIES
        "foundry.katana.FnAsset"
        "foundry.katana.FnAttribute"
        "foundry.katana.FnLogging"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY INTERFACE_LINK_LIBRARIES
        "$<LINK_ONLY:foundry.katana.FnAsset>"
        "foundry.katana.FnAttribute"
        "$<LINK_ONLY:foundry.katana.FnLogging>"
        "foundry.katana.FnPluginSystem"
    )
    set_property(TARGET foundry.katana.FnViewerModifierPlugin
        PROPERTY POSITION_INDEPENDENT_CODE
        "TRUE"
    )
endif ()
if (NOT TARGET Katana::FnViewerModifierPlugin)
    add_library(Katana::FnViewerModifierPlugin ALIAS foundry.katana.FnViewerModifierPlugin)
endif ()
