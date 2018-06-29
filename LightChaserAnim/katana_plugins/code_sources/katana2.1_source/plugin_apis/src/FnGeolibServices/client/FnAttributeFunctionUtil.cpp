// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnGeolibServices/FnAttributeFunctionUtil.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    
    
    FnAttribute::Attribute FnAttributeFunctionUtil::run(
            const std::string & fncName, FnAttribute::Attribute args)
    {
        const FnAttributeFunctionUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return FnAttribute::Attribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->run(fncName.c_str(), args.getRetainedHandle()));
    }

    
    void FnAttributeFunctionUtil::getRegisteredFunctionNames(
            std::vector<std::string> & names)
    {
        names.clear();
        
        const FnAttributeFunctionUtilHostSuite_v1 * suite = _getSuite();
        if (!suite)
        {
            return;
        }
        
        FnAttribute::StringAttribute attr =
                FnAttribute::Attribute::CreateAndSteal(
                        suite->getRegisteredFunctionNames());
        
        if (attr.isValid())
        {
            FnAttribute::StringAttribute::array_type v =
                    attr.getNearestSample(0.0f);
            
            names.reserve(v.size());
            names.insert(names.begin(), v.begin(), v.end());            
        }
    }
    

    void FnAttributeFunctionUtil::flushCache()
    {
        const FnAttributeFunctionUtilHostSuite_v1 * suite = _getSuite();
        if (!suite) return;

        suite->flushCache();
    }
    
    ////////////////////////////////////////////////////////////////////////////

    const FnAttributeFunctionUtilHostSuite_v1 *FnAttributeFunctionUtil::_getSuite()
    {
        static const FnAttributeFunctionUtilHostSuite_v1 * suite = 0x0;

        if (!suite)
        {
            suite = reinterpret_cast<const FnAttributeFunctionUtilHostSuite_v1 *>(
                    FnPluginManager::PluginManager::getHostSuite(
                            "AttributeFunctionUtilHost", 1));
        }

        return suite;
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
