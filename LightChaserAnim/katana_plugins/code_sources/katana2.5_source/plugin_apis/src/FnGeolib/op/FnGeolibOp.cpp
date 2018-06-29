// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cstring>
#include <iostream>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginManager/FnPluginManager.h>

#include <FnGeolib/op/FnGeolibOp.h>

#ifdef FNGEOLIBOP_INIT_FNASSET_HOSTS
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>
#endif // FNGEOLIBOP_INIT_FNASSET_HOSTS

FNGEOLIBOP_NAMESPACE_ENTER
{
        FnPlugStatus GeolibOp::setHost(FnPluginHost *host)
        {
            _host = host;
            FnPluginManager::PluginManager::setHost(host);
            FnAttribute::GroupBuilder::setHost(host);

            #ifdef FNGEOLIBOP_INIT_FNASSET_HOSTS
                FnAsset::DefaultAssetPlugin::setHost(host);
                FnAsset::DefaultFileSequencePlugin::setHost(host);
            #endif

            return FnAttribute::Attribute::setHost(host);
        }

        FnPluginHost *GeolibOp::getHost() {return _host;}

        void GeolibOp::flush()
        {
        }

        FnGeolibOpSuite_v1 GeolibOp::createSuite(
            void (*setup)(FnGeolibSetupInterfaceHandle setupInterfaceHandle,
                         FnGeolibSetupInterfaceSuite_v1 *setupInterfaceSuite),
            void (*cook)(FnGeolibCookInterfaceHandle cookInterfaceHandle,
                         FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
                         uint8_t * didAbort),
            FnAttributeHandle (*describe)())
        {
            FnGeolibOpSuite_v1 suite = {
                setup,
                cook,
                describe
            };

            return suite;
        }

        void GeolibOp::callSetup(
            void (*setupFunc)(GeolibSetupInterface &setupInterface),
            FnGeolibSetupInterfaceHandle setupInterfaceHandle,
            FnGeolibSetupInterfaceSuite_v1 *setupInterfaceSuite)
        {
            GeolibSetupInterface setupInterface(
                setupInterfaceHandle, setupInterfaceSuite);

            try
            {
                setupFunc(setupInterface);
            }
            catch (...)
            {
                // TODO: What to do if setup throws an exception?
                //*didAbort = true;
            }
        }

        void GeolibOp::callCook(
            void (*cookFunc)(GeolibCookInterface &cookInterface),
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
            uint8_t * didAbort)
        {
            *didAbort = false;
            GeolibCookInterface cookInterface(
                cookInterfaceHandle, cookInterfaceSuite);

            try
            {
                cookFunc(cookInterface);
            }
            catch (GeolibCookInterface::QueryAbortException &)
            {
                *didAbort = true;
            }
            catch (std::exception & e)
            {
                cookInterfaceSuite->setAttr(cookInterfaceHandle,
                    "type", static_cast<int32_t>(strlen("type")),
                    FnAttribute::StringAttribute("error").getHandle(), true);
                std::string errorMsg = cookInterface.getOpType();
                errorMsg += " Op: ";
                errorMsg += e.what();
                cookInterfaceSuite->setAttr(cookInterfaceHandle,
                    "errorMessage",
                    static_cast<int32_t>(strlen("errorMessage")),
                    FnAttribute::StringAttribute(errorMsg).getHandle(), true);
            }
            catch (...)
            {
                cookInterfaceSuite->setAttr(cookInterfaceHandle,
                    "type", static_cast<int32_t>(strlen("type")),
                    FnAttribute::StringAttribute("error").getHandle(), true);

                std::string errorMsg = cookInterface.getOpType();
                errorMsg += " Op: Unknown exception during op cook";

                cookInterfaceSuite->setAttr(cookInterfaceHandle,
                    "errorMessage",
                    static_cast<int32_t>(strlen("errorMessage")),
                    FnAttribute::StringAttribute(errorMsg).getHandle(), true);
            }
        }

        FnAttributeHandle GeolibOp::callDescribe(
                FnAttribute::GroupAttribute (*describeFunc)())
        {
            try
            {
                FnAttribute::Attribute attr = describeFunc();
                return attr.getRetainedHandle();
            }
            catch (std::exception & e)
            {
                std::cerr << "GeolibOp::describe() failed: "
                          << e.what() << std::endl;
            }

            return 0x0;
        }

        FnAttribute::GroupAttribute GeolibOp::describe()
        {
            return FnAttribute::GroupAttribute();
        }

        unsigned int GeolibOp::_apiVersion = 1;
        const char* GeolibOp::_apiName = "GeolibOp";
        FnPluginHost *GeolibOp::_host = 0x0;

}
FNGEOLIBOP_NAMESPACE_EXIT
