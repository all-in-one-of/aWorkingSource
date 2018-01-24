// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_XFORM_H
#define INCLUDED_INSTANCE_SCENE_XFORM_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"


namespace instance_scene
{
    class Xform
    {
    public:
        Xform(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);
        void getValues(int frame, std::vector<double> & values);
        
        
        bool isAnimated();
        
    private:
        std::vector<double> m_values;
        ChannelRangeRcPtr m_channels;
        int m_channelIndex;
    };
    
    typedef boost::shared_ptr<Xform> XformRcPtr;
    
}

#endif
