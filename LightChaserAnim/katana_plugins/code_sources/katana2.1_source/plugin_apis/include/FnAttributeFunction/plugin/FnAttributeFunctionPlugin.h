#ifndef FnAttributeFunctionPlugin_H
#define FnAttributeFunctionPlugin_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>

#include <FnAttributeFunction/suite/FnAttributeFunctionSuite.h>

#include <vector>
#include <string>

#include "ns.h"

FNATTRIBUTEFUNCTION_NAMESPACE_ENTER
{
    // AttributeFunction and classes derived from it are never
    // instantiated.  The base class contains some common static functions
    // used by the plugin system (setHost/getHost/flush/createSuite).
    //
    // Each derived class should create a static run function
    // which will be registered in a FnAttributeFunctionSuite_v1
    // instance.
    //
    // run func: static FnAttribute::Attribute run(FnAttribute::Attribute args);
    class AttributeFunction
    {
    public:
        ///@cond FN_INTERNAL_DEV
        static FnPlugStatus setHost(FnPluginHost *host);
        static FnPluginHost *getHost();

        static void flush();

        static FnAttributeFunctionSuite_v1 createSuite(
                FnAttributeHandle (*run)(FnAttributeHandle));

        typedef FnAttribute::Attribute (*RunFunc)(FnAttribute::Attribute);

        static FnAttribute::Attribute callRun(
                RunFunc runFunc, FnAttribute::Attribute args);

        static unsigned int _apiVersion;
        static const char* _apiName;

    private:
        static FnPluginHost *_host;

        // AttributeFunction classes are never instantiated.
        AttributeFunction();
        ~AttributeFunction();
        ///@endcond
    };
}
FNATTRIBUTEFUNCTION_NAMESPACE_EXIT


///@cond FN_INTERNAL_DEV

// Plugin Registering Macro.

#define DEFINE_ATTRIBUTEFUNCTION(ATTRFNC_CLASS)                             \
    FnAttributeHandle ATTRFNC_CLASS##_run(                                  \
        FnAttributeHandle args)                                             \
    {                                                                       \
        return FNATTRIBUTEFUNCTION_NAMESPACE::AttributeFunction::callRun(   \
            ATTRFNC_CLASS::run, FnAttribute::Attribute::CreateAndRetain(    \
                args)).getRetainedHandle();                                 \
    }                                                                       \
                                                                            \
    FnAttributeFunctionSuite_v1 ATTRFNC_CLASS##_suite =                     \
        FNATTRIBUTEFUNCTION_NAMESPACE::AttributeFunction::createSuite(      \
            ATTRFNC_CLASS##_run);                                           \
                                                                            \
    const void* ATTRFNC_CLASS##_getSuite()                                  \
    {                                                                       \
        return &ATTRFNC_CLASS##_suite;                                      \
    }

#define DEFINE_ATTRIBUTEFUNCTION_PLUGIN(ATTRFNC_CLASS)                      \
                                                                            \
    FnPlugin ATTRFNC_CLASS##_plugin;                                        \
                                                                            \
    DEFINE_ATTRIBUTEFUNCTION(ATTRFNC_CLASS)

///@endcond

#endif // FnAttributeFunctionPlugin_H
