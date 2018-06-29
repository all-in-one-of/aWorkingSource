// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnGeolib/op/FnGeolibSetupInterface.h>

FNGEOLIBOP_NAMESPACE_ENTER
{

    GeolibSetupInterface::GeolibSetupInterface(
        FnGeolibSetupInterfaceHandle interfaceHandle,
        FnGeolibSetupInterfaceSuite_v1 *interfaceSuite) :
        _handle(interfaceHandle), _suite(interfaceSuite)
    {
    }

    void GeolibSetupInterface::setThreading(GeolibSetupInterface::ThreadMode threadMode)
    {
        _suite->setThreading(_handle, static_cast<int32_t>(threadMode));
    }
}
FNGEOLIBOP_NAMESPACE_EXIT
