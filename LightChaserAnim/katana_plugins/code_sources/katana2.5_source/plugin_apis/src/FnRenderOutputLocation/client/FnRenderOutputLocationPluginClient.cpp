#include <FnRenderOutputLocation/FnRenderOutputLocationPluginClient.h>

FNRENDEROUTPUTLOCATION_NAMESPACE_ENTER
{
    FnRenderOutputLocationPluginClient::FnRenderOutputLocationPluginClient(
            FnRenderOutputLocationPluginSuite_v2 * suite) :
                _suite(suite), _handle(0x0)
    {
        if (!_suite)
        {
            throw std::runtime_error(
                    "FnRenderOutputLocationPluginClient constructed with no "
                    "FnRenderOutputLocation plugin API suite.");
        }

        _handle = _suite->create();
        if (!_handle)
        {
            throw std::runtime_error(
                    "FnRenderOutputLocationPluginClient failed to create "
                    "FnRenderOutputLocation plugin instance.");
        }
    }

    FnRenderOutputLocationPluginClient::~FnRenderOutputLocationPluginClient()
    {
        _suite->destroy(_handle);
    }

    FnAttribute::GroupAttribute
    FnRenderOutputLocationPluginClient::getLocationSettingsAttr(
            const std::string & outputType,
            const FnAttribute::GroupAttribute & incomingOutputAttr)
    {
        return FnAttribute::Attribute::CreateAndSteal(
            _suite->getLocationSettingsAttr(
                outputType.c_str(), incomingOutputAttr.getHandle()));
    }

    std::string FnRenderOutputLocationPluginClient::computeFilePath(
            const FnAttribute::GroupAttribute & outputAttrs,
            const FnAttribute::GroupAttribute & locationAttrs,
            const FnAttribute::GroupAttribute & renderSettingsAttrs,
            const FnAttribute::GroupAttribute & imageInfo,
            bool makeVersionTemplate)
    {
        FnAttribute::StringAttribute resultAttr =
            FnAttribute::Attribute::CreateAndSteal(
                _suite->computeFilePath(
                    _handle, outputAttrs.getHandle(),
                    locationAttrs.getHandle(),
                    renderSettingsAttrs.getHandle(),
                    imageInfo.getHandle(), makeVersionTemplate));
        return resultAttr.getValue("", false);
    }

    std::string FnRenderOutputLocationPluginClient::computeLocation(
            const FnAttribute::GroupAttribute & outputAttrs,
            const FnAttribute::GroupAttribute & locationAttrs,
            const FnAttribute::GroupAttribute & renderSettingsAttrs,
            const FnAttribute::GroupAttribute & imageInfo)
    {
        FnAttribute::StringAttribute resultAttr =
            FnAttribute::Attribute::CreateAndSteal(
                _suite->computeLocation(
                    _handle, outputAttrs.getHandle(),
                    locationAttrs.getHandle(),
                    renderSettingsAttrs.getHandle(),
                    imageInfo.getHandle()));
        return resultAttr.getValue("", false);
    }
}
FNRENDEROUTPUTLOCATION_NAMESPACE_EXIT
