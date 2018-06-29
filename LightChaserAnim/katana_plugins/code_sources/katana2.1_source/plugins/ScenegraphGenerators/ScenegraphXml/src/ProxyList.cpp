// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <ProxyList.h>
#include <iostream>

namespace instance_scene
{ 
    ProxyData::ProxyData(ptree element, std::string basedir, ErrorReport &errorReport)
    {
        if( element.empty() )
        {
            errorReport.addErrorMessage("ScenegraphXML: error reading XML element for ProxyData\n");
            return;
        }

        if( PTreeHelper::hasAttr( element, "name" ) )
        {
            m_name = PTreeHelper::getAttr( element, "name" );
        }

        if( PTreeHelper::hasAttr( element, "ref" ) )
        {
            m_referencePath = PTreeHelper::getAttr( element, "ref" );

            // if m_reference path isn't an explicit path, construct explicit path using basedir
            if (m_referencePath[0] != '/')
            {
                m_referencePath = basedir + m_referencePath;
            }
        }
    }
    
    std::string ProxyData::getName()
    {
        return m_name;
    }

    std::string ProxyData::getReferencePath()
    {
        return m_referencePath;
    }
    
///////////////////////////////////////////////////////////////////////////////

    ProxyList::ProxyList(ptree element, std::string basedir, ErrorReport &errorReport)
    {
        if( element.empty() )
        {
            errorReport.addErrorMessage("ScenegraphXML: error reading XML element for ProxyList\n");
            return;
        }

        m_proxies.clear();
        unsigned int numProxies = element.size();
        m_proxies.reserve(numProxies);
        for ( ptree::const_iterator iter = element.begin(); iter != element.end(); ++iter )
        {
            std::string const & type = (*iter).first;
            ptree const & child = (*iter).second;

            if( type == "proxy" )
            {                
                ProxyDataRcPtr newProxy;
                newProxy = ProxyDataRcPtr(new ProxyData(child, basedir, errorReport));
                m_proxies.push_back(newProxy);
            }
            else
            {
                errorReport.addWarningMessage("ScenegraphXML: found unexpected XML element in ProxyList\n");
            }
        }
   }

    size_t ProxyList::getNumberOfProxies()
    {
        return m_proxies.size();
    }
    
    ProxyDataRcPtr ProxyList::getProxy(size_t index)
    {
        if (index < m_proxies.size())
        {
            return m_proxies[index];
        }
        
        return ProxyDataRcPtr();
    }
}
