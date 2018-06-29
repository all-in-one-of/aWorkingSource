#ifndef FnGeolibServicesHintUtil_H
#define FnGeolibServicesHintUtil_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/suite/FnHintUtilSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FnHintUtil
    {
    public:

        static FnAttribute::GroupAttribute getHintGroup(
                const FnAttribute::Attribute & inputAttr);

        static FnAttribute::GroupAttribute parseConditionalStateGrammar(
                const std::string & inputExpr,
                const std::string & prefix = std::string("conditionalVis"),
                const std::string & secondaryPrefix = std::string());

    private:
        FnHintUtil();

        static const FnHintUtilHostSuite_v1 * _getSuite();
    };


}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesHintUtil_H
