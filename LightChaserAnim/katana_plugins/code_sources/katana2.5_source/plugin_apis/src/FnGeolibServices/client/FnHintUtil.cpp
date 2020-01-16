// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnGeolibServices/FnHintUtil.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    FnAttribute::GroupAttribute
    FnHintUtil::getHintGroup(
            const FnAttribute::Attribute & inputAttr)
    {
        const FnHintUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        return FnAttribute::GroupAttribute(
                FnAttribute::Attribute::CreateAndSteal(
                        suite->getHintGroup(inputAttr.getHandle())));
    }

    FnAttribute::GroupAttribute
    FnHintUtil::parseConditionalStateGrammar(
            const std::string & inputExpr,
            const std::string & prefix, const std::string & secondaryPrefix)
    {
        const FnHintUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::GroupAttribute();

        FnAttributeHandle errorMessageHandle = 0x0;

        FnAttribute::GroupAttribute returnAttr = 
                FnAttribute::Attribute::CreateAndSteal(
                        suite->parseConditionalStateGrammar(
                                inputExpr.c_str(),
                                prefix.c_str(),
                                secondaryPrefix.c_str(),
                                &errorMessageHandle
                                ));
        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (errorMessageAttr.isValid())
        {
            throw std::runtime_error(errorMessageAttr.getValue());
        }

        return returnAttr;
    }

    const FnHintUtilHostSuite_v1 *FnHintUtil::_getSuite()
    {
        static FnPluginManager::LazyHostSuite<FnHintUtilHostSuite_v1> suite = {
            "HintUtilHost", 1
        };
        return suite.get();
    }


}
FNGEOLIBSERVICES_NAMESPACE_EXIT
