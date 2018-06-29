// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnMaterialResolveUtilSuite_H
#define FnMaterialResolveUtilSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnMaterialResolveUtilHostSuite_version 1

/** @brief MaterialResolveUtil host suite
*/
struct FnMaterialResolveUtilHostSuite_v1
{

    FnAttributeHandle (*resolveMaterialReferences)(
            FnAttributeHandle inputMaterial, bool processHints);

    FnAttributeHandle (*extractMaterialReferences)(
            FnAttributeHandle inputMaterial, bool processHints);

    FnAttributeHandle (*resolveDeferredNetworkOps)(
            FnAttributeHandle inputMaterial);

    FnAttributeHandle (*resolveMaterialLayers)(
            FnAttributeHandle inputMaterial, bool processHints);

    FnAttributeHandle (*getLayerPrefixesFromMaterial)(
            FnAttributeHandle inputMaterial);

    FnAttributeHandle (*combineLayeredMaterialOverrides)(
            FnAttributeHandle inputMaterial,
            FnAttributeHandle overridesAttr,
            FnAttributeHandle layeredOverridesAttr);

};

}
#endif // FnMaterialResolveUtilSuite_H
