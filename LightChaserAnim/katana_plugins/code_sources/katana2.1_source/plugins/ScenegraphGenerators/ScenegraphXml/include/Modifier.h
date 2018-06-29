// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_MODIFIER_H
#define INCLUDED_INSTANCE_SCENE_MODIFIER_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"


namespace instance_scene
{
    class Modifier
    {
    public:
        Modifier(std::string type, std::string filepath, ErrorReport &errorReport);
        std::string getType() { return m_type; }
        std::string getFilepath() { return m_filepath; }

    private:
        std::string m_type;
        std::string m_filepath;
    };
    
    typedef boost::shared_ptr<Modifier> ModifierRcPtr;

}

#endif
