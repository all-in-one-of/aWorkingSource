// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnAttributeModifier/plugin/FnAttributeModifier.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnLogging/FnLogging.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>


#include <memory>
#include <vector>
#include <string>

#include <iostream>

struct FnAttributeModifierStruct
{
public:
    FnAttributeModifierStruct(Foundry::Katana::AttributeModifier *amp);
    ~FnAttributeModifierStruct();
    Foundry::Katana::AttributeModifier &getAttributeModifier() {return *_attributeModifier;}

    std::string &cacheIdCache();
private:
    std::auto_ptr<Foundry::Katana::AttributeModifier> _attributeModifier;
    std::auto_ptr<std::string> _cacheIdCache;
};

FnAttributeModifierStruct::FnAttributeModifierStruct(Foundry::Katana::AttributeModifier *am) :
    _attributeModifier(am)
{
}

FnAttributeModifierStruct::~FnAttributeModifierStruct()
{
}

std::string &FnAttributeModifierStruct::cacheIdCache()
{
    if (!_cacheIdCache.get())
    {
        _cacheIdCache.reset(new std::string);
    }
    return *_cacheIdCache;
}


// Implement C callbacks for the plugin suite

void destroy(FnAttributeModifierHandle handle)
{
    delete handle;
}

void cook(FnAttributeModifierHandle handle,FnAttributeModifierHostHandle hostHandle)
{
    Foundry::Katana::AttributeModifierInput input( hostHandle );

    // setHost() has been set here instead of in the AttributeModifier setHost()
    // because if we load any AttributeModifiers BEFORE the
    // AttributeModifieHost Op is loaded, we will have a null pointer passed
    // as the host suite. This will cause the AMP to crash when used. So,
    // instead, we setHost() here each time before the AMP is used, and check
    // inside AttributeModifierInput::setHost() so that we only set the host
    // once.
    // TODO: find a better solution.
    Foundry::Katana::AttributeModifierInput::setHost(
            Foundry::Katana::AttributeModifier::getHost());
    handle->getAttributeModifier().cook( input );
}

const char* getCacheId(FnAttributeModifierHandle handle)
{
    std::string &cacheIdCache = handle->cacheIdCache();
    cacheIdCache = handle->getAttributeModifier().getCacheId();

    return cacheIdCache.c_str();
}

namespace Foundry
{
    namespace Katana
    {
        std::string AttributeModifier::getCacheId(){
            return std::string(_args.getXML() );
        }


        FnPlugStatus AttributeModifier::setHost(FnPluginHost *host)
        {
            _host = host;
            AttributeModifierInput::setHost(host);
            GroupBuilder::setHost(host);
            FnLogging::setHost(host);
            DefaultAssetPlugin::setHost(host);
            DefaultFileSequencePlugin::setHost(host);
            return Attribute::setHost(host);
        }


        FnPluginHost *AttributeModifier::getHost() {return _host;}

        FnAttributeModifierHandle AttributeModifier::newAttributeModifierHandle(AttributeModifier *am)
        {
            if (!am) return 0x0;

            FnAttributeModifierHandle h = (FnAttributeModifierHandle)new FnAttributeModifierStruct(am);
            return h;
        }

        FnAttributeModifierPluginSuite_v1 AttributeModifier::createSuite(
                    FnAttributeModifierHandle (*create)(FnAttributeHandle),
                    FnAttributeHandle (*getArgumentTemplate)() )
        {

            FnAttributeModifierPluginSuite_v1 suite = {
                create,
                getArgumentTemplate,

                ::destroy,
                ::cook,
                ::getCacheId,
            };

            return suite;
        }

        unsigned int AttributeModifier::_apiVersion = 1;
        const char* AttributeModifier::_apiName = "AttributeModifierPlugin";
        FnPluginHost *AttributeModifier::_host = 0x0;

    }//namespace
}//namespace
