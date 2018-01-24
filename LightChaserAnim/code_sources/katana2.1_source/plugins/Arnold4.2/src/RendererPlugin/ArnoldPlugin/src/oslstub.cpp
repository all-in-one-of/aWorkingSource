// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#define OPENIMAGEIO_NAMESPACE OpenImageIO_Arnold
#define OSL_NAMESPACE OSL_Arnold


#include <map>


namespace
{
    typedef std::map<std::string, OSL::OSLQuery *> QueryMap;
    QueryMap g_queryMap;
}

extern "C"
{
    bool OSLShaderExists(const std::string & name, const std::string & path)
    {
        QueryMap::iterator I = g_queryMap.find(name);
        if (I != g_queryMap.end())
        {
            return (*I).second != 0;
        }
        
        OSL::OSLQuery * q = new OSL::OSLQuery;
        
        if (!q->open(name, path))
        {
            delete q;
            q = 0;
        }
        
        g_queryMap[name] = q;
        
        return q != 0;
    }
    
    std::string GetOSLTypeStringForParameter(const std::string & name,
        const std::string & param, const std::string & path)
    {
        if (!OSLShaderExists(name, path))
        {
            return "";
        }
        
        const OSL::OSLQuery::Parameter * oslParam =
                g_queryMap[name]->getparam(param);
        
        if (!oslParam)
        {
            return "";
        }
        
        return oslParam->type.c_str();
    }
    
}
