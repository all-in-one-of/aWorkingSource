#ifndef FnGeolibOp_H
#define FnGeolibOp_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/suite/FnGeolibOpSuite.h>
#include <FnGeolib/suite/FnGeolibCookInterfaceSuite.h>
#include <FnGeolib/suite/FnGeolibSetupInterfaceSuite.h>

#include <FnAttribute/FnAttribute.h>

#include "FnGeolibCookInterface.h"
#include "FnGeolibSetupInterface.h"
#include "FnOpDescriptionBuilder.h"

#include <vector>
#include <string>

#include "ns.h"

/**
 * \defgroup FnGeolibOp Op API
 * @{
 *
 * @brief The Op API offers a unified interface for manipulating the scene graph
 *     and modifying attributes, something that was previously only possible
 *     through a combination of SGGs and AMPs. All of Katana's shipped Ops are
 *     written with the Op API.
 *
 * @par Utility Functions
 *
 *  The Op API also provides a number of utility functions for a number of
 *  common tasks such as obtaining frequently used attributes (such as bounding
 *  boxes) and reporting errors to the scene graph. The majority of the utility
 *  functions are designed to be used within the context of an Op's \c cook()
 *  function and require a reference to the GeolibCookInterface that has been
 *  passed to the \c cook() function.
 */
FNGEOLIBOP_NAMESPACE_ENTER
{
    // GeolibOp and classes derived from it are never instantiated.
    // The base class contains some common static functions used by
    // the plugin system (setHost/getHost/flush/createSuite).
    //
    // Each derived class should create a static cook function
    // which will be registered in a FnGeolibOpSuite_v1 instance.
    //
    // Cook func: static void cook(GeolibCookInterface &cookInterface);
    class GeolibOp
    {
    public:
        /// @cond FN_INTERNAL_DEV
        static FnPlugStatus setHost(FnPluginHost *host);
        static FnPluginHost *getHost();

        static void flush();

        static FnGeolibOpSuite_v1 createSuite(
            void (*setup)(FnGeolibSetupInterfaceHandle cookInterfaceHandle,
                          FnGeolibSetupInterfaceSuite_v1 *cookInterfaceSuite),
            void (*cook)(FnGeolibCookInterfaceHandle cookInterfaceHandle,
                         FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
                         uint8_t * didAbort),
            FnAttributeHandle (*describe)());

        static void callSetup(
            void (*setupFunc)(GeolibSetupInterface & setupInterface),
            FnGeolibSetupInterfaceHandle setupInterfaceHandle,
            FnGeolibSetupInterfaceSuite_v1 *setupInterfaceSuite);

        static void callCook(
            void (*cookFunc)(GeolibCookInterface & cookInterface),
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,
            uint8_t * didAbort);

        static FnAttributeHandle callDescribe(
            FnAttribute::GroupAttribute (*describeFunc)());

        static FnAttribute::GroupAttribute describe();

        static unsigned int _apiVersion;
        static const char* _apiName;

    private:
        static FnPluginHost *_host;

        // op instances are never made (just a derivable placeholder for
        // static funcs).
        GeolibOp();
        ~GeolibOp();
        /// @endcond
    };
}
FNGEOLIBOP_NAMESPACE_EXIT

/**
 * @}
 */

/// @cond FN_INTERNAL_DEV

// Plugin Registering Macro.

#define DEFINE_GEOLIBOP(OP_CLASS)                               \
                                                                \
    void OP_CLASS##_setup(                                      \
        FnGeolibSetupInterfaceHandle setupInterfaceHandle,      \
        FnGeolibSetupInterfaceSuite_v1 *setupInterfaceSuite)    \
    {                                                           \
        FNGEOLIBOP_NAMESPACE::GeolibOp::callSetup(OP_CLASS::setup,    \
            setupInterfaceHandle, setupInterfaceSuite           \
            );                                                  \
    }                                                           \
                                                                \
    void OP_CLASS##_cook(                                       \
        FnGeolibCookInterfaceHandle cookInterfaceHandle,        \
        FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,      \
        uint8_t * didAbort)                                     \
    {                                                           \
        FNGEOLIBOP_NAMESPACE::GeolibOp::callCook(OP_CLASS::cook,     \
            cookInterfaceHandle, cookInterfaceSuite,            \
            didAbort);                                          \
    }                                                           \
                                                                \
    FnAttributeHandle OP_CLASS##_describe()                     \
    {                                                           \
        return FNGEOLIBOP_NAMESPACE::GeolibOp::callDescribe(    \
            OP_CLASS::describe);                                \
    }                                                           \
                                                                \
    FnGeolibOpSuite_v1 OP_CLASS##_suite =                       \
        FNGEOLIBOP_NAMESPACE::GeolibOp::createSuite(            \
            OP_CLASS##_setup,                                   \
            OP_CLASS##_cook,                                    \
            OP_CLASS##_describe);                               \
                                                                \
    const void* OP_CLASS##_getSuite()                           \
    {                                                           \
        return &OP_CLASS##_suite;                               \
    }

#define DEFINE_GEOLIBOP_PLUGIN(OP_CLASS)                        \
                                                                \
    FnPlugin OP_CLASS##_plugin;                                 \
                                                                \
    DEFINE_GEOLIBOP(OP_CLASS)


// NO-SETUP VARIANT
#define DEFINE_GEOLIBOP_NOSETUP(OP_CLASS)                               \
                                                                \
    void OP_CLASS##_setup(                                      \
        FnGeolibSetupInterfaceHandle,      \
        FnGeolibSetupInterfaceSuite_v1 *)    \
    {                                                           \
    }                                                           \
                                                                \
    void OP_CLASS##_cook(                                       \
        FnGeolibCookInterfaceHandle cookInterfaceHandle,        \
        FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite,      \
        uint8_t * didAbort)                                     \
    {                                                           \
        FNGEOLIBOP_NAMESPACE::GeolibOp::callCook(OP_CLASS::cook,     \
            cookInterfaceHandle, cookInterfaceSuite,            \
            didAbort);                                          \
    }                                                           \
                                                                \
    FnAttributeHandle OP_CLASS##_describe()                     \
    {                                                           \
        return FNGEOLIBOP_NAMESPACE::GeolibOp::callDescribe(    \
            OP_CLASS::describe);                                \
    }                                                           \
                                                                \
    FnGeolibOpSuite_v1 OP_CLASS##_suite =                       \
        FNGEOLIBOP_NAMESPACE::GeolibOp::createSuite(            \
            OP_CLASS##_setup,                                   \
            OP_CLASS##_cook,                                    \
            OP_CLASS##_describe);                               \
                                                                \
    const void* OP_CLASS##_getSuite()                           \
    {                                                           \
        return &OP_CLASS##_suite;                               \
    }

#define DEFINE_GEOLIBOP_PLUGIN_NOSETUP(OP_CLASS)                \
                                                                \
    FnPlugin OP_CLASS##_plugin;                                 \
                                                                \
    DEFINE_GEOLIBOP_NOSETUP(OP_CLASS)


///@endcond

#endif // FnGeolibOp_H
