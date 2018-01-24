// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_LODDATA_H
#define INCLUDED_INSTANCE_SCENE_LODDATA_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"


namespace instance_scene
{
    class LodData
    {
    public:
        LodData(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);
        std::string getTag();
        float getWeight(int frame);
        
        bool isAnimated();
        
    private:
        double m_weight;
        std::string m_tag;
        ChannelRangeRcPtr m_channels;
        int m_channelIndex;
    };
    
    typedef boost::shared_ptr<LodData> LodDataRcPtr;
}

#endif
