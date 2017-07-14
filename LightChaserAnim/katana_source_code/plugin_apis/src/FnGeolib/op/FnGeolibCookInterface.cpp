// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cstdlib>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnGeolib/op/FnGeolibCookInterface.h>

#include <FnGeolib/util/Path.h>


// Performance testing on scenes from SPI suggest 512 is a reasonable upper
// bound (paths of length ~300 are common for big scenes)
#define DEFAULT_PATH_BUFFER_SIZE 512

FNGEOLIBOP_NAMESPACE_ENTER
{

    GeolibCookInterface::GeolibCookInterface(FnGeolibCookInterfaceHandle interfaceHandle,
        FnGeolibCookInterfaceSuite_v1 *interfaceSuite) :
        _handle(interfaceHandle), _suite(interfaceSuite)
    {
    }

    std::string GeolibCookInterface::getOutputName() const
    {
        int32_t stringlen = 0;
        const char * str = _suite->getOutputName(_handle, &stringlen);
        return std::string(str, stringlen);
    }

    std::string GeolibCookInterface::getInputName() const
    {
        int32_t stringlen = 0;
        const char * str = _suite->getInputName(_handle, &stringlen);
        return std::string(str, stringlen);
    }

    std::string GeolibCookInterface::getOutputLocationPath() const
    {
        char buffer[DEFAULT_PATH_BUFFER_SIZE];

        int32_t charswritten = _suite->getOutputLocationPath(_handle, buffer, DEFAULT_PATH_BUFFER_SIZE);
        if(charswritten < DEFAULT_PATH_BUFFER_SIZE)
        {
            return std::string(buffer, charswritten);
        }
        else
        {
          char *tmpBuffer = (char*)malloc(charswritten + 1);
          charswritten = _suite->getOutputLocationPath(_handle, tmpBuffer, charswritten + 1);
          std::string s(tmpBuffer, charswritten);
          free(tmpBuffer);
          return s;
        }

    }

    std::string GeolibCookInterface::getInputLocationPath() const
    {
        char buffer[DEFAULT_PATH_BUFFER_SIZE];
        int32_t charswritten = _suite->getInputLocationPath(_handle, buffer, DEFAULT_PATH_BUFFER_SIZE);
        if(charswritten < DEFAULT_PATH_BUFFER_SIZE)
        {
            return std::string(buffer, charswritten);
        }
        else
        {
            char *tmpBuffer = (char*)malloc(charswritten + 1);
            charswritten = _suite->getInputLocationPath(_handle, tmpBuffer, charswritten + 1);
            std::string s(tmpBuffer, charswritten);
            free(tmpBuffer);
            return s;
        }
    }

    std::string GeolibCookInterface::getRelativeOutputLocationPath() const
    {
        char buffer[DEFAULT_PATH_BUFFER_SIZE];
        int32_t charswritten = _suite->getRelativeOutputLocationPath(
            _handle, buffer, DEFAULT_PATH_BUFFER_SIZE);
        if(charswritten < DEFAULT_PATH_BUFFER_SIZE)
        {
            return std::string(buffer, charswritten);
        }
        else
        {
            char *tmpBuffer = (char*)malloc(charswritten + 1);
            charswritten = _suite->getRelativeOutputLocationPath(_handle, tmpBuffer, charswritten + 1);
            std::string s(tmpBuffer, charswritten);
            free(tmpBuffer);
            return s;
        }
    }

    std::string GeolibCookInterface::getRelativeInputLocationPath() const
    {
        char buffer[DEFAULT_PATH_BUFFER_SIZE];
        int32_t charswritten = _suite->getRelativeInputLocationPath(
            _handle, buffer, DEFAULT_PATH_BUFFER_SIZE);
        if(charswritten < DEFAULT_PATH_BUFFER_SIZE)
        {
            return std::string(buffer, charswritten);
        }
        else
        {
            char *tmpBuffer = (char*)malloc(charswritten + 1);
            charswritten = _suite->getRelativeInputLocationPath(_handle, tmpBuffer, charswritten + 1);
            std::string s(tmpBuffer, charswritten);
            free(tmpBuffer);
            return s;
        }

    }

    std::string GeolibCookInterface::getRootLocationPath() const
    {
        char buffer[DEFAULT_PATH_BUFFER_SIZE];
        int32_t charswritten = _suite->getRootLocationPath(_handle, buffer,
            DEFAULT_PATH_BUFFER_SIZE);
        if(charswritten < DEFAULT_PATH_BUFFER_SIZE)
        {
          return std::string(buffer, charswritten);
        }
        else
        {
            char *tmpBuffer = (char*)malloc(charswritten + 1);
            charswritten = _suite->getRootLocationPath(_handle, tmpBuffer, charswritten + 1);
            std::string s(tmpBuffer, charswritten);
            free(tmpBuffer);
            return s;
        }
    }

    bool GeolibCookInterface::atRoot() const
    {
        return static_cast<bool>(_suite->atRoot(_handle));
    }

    int GeolibCookInterface::getNumInputs() const
    {
        return _suite->getNumInputs(_handle);
    }

    int GeolibCookInterface::getInputIndex() const
    {
        return _suite->getInputIndex(_handle);
    }

    std::string GeolibCookInterface::getOpType() const
    {
        int32_t stringlen = 0;
        const char * str = _suite->getOpType(_handle, &stringlen);
        return std::string(str, stringlen);
    }

    FnAttribute::Attribute GeolibCookInterface::getOpArg(
        const std::string & specificArgName) const
    {
        FnAttributeHandle attrHandle = _suite->getOpArg(_handle,
            specificArgName.c_str(), (int32_t) specificArgName.size());
        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    void * GeolibCookInterface::getPrivateData() const
    {
        return _suite->getPrivateData(_handle);
    }

    void GeolibCookInterface::prefetch(
        const std::string & inputLocationPath, int inputIndex) const
    {
        _suite->prefetch(_handle,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex);
    }

    FnAttribute::Attribute GeolibCookInterface::getAttr(
        const std::string &attrname,
        const std::string &inputLocationPath, int inputIndex) const
    {
        uint8_t didAbort = false;
        FnAttributeHandle attrHandle = _suite->getAttr(
                _handle, attrname.c_str(), (int32_t) attrname.size(),
                inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
                inputIndex, &didAbort);
        if (didAbort) throw QueryAbortException();
        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    bool GeolibCookInterface::doesLocationExist(
        const std::string & inputLocationPath, int inputIndex) const
    {
        uint8_t didAbort = false;
        uint8_t exists = _suite->doesLocationExist(_handle,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex, &didAbort);
        if (didAbort) throw QueryAbortException();
        return exists;
    }

    FnAttribute::StringAttribute GeolibCookInterface::getPotentialChildren(
        const std::string & inputLocationPath, int inputIndex) const
    {
        uint8_t didAbort = false;
        FnAttributeHandle attrHandle =_suite->getPotentialChildren(_handle,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex, &didAbort);
        if (didAbort)
        {
            throw QueryAbortException();
        }

        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    FnAttribute::Attribute GeolibCookInterface::getOutputAttr(
        const std::string & attrname) const
    {
        return FnAttribute::Attribute::CreateAndSteal(
            _suite->getOutputAttr(_handle,
            attrname.c_str(), (int32_t) attrname.size()));
    }

    void GeolibCookInterface::createChild(const std::string & name,
        const std::string & optype,
        const FnAttribute::Attribute & args,
        ResetRoot resetRoot_,
        void *privateData,
        void (*deletePrivateData)(void *data))
    {
        if (name.empty() || name.find('/') != std::string::npos)
        {
            throw std::runtime_error("createChild(): "
                    "Invalid child name: \"" + name + "\"");
        }
        if (privateData && !deletePrivateData)
        {
            throw std::runtime_error(
                "Private data provided for child without deletion method.");
        }
        _suite->createChild(_handle,
            name.c_str(), (int32_t) name.size(),
            optype.c_str(), (int32_t) optype.size(),
            args.getHandle(),
            static_cast<int32_t>(resetRoot_),
            privateData, deletePrivateData);
    }

    void GeolibCookInterface::deleteSelf()
    {
        _suite->deleteSelf(_handle);
    }

    void GeolibCookInterface::replaceChildren(
        const std::string & inputLocationPath, int inputIndex)
    {
        uint8_t didAbort = false;
        _suite->replaceChildren(_handle,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex, &didAbort);
        if (didAbort) throw QueryAbortException();
    }

    void GeolibCookInterface::deleteChildren()
    {
        _suite->deleteChildren(_handle);
    }

    void GeolibCookInterface::deleteChild(const std::string & child)
    {
        _suite->deleteChild(_handle,
            child.c_str(), (int32_t) child.size());
    }

    void GeolibCookInterface::copyLocationToChild(const std::string & child,
            const std::string & inputLocationPath,
            int inputIndex,
            const std::string & orderBefore)
    {
        uint8_t didAbort = false;
        _suite->copyLocationToChild(_handle,
            child.c_str(), (int32_t) child.size(),
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            (int32_t) inputIndex,
            orderBefore.c_str(), (int32_t) orderBefore.size(), &didAbort);
        if (didAbort) throw QueryAbortException();
    }

    void GeolibCookInterface::setAttr(const std::string & attrname,
        const FnAttribute::Attribute & value, const bool groupInherit)
    {
        _suite->setAttr(_handle,
            attrname.c_str(), (int32_t) attrname.size(),
            value.getHandle(),
            (uint8_t) groupInherit);
    }

    void GeolibCookInterface::copyAttr(const std::string & dstattrname,
        const std::string & srcattrname,
        const bool groupInherit,
        const std::string & inputLocationPath,
        int inputIndex)
    {
        uint8_t didAbort = false;
        _suite->copyAttr(_handle,
            dstattrname.c_str(), (int32_t) dstattrname.size(),
            srcattrname.c_str(), (int32_t) srcattrname.size(),
            (uint8_t) groupInherit,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex, &didAbort);
        if (didAbort) throw QueryAbortException();
    }

    void GeolibCookInterface::extendAttr(const std::string & dstattrname,
        const FnAttribute::Attribute & value,
        const std::string & srcattrname,
        const bool groupInherit,
        const std::string & inputLocationPath,
        int inputIndex)
    {
        uint8_t didAbort = false;
        _suite->extendAttr(_handle,
            dstattrname.c_str(), (int32_t) dstattrname.size(),
            value.getHandle(),
            srcattrname.c_str(), (int32_t) srcattrname.size(),
            (uint8_t) groupInherit,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex, &didAbort);
        if (didAbort) throw QueryAbortException();
    }

    void GeolibCookInterface::deleteAttr(const std::string & attrname)
    {
        _suite->deleteAttr(_handle,
            attrname.c_str(), (int32_t) attrname.size());
    }

    void GeolibCookInterface::deleteAttrs()
    {
        _suite->deleteAttrs(_handle);
    }

    void GeolibCookInterface::replaceAttrs(
        const std::string & inputLocationPath, int inputIndex)
    {
        uint8_t didAbort = false;
        _suite->replaceAttrs(_handle,
            inputLocationPath.c_str(), (int32_t) inputLocationPath.size(),
            inputIndex, &didAbort);
        if (didAbort) throw QueryAbortException();
    }

    void GeolibCookInterface::stopChildTraversal()
    {
        _suite->stopChildTraversal(_handle);
    }

    void GeolibCookInterface::execOp(const std::string & optype,
        const FnAttribute::GroupAttribute & args)
    {
        uint8_t didAbort = false;
        _suite->execOp(_handle,
            optype.c_str(), (int32_t) optype.size(),
            args.getHandle(), &didAbort);

        if (didAbort)
        {
            throw QueryAbortException();
        }
    }

    void GeolibCookInterface::replaceChildTraversalOp(
        const std::string & optype,
        const FnAttribute::GroupAttribute & args,
        void *privateData, void (*deletePrivateData)(void *))
    {
        _suite->replaceChildTraversalOp(_handle,
            optype.c_str(), (int32_t) optype.size(),
            args.getHandle(),
            privateData, deletePrivateData);
    }

    void GeolibCookInterface::resetRoot()
    {
        _suite->resetRoot(_handle);
    }
}
FNGEOLIBOP_NAMESPACE_EXIT
