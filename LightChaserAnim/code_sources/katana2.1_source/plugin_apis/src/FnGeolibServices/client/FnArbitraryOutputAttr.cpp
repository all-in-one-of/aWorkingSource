// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnGeolibServices/FnArbitraryOutputAttr.h>

#include <FnAttribute/FnAttribute.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

ArbitraryOutputAttr::ArbitraryOutputAttr(
    const std::string & arbitraryOutputName,
    const FnAttribute::GroupAttribute & arbitraryOutputAttr,
    const std::string & geometryType,
    const FnAttribute::GroupAttribute & geometryAttr) : _handle(0)
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite) return;
    _handle = suite->createArbitraryOutputAttr(
        arbitraryOutputName.c_str(), arbitraryOutputAttr.getHandle(),
        geometryType.c_str(), geometryAttr.getHandle());
}

ArbitraryOutputAttr::~ArbitraryOutputAttr()
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return;
    suite->destroyArbitraryOutputAttr(_handle);
}

std::string ArbitraryOutputAttr::getName() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getNameAOA(_handle);
}

std::string ArbitraryOutputAttr::getScope() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getScopeAOA(_handle);
}

std::string ArbitraryOutputAttr::getInputBaseType() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getInputBaseTypeAOA(_handle);
}

int ArbitraryOutputAttr::getElementSize() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return 0;
    return int(suite->getElementSizeAOA(_handle));
}

std::string ArbitraryOutputAttr::getInputType() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getInputTypeAOA(_handle);
}

std::string ArbitraryOutputAttr::getOutputType() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getOutputTypeAOA(_handle);
}

std::string ArbitraryOutputAttr::getInterpolationType() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getInterpolationTypeAOA(_handle);
}

bool ArbitraryOutputAttr::hasIndexedValueAttr() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return false;
    return bool(suite->hasIndexedValueAttrAOA(_handle));
}

FnAttribute::IntAttribute ArbitraryOutputAttr::getIndexAttr(bool validateIndices) const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return FnAttribute::IntAttribute();
    return FnAttribute::Attribute::CreateAndSteal(
            suite->getIndexAttrAOA(_handle, uint8_t(validateIndices)));
}

FnAttribute::Attribute ArbitraryOutputAttr::getIndexedValueAttr(const std::string & baseOutputType) const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return FnAttribute::Attribute();
    return FnAttribute::Attribute::CreateAndSteal(
        suite->getIndexedValueAttrAOA(_handle, baseOutputType.c_str()));
}

FnAttribute::Attribute ArbitraryOutputAttr::getValueAttr(const std::string & baseOutputType) const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return FnAttribute::Attribute();
    return FnAttribute::Attribute::CreateAndSteal(
        suite->getValueAttrAOA(_handle, baseOutputType.c_str()));
}

bool ArbitraryOutputAttr::isValid() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return false;
    return bool(suite->isValidAOA(_handle));
}

std::string ArbitraryOutputAttr::getError() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return "";
    return suite->getErrorAOA(_handle);
}

bool ArbitraryOutputAttr::isArray() const
{
    const FnArbitraryOutputAttrHostSuite_v1 * suite = _getSuite();
    if(!suite || !_handle) return false;
    return bool(suite->isArrayAOA(_handle));
}

// const FnArbitraryOutputAttrHostSuite_v1 * ArbitraryOutputAttr::_suite = 0x0;

////////////////////////////////////////////////////////////////////////////

const FnArbitraryOutputAttrHostSuite_v1 *ArbitraryOutputAttr::_getSuite()
{
    static const FnArbitraryOutputAttrHostSuite_v1 * suite = 0x0;

    if (!suite)
    {
        suite = reinterpret_cast<const FnArbitraryOutputAttrHostSuite_v1 *>(
                FnPluginManager::PluginManager::getHostSuite(
                        "ArbitraryOutputAttrHost", 1));
    }

    return suite;
}

}
FNGEOLIBSERVICES_NAMESPACE_EXIT
