// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <stdio.h>
#include <stdexcept>

#include <FnGeolib/op/FnGeolibCookInterface.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    void FnGeolibCookInterfaceUtils::matchesCEL(
            MatchesCELInfo & matchesCELInfo,
            const FnGeolibOp::GeolibCookInterface & interface,
            const FnAttribute::StringAttribute & cel,
            const std::string & inputLocationPath, int inputIndex)
    {
        matchesCELInfo.matches = false;
        matchesCELInfo.canMatchChildren = false;

        const FnGeolibCookInterfaceUtilsHostSuite_v2 * suite = _getSuite();
        if (!suite) return;

        struct FnGeolibCookInterfaceUtilsMatchesCELInfo cMatchesCELInfo;

        uint8_t didAbort = false;
        suite->matchesCEL(&cMatchesCELInfo,
                interface.getHandle(), interface.getSuite(),
                cel.getHandle(),
                inputLocationPath.c_str(),
                static_cast<int32_t>(inputLocationPath.size()),
                inputIndex, &didAbort);

        if (didAbort) throw FnGeolibOp::GeolibCookInterface::QueryAbortException();

        matchesCELInfo.matches = cMatchesCELInfo.matches;
        matchesCELInfo.canMatchChildren = cMatchesCELInfo.canMatchChildren;
    }

    FnAttribute::GroupAttribute FnGeolibCookInterfaceUtils::cookDaps(
            const FnGeolibOp::GeolibCookInterface & interface,
            const std::string & attrRoot,
            const std::string & inputLocationPath, int inputIndex,
            const FnAttribute::Attribute & cookOrderAttr)
    {
        const FnGeolibCookInterfaceUtilsHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        uint8_t didAbort = false;
        FnAttributeHandle attrHandle = suite->cookDaps(
                interface.getHandle(), interface.getSuite(),
                attrRoot.c_str(), static_cast<int32_t>(attrRoot.size()),
                inputLocationPath.c_str(),
                static_cast<int32_t>(inputLocationPath.size()),
                inputIndex,
                cookOrderAttr.getHandle(),
                &didAbort);

        if (didAbort) throw FnGeolibOp::GeolibCookInterface::QueryAbortException();
        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

   FnGeolibCookInterfaceUtils::BuildLocalsAndGlobalsInfo
            FnGeolibCookInterfaceUtils::buildLocalsAndGlobals(
                    const FnGeolibOp::GeolibCookInterface & interface,
                    const std::string & attrRoot)
    {
        BuildLocalsAndGlobalsInfo info;

        const FnGeolibCookInterfaceUtilsHostSuite_v2 * suite = _getSuite();
        if (!suite) return info;

        uint8_t didAbort = false;
        struct FnGeolibCookInterfaceUtilsBuildLocalsAndGlobalsInfo cInfo;
        suite->buildLocalsAndGlobals(&cInfo,
                interface.getHandle(), interface.getSuite(),
                attrRoot.c_str(), static_cast<int32_t>(attrRoot.size()),
                &didAbort);

        if (didAbort) throw FnGeolibOp::GeolibCookInterface::QueryAbortException();
        info.locals = FnAttribute::Attribute::CreateAndSteal(cInfo.locals);
        info.globals = FnAttribute::Attribute::CreateAndSteal(cInfo.globals);
        return info;
    }

    FnAttribute::Attribute FnGeolibCookInterfaceUtils::getDefaultDapCookOrder()
    {
        const FnGeolibCookInterfaceUtilsHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getDefaultDapCookOrder());
    }



    FnAttribute::GroupAttribute
    FnGeolibCookInterfaceUtils::resolveFileNamesWithinGroup(
            const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
            const FnAttribute::GroupAttribute & inputGroup,
            const FnAttribute::StringAttribute & celAttr,
            int frameNumber)
    {
        const FnGeolibCookInterfaceUtilsHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();

        uint8_t didAbort = false;
        FnAttributeHandle attrHandle = suite->resolveFileNamesWithinGroup(
                interface.getHandle(), interface.getSuite(),
                inputGroup.getHandle(), celAttr.getHandle(),
                frameNumber, &didAbort);

        if (didAbort) throw FnGeolibOp::GeolibCookInterface::QueryAbortException();
        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    FnAttribute::StringAttribute
    FnGeolibCookInterfaceUtils::resolveFileNameInString(
            const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
            const FnAttribute::StringAttribute & inputString,
            int frameNumber)
    {
        const FnGeolibCookInterfaceUtilsHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();

        uint8_t didAbort = false;
        FnAttributeHandle attrHandle = suite->resolveFileNameInString(
                interface.getHandle(), interface.getSuite(),
                inputString.getHandle(), frameNumber, &didAbort);

        if (didAbort) throw FnGeolibOp::GeolibCookInterface::QueryAbortException();
        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    struct GlobalAttributeProviderAdapter
    {
        explicit GlobalAttributeProviderAdapter(
                BaseGlobalAttributeProvider *provider)
            : _provider(provider)
        {
        }

        static FnGeolibGlobalAttributeProviderStatus
        GlobalAttributeProviderCallback(void *context,
                                        const char *attrPath,
                                        int32_t attrPathLen,
                                        const char *leafLocationPath,
                                        int32_t leafLocationPathLen,
                                        int32_t relativeDepth,
                                        FnAttributeHandle *attributeHandlePtr)
        {
            GlobalAttributeProviderAdapter *const self =
                    static_cast<GlobalAttributeProviderAdapter *>(context);
            try
            {
                FnAttribute::Attribute attr;
                FnGeolibGlobalAttributeProviderStatus status =
                        self->_provider->provideAttribute(
                                std::string(attrPath, attrPathLen),
                                std::string(leafLocationPath,
                                            leafLocationPathLen),
                                relativeDepth, &attr);

                *attributeHandlePtr = attr.getRetainedHandle();
                return status;
            }
            catch (const std::exception &e)
            {
                fprintf(stderr, "GlobalAttributeProvider exception: %s\n",
                        e.what());
                return kFnGeolibGlobalAttributeProviderStatus_InvalidLocation;
            }
            catch (...)
            {
                fprintf(stderr,
                        "GlobalAttributeProvider exception: Unknown "
                        "exception\n");
                return kFnGeolibGlobalAttributeProviderStatus_InvalidLocation;
            }
        }

        BaseGlobalAttributeProvider *_provider;
    };

    FnAttribute::Attribute FnGeolibCookInterfaceUtils::getGlobalAttrGeneric(
            BaseGlobalAttributeProvider & provider, const std::string &name,
            const std::string &locationPath, bool *didAbortPtr)
    {
        const FnGeolibCookInterfaceUtilsHostSuite_v2 *suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();

        GlobalAttributeProviderAdapter providerAdapter(&provider);

        uint8_t didAbort = false;
        FnAttribute::Attribute attr = FnAttribute::Attribute::CreateAndSteal(
                suite->getGlobalAttrGeneric(
                        providerAdapter.GlobalAttributeProviderCallback,
                        &providerAdapter, name.c_str(),
                        static_cast<int32_t>(name.size()), locationPath.c_str(),
                        static_cast<int32_t>(locationPath.size()), &didAbort));
        if (didAbortPtr) *didAbortPtr = didAbort;

        return attr;
    }

    const FnGeolibCookInterfaceUtilsHostSuite_v2 *FnGeolibCookInterfaceUtils::_getSuite()
    {
        static const FnGeolibCookInterfaceUtilsHostSuite_v2 *suite =
                static_cast<const FnGeolibCookInterfaceUtilsHostSuite_v2 *>(
                        FnPluginManager::PluginManager::getHostSuite(
                                "GeolibCookInterfaceUtilsHost", 2));
        return suite;
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
