// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnScenegraphGenerator/plugin/FnScenegraphGenerator.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnLogging/FnLogging.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>


#include <memory>
#include <vector>
#include <string>

#include <iostream>



struct FnScenegraphGeneratorStruct
{
public:
    FnScenegraphGeneratorStruct(Foundry::Katana::ScenegraphGenerator *sg);
    ~FnScenegraphGeneratorStruct();
    Foundry::Katana::ScenegraphGenerator &getScenegraphGenerator() {return *_scenegraphGenerator;}
private:
    std::auto_ptr<Foundry::Katana::ScenegraphGenerator> _scenegraphGenerator;
};

// --- FnSGContextStruct -------------------------------------------------------------

struct FnSGContextStruct
{
public:
    FnSGContextStruct(Foundry::Katana::ScenegraphContext *context);
    ~FnSGContextStruct();
    Foundry::Katana::ScenegraphContext &getSGContext() {return *_sgContext;}

    std::vector<std::string> &attrNameCache();
    std::vector<const char *> &attrNameCharCache();
private:
    std::auto_ptr<Foundry::Katana::ScenegraphContext> _sgContext;
    std::auto_ptr<std::vector<std::string> > _attrNameCache;
    std::auto_ptr<std::vector<const char *> > _attrNameCharCache;
};

#ifdef D_UNUSED
namespace // anonymous
{
    //TODO: temp?  this is for debugging/verifying object lifetime
    class PrintObjectCountsOnUnload
    {
    public:
        PrintObjectCountsOnUnload() : _countSG(0), _countContext(0) {}
        ~PrintObjectCountsOnUnload()
        {
            /*
            std::cerr << "Final '" << Foundry::Katana::ScenegraphGenerator::pluginName
                      << "' FnScenegraphGeneratorStruct count: " << _countSG << std::endl;
            std::cerr << "Final '" << Foundry::Katana::ScenegraphGenerator::pluginName
                      << "' Foundry::Katana::ScenegraphContext count: " << _countContext << std::endl;
            */
        }
        void incrSG(void *ptr)
        {
            ++_countSG;
            //std::cerr << "+ FnScenegraphGeneratorStruct: 0x" << ptr << ", total count: " << _countSG << std::endl;
        }
        void decrSG(void *ptr)
        {
            --_countSG;
            //std::cerr << "- FnScenegraphGeneratorStruct: 0x" << ptr << ", total count: " << _countSG << std::endl;
        }
        void incrContext(void *ptr)
        {
            ++_countContext;
            //std::cerr << "+ FnSGContextStruct: 0x" << ptr << ", total count: " << _countContext << std::endl;
        }
        void decrContext(void *ptr)
        {
            --_countContext;
            //std::cerr << "- FnSGContextStruct: 0x" << ptr << ", total count: " << _countContext << std::endl;
        }
    private:
        int _countSG;
        int _countContext;
    };
    //PrintObjectCountsOnUnload g_printObjectCountsOnUnload;
}
#endif //D_UNUSED

FnScenegraphGeneratorStruct::FnScenegraphGeneratorStruct(Foundry::Katana::ScenegraphGenerator *sg) :
    _scenegraphGenerator(sg)
{
    //g_printObjectCountsOnUnload.incrSG(this);
}

FnScenegraphGeneratorStruct::~FnScenegraphGeneratorStruct()
{
    //g_printObjectCountsOnUnload.decrSG(this);
}

FnSGContextStruct::FnSGContextStruct(Foundry::Katana::ScenegraphContext *context) :
    _sgContext(context)
{
    //g_printObjectCountsOnUnload.incrContext(this);
}

FnSGContextStruct::~FnSGContextStruct()
{
    //g_printObjectCountsOnUnload.decrContext(this);
}

std::vector<std::string> &FnSGContextStruct::attrNameCache()
{
    if (!_attrNameCache.get())
    {
        _attrNameCache.reset(new std::vector<std::string>);
    }
    return *_attrNameCache;
}

std::vector<const char *> &FnSGContextStruct::attrNameCharCache()
{
    if (!_attrNameCharCache.get())
    {
        _attrNameCharCache.reset(new std::vector<const char *>);
    }
    return *_attrNameCharCache;
}



// C callbacks implementations for the plugin suite

void destroy(FnScenegraphGeneratorHandle handle)
{
    delete handle;
}

bool setArgs(FnScenegraphGeneratorHandle handle, FnAttributeHandle args)
{
    return handle->getScenegraphGenerator().setArgs(
        Foundry::Katana::Attribute::CreateAndRetain(args));
}

bool checkArgs(FnScenegraphGeneratorHandle handle, FnAttributeHandle args)
{
    return handle->getScenegraphGenerator().checkArgs(
        Foundry::Katana::Attribute::CreateAndRetain(args));
}

FnSGContextHandle getRoot(FnScenegraphGeneratorHandle handle)
{
    return Foundry::Katana::ScenegraphContext::newSGContextHandle(handle->getScenegraphGenerator().getRoot());
}


FnSGContextHandle getFirstChild(FnSGContextHandle handle)
{
    return Foundry::Katana::ScenegraphContext::newSGContextHandle(handle->getSGContext().getFirstChild());
}

