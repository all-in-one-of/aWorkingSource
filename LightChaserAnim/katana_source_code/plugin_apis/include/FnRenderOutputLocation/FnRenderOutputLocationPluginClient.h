#ifndef FnRenderOutputLocationPluginClient_H
#define FnRenderOutputLocationPluginClient_H

#include "ns.h"

#include <FnRenderOutputLocation/suite/FnRenderOutputLocationSuite.h>

#include <FnAttribute/FnAttribute.h>

#include <tr1/memory>
#define PLUGIN_SHARED_PTR std::tr1::shared_ptr

FNRENDEROUTPUTLOCATION_NAMESPACE_ENTER
{
    // Class that wraps a specific instance of
    // FnRenderOutputLocationPluginSuite_v2
    class FnRenderOutputLocationPluginClient
    {
        public:
            FnRenderOutputLocationPluginClient(
                    FnRenderOutputLocationPluginSuite_v2 * suite);
            ~FnRenderOutputLocationPluginClient();

            typedef PLUGIN_SHARED_PTR<FnRenderOutputLocationPluginClient> Ptr;

            FnAttribute::GroupAttribute getLocationSettingsAttr(
                    const std::string & outputType,
                    const FnAttribute::GroupAttribute & incomingOutputAttr);

            std::string computeFilePath(
                    const FnAttribute::GroupAttribute & outputAttrs,
                    const FnAttribute::GroupAttribute & locationAttrs,
                    const FnAttribute::GroupAttribute & renderSettingsAttrs,
                    const FnAttribute::GroupAttribute & imageInfo,
                    bool makeVersionTemplate);

            std::string computeLocation(
                    const FnAttribute::GroupAttribute & outputAttrs,
                    const FnAttribute::GroupAttribute & locationAttrs,
                    const FnAttribute::GroupAttribute & renderSettingsAttrs,
                    const FnAttribute::GroupAttribute & imageInfo);

        private:
            FnRenderOutputLocationPluginSuite_v2 *_suite;
            FnRenderOutputLocationPluginHandle _handle;

            // no copy/assign
            FnRenderOutputLocationPluginClient(
                    const FnRenderOutputLocationPluginClient &rhs);
            FnRenderOutputLocationPluginClient &operator=(
                    const FnRenderOutputLocationPluginClient &rhs);
    };
}
FNRENDEROUTPUTLOCATION_NAMESPACE_EXIT

#endif // FnRenderOutputLocationPluginClient_H
