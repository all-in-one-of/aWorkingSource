#ifndef FnGeolibServicesHintUtil_H
#define FnGeolibServicesHintUtil_H

#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/FnGeolibServicesAPI.h>
#include <FnGeolibServices/ns.h>
#include <FnGeolibServices/suite/FnHintUtilSuite.h>

#include <FnPluginSystem/FnPluginSystem.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FNGEOLIBSERVICES_API FnHintUtil
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
