// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <Bounds.h>

namespace instance_scene
{
    
    Bounds::Bounds(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : m_minx(0)
    , m_maxx(0)
    , m_miny(0)
    , m_maxy(0)
    , m_minz(0)
    , m_maxz(0)
    , m_channels(channels)
    , m_channelIndex(-1)
    {
        if( element.empty() )
        {
            errorReport.addErrorMessage("ScenegraphXML: Error parsing XML for bounds\n");
            return;
        }
        if( PTreeHelper::hasAttr( element, "minx" ) )
        {
            m_minx = atof(PTreeHelper::getAttr( element, "minx" ).c_str());
        }
        if( PTreeHelper::hasAttr( element, "maxx" ) )
        {
            m_maxx = atof(PTreeHelper::getAttr( element, "maxx" ).c_str());
        }
        if( PTreeHelper::hasAttr( element, "miny" ) )
        {
            m_miny = atof(PTreeHelper::getAttr( element, "miny" ).c_str());
        }
        if( PTreeHelper::hasAttr( element, "maxy" ) )
        {
            m_maxy = atof(PTreeHelper::getAttr( element, "maxy" ).c_str());
        }
        if( PTreeHelper::hasAttr( element, "minz" ) )
        {
            m_minz = atof(PTreeHelper::getAttr( element, "minz" ).c_str());
        }
        if( PTreeHelper::hasAttr( element, "maxz" ) )
        {
            m_maxz = atof(PTreeHelper::getAttr( element, "maxz" ).c_str());
        }
        if( PTreeHelper::hasAttr( element, "channelIndex" ) )
        {
            m_channelIndex = atoi(PTreeHelper::getAttr( element, "channelIndex" ).c_str());
        }
    }
    
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
    
    double Bounds::getMinX(int frame)
    {
        return getIndexedValue(m_minx, m_channels, m_channelIndex, 0, frame);
    }
    
    double Bounds::getMaxX(int frame)
    {
        return getIndexedValue(m_maxx, m_channels, m_channelIndex, 1, frame);
    }
    
    double Bounds::getMinY(int frame)
    {
        return getIndexedValue(m_miny, m_channels, m_channelIndex, 2, frame);
    }
    
    double Bounds::getMaxY(int frame)
    {
        return getIndexedValue(m_maxy, m_channels, m_channelIndex, 3, frame);
    }
    
    double Bounds::getMinZ(int frame)
    {
        return getIndexedValue(m_minz, m_channels, m_channelIndex, 4, frame);
    }
    
    double Bounds::getMaxZ(int frame)
    {
        return getIndexedValue(m_maxz, m_channels, m_channelIndex, 5, frame);
    }
    
    bool Bounds::isAnimated()
    {
        return (m_channels && m_channelIndex >= 0);
    }
    
}
