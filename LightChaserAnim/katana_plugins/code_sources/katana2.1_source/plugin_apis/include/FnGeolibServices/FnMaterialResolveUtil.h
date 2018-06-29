#ifndef FnGeolibServicesMaterialResolveUtil_H
#define FnGeolibServicesMaterialResolveUtil_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/suite/FnMaterialResolveUtilSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FnMaterialResolveUtil
    {
    public:

        static FnAttribute::GroupAttribute resolveMaterialReferences(
                const FnAttribute::GroupAttribute & inputMaterial,
                bool processHints);

        static FnAttribute::GroupAttribute extractMaterialReferences(
                const FnAttribute::GroupAttribute & inputMaterial,
                bool processHints);

        static FnAttribute::GroupAttribute resolveDeferredNetworkOps(
                const FnAttribute::GroupAttribute & inputMaterial);

        static FnAttribute::GroupAttribute resolveMaterialLayers(
                const FnAttribute::GroupAttribute & inputMaterial,
                bool processHints);


        static FnAttribute::StringAttribute getLayerPrefixesFromMaterial(
                const FnAttribute::GroupAttribute & inputMaterial);

        static FnAttribute::GroupAttribute combineLayeredMaterialOverrides(
                const FnAttribute::GroupAttribute & inputMaterial,
                const FnAttribute::GroupAttribute & overridesAttr,
                const FnAttribute::GroupAttribute & layeredOverridesAttr);


    private:
        FnMaterialResolveUtil();

        static const FnMaterialResolveUtilHostSuite_v1 * _getSuite();
    };


}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesMaterialResolveUtil_H
