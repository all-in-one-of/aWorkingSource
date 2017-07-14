// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <LodData.h>
#include <iostream>

namespace instance_scene
{
    namespace
    {
        double getIndexedValue(double defaultValue, ChannelRangeRcPtr channels,
                int channelIndex, int channelOffset, int frame)
        {
            if (!channels || channelIndex < 0)
            {
                return defaultValue;
            }
            
            return channels->getValue(channelIndex+channelOffset, frame);
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    LodData::LodData(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : m_channels(channels)
    , m_channelIndex(-1)
    {
        if( element.empty() )
        {
            errorReport.addErrorMessage("ScenegraphXML: error reading XML element for LodData\n");
            return;
        }

        if( PTreeHelper::hasAttr( element, "tag" ) )
        {
            m_tag = PTreeHelper::getAttr( element, "tag" );
        }

        if( PTreeHelper::hasAttr( element, "weight" ) )
        {
            m_weight = atof(PTreeHelper::getAttr( element, "weight" ).c_str());
        }

        if( PTreeHelper::hasAttr( element, "channelIndex" ) )
        {
            m_channelIndex = atoi(PTreeHelper::getAttr( element, "channelIndex" ).c_str());
        }
    }

    std::string LodData::getTag()
    {
        return m_tag;
    }

    float LodData::getWeight(int frame)
    {
        return (float)getIndexedValue(m_weight, m_channels, m_channelIndex, 0, frame);
    }

    bool LodData::isAnimated()
    {
        return (m_channels && m_channelIndex >= 0);
    }    
}
