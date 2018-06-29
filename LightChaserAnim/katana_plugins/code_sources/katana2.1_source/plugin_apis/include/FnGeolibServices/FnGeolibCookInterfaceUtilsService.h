// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibCookInterfaceUtilsService_H
#define FnGeolibCookInterfaceUtilsService_H

#include <string>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>
#include <FnGeolib/op/FnGeolibCookInterface.h>

#include <FnGeolibServices/suite/FnGeolibCookInterfaceUtilsSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class BaseGlobalAttributeProvider;

    class FnGeolibCookInterfaceUtils
    {
    public:
        struct MatchesCELInfo
        {
            bool matches;
            bool canMatchChildren;
        };

        static void matchesCEL(MatchesCELInfo & matchesCELInfo,
                const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
                const FnAttribute::StringAttribute & cel,
                const std::string & inputLocationPath=std::string(),
                int inputIndex=kFnKatGeolibDefaultInput);


        static FnAttribute::GroupAttribute cookDaps(
                const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
                const std::string & attrRoot,
                const std::string & inputLocationPath=std::string(),
                int inputIndex=kFnKatGeolibDefaultInput,
                const FnAttribute::Attribute & cookOrderAttr=FnAttribute::Attribute());

        struct BuildLocalsAndGlobalsInfo
        {
            FnAttribute::GroupAttribute locals;
            FnAttribute::GroupAttribute globals;
        };

        static BuildLocalsAndGlobalsInfo buildLocalsAndGlobals(
                const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
                const std::string & attrRoot);

        static FnAttribute::Attribute getDefaultDapCookOrder();

        static FnAttribute::GroupAttribute resolveFileNamesWithinGroup(
                const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
                const FnAttribute::GroupAttribute & inputGroup,
                const FnAttribute::StringAttribute & celAttr,
                int frameNumber);

        static FnAttribute::StringAttribute resolveFileNameInString(
                const FNGEOLIBOP_NAMESPACE::GeolibCookInterface & interface,
                const FnAttribute::StringAttribute & inputString,
                int frameNumber);

        static FnAttribute::Attribute getGlobalAttrGeneric(
                BaseGlobalAttributeProvider & provider,
                const std::string & name,
                const std::string & locationPath,
                bool* didAbortPtr = NULL);

    private:
        FnGeolibCookInterfaceUtils();

        static const FnGeolibCookInterfaceUtilsHostSuite_v2 * _getSuite();
    };

    // Base abstract class representing a generic interface to query global
    // attributes starting from a specified scene graph location.  A global
    // attribute provider is meant to be able to walk a scene graph hierarchy
    // (upwards) and look for attributes by name
    class BaseGlobalAttributeProvider
    {
    public:
        virtual ~BaseGlobalAttributeProvider() {}

        virtual FnGeolibGlobalAttributeProviderStatus provideAttribute(
                const std::string& attrPath,
                const std::string& leafLocationPath,
                int32_t relativeDepth,
                FnAttribute::Attribute* attributeOut) = 0;
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesCookInterfaceUtilsService_H
