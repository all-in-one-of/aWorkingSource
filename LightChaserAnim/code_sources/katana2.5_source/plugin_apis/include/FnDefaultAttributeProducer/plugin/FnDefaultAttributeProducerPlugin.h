#ifndef FnDefaultAttributeProducerPlugin_H
#define FnDefaultAttributeProducerPlugin_H

#include <string>
#include <vector>

#include <FnAttribute/FnAttribute.h>

#include <FnDefaultAttributeProducer/FnDefaultAttributeProducerAPI.h>
#include <FnDefaultAttributeProducer/plugin/ns.h>
#include <FnDefaultAttributeProducer/suite/FnDefaultAttributeProducerSuite.h>

#include <FnGeolib/op/FnGeolibCookInterface.h>
#include <FnGeolib/suite/FnGeolibCookInterfaceSuite.h>

#include <FnPluginSystem/FnPlugin.h>
#include <FnPluginSystem/FnPluginSystem.h>


FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_ENTER
{

    // DefaultAttributeProducer and classes derived from it are never
    // instantiated.  The base class contains some common static functions
    // used by the plugin system (setHost/getHost/flush/createSuite).
    //
    // Each derived class should create a static cook function
    // which will be registered in a FnDefaultAttributeProducerSuite_v1
    // instance.
    //
    // Cook func: static void cook(GeolibCookInterface &cookInterface
    //                             const std::string & attrRoot,
    //                             const std::string & inputLocationPath,
    //                             int32_t inputIndex);
    class FNDEFAULTATTRIBUTEPRODUCER_API DefaultAttributeProducer
    {
    public:
        ///@cond FN_INTERNAL_DEV
        static FnPlugStatus setHost(FnPluginHost *host);
        static FnPluginHost *getHost();

        static void flush();

        static FnDefaultAttributeProducerSuite_v1 createSuite(
                FnAttributeHandle (*cook)(
                         FnGeolibCookInterfaceHandle cookInterfaceHandle,
                         FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
                         const char *attrRoot, int32_t stringlen1,
                         const char *inputLocationPath, int32_t stringlen2,
                         int32_t inputIndex,
                         uint8_t * didAbort));

        typedef FnAttribute::GroupAttribute (*CookFunc)(
                const FnGeolibOp::GeolibCookInterface & cookInterface,
                const std::string & attrRoot,
                const std::string & inputLocationPath,
                int32_t inputIndex);

        static FnAttribute::GroupAttribute callCook(
            CookFunc cookFunc,
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
            const char *attrRoot, int32_t stringlen1,
            const char *inputLocationPath, int32_t stringlen2,
            int32_t inputIndex,
            uint8_t * didAbort);

        static unsigned int _apiVersion;
        static const char* _apiName;

    private:
        static FnPluginHost *_host;

        // dap instances are never made (just a derivable placeholder for
        // static funcs).
        DefaultAttributeProducer();
        ~DefaultAttributeProducer();
        ///@endcond
    };
}
FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE_EXIT


///@cond FN_INTERNAL_DEV

// Plugin Registering Macro.

#define DEFINE_DEFAULTATTRIBUTEPRODUCER(DAP_CLASS)                      \
    FnAttributeHandle DAP_CLASS##_cook(                                 \
        FnGeolibCookInterfaceHandle cookInterfaceHandle,                \
        FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,              \
        const char *attrRoot, int32_t stringlen1,                       \
        const char *inputLocationPath, int32_t stringlen2,              \
        int32_t inputIndex,                                             \
        uint8_t * didAbort)                                             \
    {                                                                   \
        return FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE::DefaultAttributeProducer::callCook(      \
            DAP_CLASS::cook,                                            \
            cookInterfaceHandle, cookInterfaceSuite,                    \
            attrRoot, stringlen1, inputLocationPath, stringlen2,        \
            inputIndex, didAbort).getRetainedHandle();                  \
    }                                                                   \
                                                                        \
    FnDefaultAttributeProducerSuite_v1 DAP_CLASS##_suite =              \
        FNDEFAULTATTRIBUTEPRODUCER_NAMESPACE::DefaultAttributeProducer::createSuite(   \
            DAP_CLASS##_cook);                                          \
                                                                        \
    const void* DAP_CLASS##_getSuite()                                  \
    {                                                                   \
        return &DAP_CLASS##_suite;                                      \
    }

#define DEFINE_DEFAULTATTRIBUTEPRODUCER_PLUGIN(DAP_CLASS)               \
                                                                        \
    FnPlugin DAP_CLASS##_plugin;                                        \
                                                                        \
    DEFINE_DEFAULTATTRIBUTEPRODUCER(DAP_CLASS)

///@endcond

#endif // FnDefaultAttributeProducerPlugin_H
