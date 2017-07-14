// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_BOUNDS_H
#define INCLUDED_INSTANCE_SCENE_BOUNDS_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"

namespace instance_scene
{
    class Bounds
    {
    public:
        Bounds(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);
        
        double getMinX(int frame);
        double getMaxX(int frame);
        double getMinY(int frame);
        double getMaxY(int frame);
        double getMinZ(int frame);
        double getMaxZ(int frame);
        
        bool isAnimated();
        
    private:
        double m_minx;
        double m_maxx;
        double m_miny;
        double m_maxy;
        double m_minz;
        double m_maxz;
        ChannelRangeRcPtr m_channels;
        int m_channelIndex;
    };
    
    typedef boost::shared_ptr<Bounds> BoundsRcPtr;
    
}

#endif
