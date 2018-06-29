// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_SCENE_H
#define INCLUDED_INSTANCE_SCENE_SCENE_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"
#include "Instance.h"
#include "Bounds.h"
#include "ArbitraryList.h"
#include "ProxyList.h"
#include <vector>

namespace instance_scene
{
    #define SGXML_VERSION_0 0
    #define SGXML_VERSION_1 1
    #define SGXML_VERSION_2 0

    class Scene;
    typedef boost::shared_ptr<Scene> SceneRcPtr;


    class Scene
    {
    public:
        Scene(const std::string & filepath);
        ~Scene()
        {
            std::vector<Scene*>::iterator it = std::find(_allScenes.begin(), _allScenes.end(), this);
            assert(it != _allScenes.end());
            if (it != _allScenes.end())
            {
                _allScenes.erase(it);
            }
        }

        static const std::vector<Scene*>& getAllScenes() { return _allScenes; }
        
        static SceneRcPtr loadCached(const std::string & filepath);
        static void flushCache();
        
        InstanceGroupRcPtr getInstanceList();
        LodDataRcPtr getLodData();
        std::string getFilePath();
        std::string getErrorMessages();
        std::string getWarningMessages();
        bool hasErrors();
        bool hasWarnings();
        
    private:
        InstanceGroupRcPtr m_instanceList; 
        ChannelRangeRcPtr m_channels;
        std::string m_filepath;
        ErrorReport m_errorReport;
        static std::vector<Scene*> _allScenes;
    };   
    
}


#endif
