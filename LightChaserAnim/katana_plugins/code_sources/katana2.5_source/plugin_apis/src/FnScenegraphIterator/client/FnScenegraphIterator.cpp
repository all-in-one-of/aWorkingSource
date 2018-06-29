// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnScenegraphIterator/FnScenegraphIterator.h>

using namespace Foundry::Katana;

FnSgIteratorHostSuite_v3 *FnScenegraphIterator::_suite = 0x0;


FnScenegraphIterator::FnScenegraphIterator(FnSgIteratorHandle handle):
    _handle(0x0)
{
    _handle = handle;
    _suite->retainSgIterator(_handle);
}

FnScenegraphIterator::FnScenegraphIterator():
        _handle(0x0)
{
}


FnScenegraphIterator::~FnScenegraphIterator()
{
    if(_handle && _suite)
    {
        _suite->releaseSgIterator(_handle);
    }
}

FnScenegraphIterator::FnScenegraphIterator(const FnScenegraphIterator& rhs) :
    _handle(0x0)
{
    acceptHandle(rhs);
}

FnScenegraphIterator & FnScenegraphIterator::operator=(const FnScenegraphIterator& rhs)
{
    acceptHandle(rhs);
    return *this;
}


void FnScenegraphIterator::acceptHandle(const FnScenegraphIterator &rhs)
{
    _suite->retainSgIterator(rhs._handle);
    if (_handle!=0x0) _suite->releaseSgIterator(_handle);
    _handle = rhs._handle;
}


std::string FnScenegraphIterator::getType() const
{
    return FnAttribute::StringAttribute(getAttribute("type")
        ).getValue("group", false);
}

std::string FnScenegraphIterator::getName() const
{
    FnAttribute::StringAttribute s =
        FnAttribute::Attribute::CreateAndSteal(_suite->getName(_handle));
    return s.getValue("", false);
}

std::string FnScenegraphIterator::getFullName() const
{
    FnAttribute::StringAttribute s =
        FnAttribute::Attribute::CreateAndSteal(_suite->getFullName(_handle));
    return s.getValue("", false);
}

FnScenegraphIterator FnScenegraphIterator::getFirstChild(bool evict) const
{
    return FnScenegraphIterator(
        _suite->getFirstChild(_handle, uint8_t(evict)));
}

FnScenegraphIterator FnScenegraphIterator::getNextSibling(bool evict) const
{
    return FnScenegraphIterator(
        _suite->getNextSibling(_handle, uint8_t(evict)));
}

FnScenegraphIterator FnScenegraphIterator::getParent() const
{
    return FnScenegraphIterator(_suite->getParent(_handle));
}

FnScenegraphIterator FnScenegraphIterator::getRoot() const
{
    return FnScenegraphIterator(_suite->getRoot(_handle));
}

FnAttribute::StringAttribute FnScenegraphIterator::getPotentialChildren() const
{
    return FnAttribute::Attribute::CreateAndSteal(
        _suite->getPotentialChildren(_handle));
}

FnScenegraphIterator FnScenegraphIterator::getChildByName(const std::string &name, bool evict) const
{
    return FnScenegraphIterator(
        _suite->getChildByName(_handle, name.c_str(), uint8_t(evict)));
}

FnScenegraphIterator FnScenegraphIterator::getByPath(const std::string &path, bool evict) const
{
    return FnScenegraphIterator(
        _suite->getByPath(_handle, path.c_str(), uint8_t(evict)));
}

FnAttribute::StringAttribute FnScenegraphIterator::getAttributeNames() const
{
    return FnAttribute::Attribute::CreateAndSteal(
        _suite->getAttributeNames(_handle));
}

FnAttribute::Attribute FnScenegraphIterator::getAttribute(const std::string &name, bool global) const
{
    FnAttributeHandle handle = 0x0;

    if (global)
    {
        handle = _suite->getGlobalAttribute(_handle, name.c_str());
    }
    else
    {
        handle = _suite->getAttribute(_handle, name.c_str());
    }

    return FnAttribute::Attribute::CreateAndSteal(handle);
}


FnAttribute::GroupAttribute FnScenegraphIterator::getGlobalXFormGroup() const
{
    return FnAttribute::Attribute::CreateAndSteal(
        _suite->getGlobalXFormGroup(_handle));
}

FnPlugStatus FnScenegraphIterator::setHost(FnPluginHost *host)
{
    if (host)
    {
        _suite = (FnSgIteratorHostSuite_v3*)host->getSuite(
            "ScenegraphIteratorHost", 3);
        if (_suite)
        {
            return FnPluginStatusOK;
        }

    }
    return FnPluginStatusError;
}

FnAttribute::GroupAttribute FnScenegraphIterator::getOpTreeDescriptionFromFile(
        const std::string & opTreeFileName)
{
    FnAttributeHandle handle =
        _suite->getOpTreeDescriptionFromFile(opTreeFileName.c_str());
    return FnAttribute::Attribute::CreateAndSteal(handle);
}

FnScenegraphIterator FnScenegraphIterator::getIteratorFromFile(
        const std::string & opTreeFileName)
{
    FnSgIteratorHandle handle =
            _suite->getIteratorFromFile(opTreeFileName.c_str());
    return FnScenegraphIterator(handle);
}

FnScenegraphIterator FnScenegraphIterator::getIteratorFromOpTreeDescription(
        const FnAttribute::GroupAttribute & opTreeDescription)
{
    FnSgIteratorHandle handle = _suite->getIteratorFromOpTreeDescription(
        opTreeDescription.getHandle());
    return FnScenegraphIterator(handle);
}
