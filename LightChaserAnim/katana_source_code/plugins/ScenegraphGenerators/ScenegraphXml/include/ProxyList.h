// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_PROXYLIST_H
#define INCLUDED_INSTANCE_SCENE_PROXYLIST_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"

namespace instance_scene
{
    class ProxyData
    {
    public:
        ProxyData(ptree element, std::string basedir, ErrorReport &errorReport);

        std::string getName();
        std::string getReferencePath();

    private:
        std::string m_name;
        std::string m_referencePath;
    };

    typedef boost::shared_ptr<ProxyData> ProxyDataRcPtr;

    class ProxyList
    {
    public:
        ProxyList(ptree element, std::string basedir, ErrorReport &errorReport);

        size_t getNumberOfProxies();
        ProxyDataRcPtr getProxy(size_t index);
      
    private:
        std::vector<ProxyDataRcPtr> m_proxies;      
    };
    
    typedef boost::shared_ptr<ProxyList> ProxyListRcPtr;
}

#endif