FnSGContextHandle getNextSibling(FnSGContextHandle handle)
{
    return Foundry::Katana::ScenegraphContext::newSGContextHandle(handle->getSGContext().getNextSibling());
}

const char **getLocalAttrNames(FnSGContextHandle handle, int *nameCount)
{
    std::vector<std::string> &attrNameCache = handle->attrNameCache();
    std::vector<const char *> &attrNameCharCache = handle->attrNameCharCache();

    handle->getSGContext().getLocalAttrNames(&attrNameCache);
    attrNameCharCache.reserve(attrNameCache.size());
    for (std::vector<std::string>::const_iterator iter=attrNameCache.begin();
         iter!=attrNameCache.end(); ++iter)
    {
        attrNameCharCache.push_back((*iter).c_str());
    }

    *nameCount = attrNameCharCache.size();

    return attrNameCharCache.data();
}

FnAttributeHandle getLocalAttr(FnSGContextHandle handle, const char* name)
{
    return handle->getSGContext().getLocalAttr(name).getRetainedHandle();
}

void destroyContext(FnSGContextHandle handle)
{
    delete handle;
}

namespace // anonymous
{
    bool _recursiveCheckArgs(Foundry::Katana::Attribute expectedArg, Foundry::Katana::Attribute arg)
    {
        if (!expectedArg.isValid()) return true;

        if (!arg.isValid()) return false;
        if (expectedArg.getType() != arg.getType()) return false;

        switch (expectedArg.getType())
        {
        case kFnKatAttributeTypeInt:
        case kFnKatAttributeTypeFloat:
        case kFnKatAttributeTypeDouble:
        case kFnKatAttributeTypeString:
        {
            Foundry::Katana::DataAttribute expData(expectedArg);
            Foundry::Katana::DataAttribute argData(arg);
            if (expData.getNumberOfValues() != argData.getNumberOfValues()) return false;

            break;
        }
        case kFnKatAttributeTypeGroup:
        {
            Foundry::Katana::GroupAttribute expGroup(expectedArg);
            Foundry::Katana::GroupAttribute argGroup(arg);
            if (!expGroup.isValid()) return true;
            if (!argGroup.isValid()) return false;

            unsigned int expChildCount = expGroup.getNumberOfChildren();
            for (unsigned int i=0; i<expChildCount; ++i)
            {
                std::string childName = expGroup.getChildName(i);
                if (!_recursiveCheckArgs(expGroup.getChildByName(childName),
                                         argGroup.getChildByName(childName)))
                {
                    return false;
                }
            }

            break;
        }
        case kFnKatAttributeTypeNull:
        case kFnKatAttributeTypeError:
            break;
        }

        return true;
    }
}


namespace Foundry
{
    namespace Katana
    {
        FnScenegraphGeneratorHandle ScenegraphGenerator::newScenegraphGeneratorHandle(ScenegraphGenerator *sg)
        {
            if (!sg) return 0x0;

            FnScenegraphGeneratorHandle h = new FnScenegraphGeneratorStruct(sg);
            return h;
        }

        FnSGContextHandle ScenegraphContext::newSGContextHandle(ScenegraphContext *context)
        {
            if (!context) return 0x0;

            FnSGContextHandle h = new FnSGContextStruct(context);
            return h;
        }

        // Default implementation of checkArgs checks given args structure
        // against expected args structure (size, type, etc).  Ignore extra
        // args, just make sure everything in expectedArgs is present in args.
        bool ScenegraphGenerator::checkArgs(GroupAttribute args)
        {
            /* checkArgs doesn't work when the argument template is returned by a static
               (non-virtual) function, unless each class has its own virtual method for
               returning one. Anyway, for now, we're short-circuiting it, on the grounds
               that, if AMPs don't have it, SGGs don't need it either.  If we want it back,
               we can give each class a virtual getExpectedArgs() instance method. */
            return true;
            //return _recursiveCheckArgs(getArgumentTemplate(), args);
        }

        FnPlugStatus ScenegraphGenerator::setHost(FnPluginHost *host)
        {
            _host = host;
            GroupBuilder::setHost(host);
            FnLogging::setHost(host);
            DefaultAssetPlugin::setHost(host);
            DefaultFileSequencePlugin::setHost(host);
            return Attribute::setHost(host);
        }

        FnPluginHost *ScenegraphGenerator::getHost() {return _host;}

        FnScenegraphGeneratorPluginSuite_v1 ScenegraphGenerator::createSuite(
            FnScenegraphGeneratorHandle (*create)(),
            FnAttributeHandle (*getArgumentTemplate)())
        {
            FnScenegraphGeneratorPluginSuite_v1 suite = {
                create,

                ::destroy,
                getArgumentTemplate,
                ::setArgs,
                ::checkArgs,
                ::getRoot,

                ::getFirstChild,
                ::getNextSibling,
                ::getLocalAttrNames,
                ::getLocalAttr,
                ::destroyContext
            };

            return suite;
        }

        unsigned int ScenegraphGenerator::_apiVersion = 1;
        const char* ScenegraphGenerator::_apiName = "ScenegraphGeneratorPlugin";
        FnPluginHost *ScenegraphGenerator::_host = 0x0;

    }//namespace
}//namespace
