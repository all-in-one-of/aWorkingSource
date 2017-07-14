// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cstring>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginManager/FnPluginManager.h>

#include <FnDefaultAttributeProducer/plugin/FnDefaultAttributeProducerPlugin.h>

FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_ENTER
{
    FnPlugStatus DefaultAttributeProducer::setHost(FnPluginHost *host)
    {
        _host = host;
        FnPluginManager::PluginManager::setHost(host);
        FnAttribute::GroupBuilder::setHost(host);
        return FnAttribute::Attribute::setHost(host);
    }

    FnPluginHost *DefaultAttributeProducer::getHost() {return _host;}

    void DefaultAttributeProducer::flush()
    {
    }

    FnDefaultAttributeProducerSuite_v1 DefaultAttributeProducer::createSuite(
            FnAttributeHandle (*cook)(
                     FnGeolibCookInterfaceHandle cookInterfaceHandle,
                     FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
                     const char *attrRoot, int32_t stringlen1,
                     const char *inputLocationPath, int32_t stringlen2,
                     int32_t inputIndex,
                     uint8_t * didAbort))
    {
        FnDefaultAttributeProducerSuite_v1 suite = {
            cook
        };

        return suite;
    }

    FnAttribute::GroupAttribute DefaultAttributeProducer::callCook(
        CookFunc cookFunc,
        FnGeolibCookInterfaceHandle cookInterfaceHandle,
        FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
        const char *attrRoot, int32_t stringlen1,
        const char *inputLocationPath, int32_t stringlen2,
        int32_t inputIndex,
        uint8_t * didAbort)
    {
        *didAbort = false;
        FnGeolibOp::GeolibCookInterface cookInterface(
            cookInterfaceHandle, cookInterfaceSuite);

        std::string attrRootStr(attrRoot, stringlen1);
        std::string inputLocationPathStr(inputLocationPath, stringlen2);

        try
        {
            return cookFunc(cookInterface, attrRootStr,
                     inputLocationPathStr, inputIndex);
        }
        catch (FnGeolibOp::GeolibCookInterface::QueryAbortException &)
        {
            *didAbort = true;
        }
        catch (std::exception & e)
        {
            std::string errorMsg = cookInterface.getOpType();
            errorMsg += " Op: ";
            errorMsg += e.what();
            cookInterfaceSuite->setAttr(cookInterfaceHandle,
                "dapErrorMessage", strlen("dapErrorMessage"),
                FnAttribute::StringAttribute(errorMsg).getHandle(), true);
        }
        catch (...)
        {
            std::string errorMsg = cookInterface.getOpType();
            errorMsg += " Op: Unknown exception during DAP cook";
            cookInterfaceSuite->setAttr(cookInterfaceHandle,
                "dapErrorMessage", strlen("dapErrorMessage"),
                FnAttribute::StringAttribute(errorMsg).getHandle(), true);
        }
        return FnAttribute::GroupAttribute();
    }

    unsigned int DefaultAttributeProducer::_apiVersion = 1;
    const char* DefaultAttributeProducer::_apiName = "DefaultAttributeProducer";
    FnPluginHost *DefaultAttributeProducer::_host = 0x0;
}
FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_EXIT
