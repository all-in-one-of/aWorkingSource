// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_CHANNELS_H
#define INCLUDED_INSTANCE_SCENE_CHANNELS_H

#include <vector>
#include <string>
#include <map>

//#include <ptr/shared_ptr.h>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace instance_scene
{
    class ChannelsSample
    {
    public:
        ChannelsSample(const std::string & filename);
        double getValue(size_t index);

    private:
        std::vector<double> m_values;
    };
    
    typedef boost::shared_ptr<ChannelsSample> ChannelsSampleRcPtr;
    
    ///////////////////////////////////////////////////////////////////////////
    
    class ChannelRange
    {
    public:
        ChannelRange(const std::string & baseFilename,
                int startFrame, int endFrame);
        
        double getValue(size_t index, int frame);  
        
        int getStartFrame();
        int getEndFrame();
          
    private:
        
        std::string m_baseFilename;
        int m_startFrame;
        int m_endFrame;
        
        typedef std::map<int, ChannelsSampleRcPtr> ChannelsSampleMap;
        ChannelsSampleMap m_samples;
    };
    
    typedef boost::shared_ptr<ChannelRange> ChannelRangeRcPtr;

    
}

#endif
