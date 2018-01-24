// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnGeolibServices/FnMaterialResolveUtil.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    FnAttribute::GroupAttribute
    FnMaterialResolveUtil::resolveMaterialReferences(
            const FnAttribute::GroupAttribute & inputMaterial,
            bool processHints)
    {
        const FnMaterialResolveUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return inputMaterial;

        return FnAttribute::Attribute::CreateAndSteal(
                suite->resolveMaterialReferences(inputMaterial.getHandle(),
                        processHints));

    }

    FnAttribute::GroupAttribute
    FnMaterialResolveUtil::extractMaterialReferences(
            const FnAttribute::GroupAttribute & inputMaterial,
            bool processHints)
    {
        const FnMaterialResolveUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->extractMaterialReferences(inputMaterial.getHandle(),
                        processHints));
    }

    FnAttribute::GroupAttribute
    FnMaterialResolveUtil::resolveDeferredNetworkOps(
            const FnAttribute::GroupAttribute & inputMaterial)
    {
        const FnMaterialResolveUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->resolveDeferredNetworkOps(inputMaterial.getHandle()));
    }


    FnAttribute::GroupAttribute
    FnMaterialResolveUtil::resolveMaterialLayers(
            const FnAttribute::GroupAttribute & inputMaterial,
            bool processHints)
    {
        const FnMaterialResolveUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->resolveMaterialLayers(inputMaterial.getHandle(),
                        processHints));
    }


    FnAttribute::StringAttribute
    FnMaterialResolveUtil::getLayerPrefixesFromMaterial(
            const FnAttribute::GroupAttribute & inputMaterial)
    {
        const FnMaterialResolveUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::StringAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getLayerPrefixesFromMaterial(
                        inputMaterial.getHandle()));
    }

    FnAttribute::GroupAttribute
    FnMaterialResolveUtil::combineLayeredMaterialOverrides(
            const FnAttribute::GroupAttribute & inputMaterial,
            const FnAttribute::GroupAttribute & overridesAttr,
            const FnAttribute::GroupAttribute & layeredOverridesAttr)
    {
        const FnMaterialResolveUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
            suite->combineLayeredMaterialOverrides(
                    inputMaterial.getHandle(),
                    overridesAttr.getHandle(),
                    layeredOverridesAttr.getHandle()));
    }


    const FnMaterialResolveUtilHostSuite_v1 *FnMaterialResolveUtil::_getSuite()
    {
        static FnPluginManager::LazyHostSuite<FnMaterialResolveUtilHostSuite_v1>
            suite = { "MaterialResolveUtilHost", 1 };
        return suite.get();
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
