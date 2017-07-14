// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <iomanip>
#include <ostream>

#include <boost/filesystem.hpp>

#include <Manifest.h>
#include <Channels.h>
#include <FnLogging/FnLogging.h>


namespace instance_scene
{
    FnLogSetup("ScenegraphXml")

    ChannelsSample::ChannelsSample(const std::string & filename)
    {
        if (!boost::filesystem::exists(filename))
        {
            FnLogError("Channel data XML file not found: \"" << filename
                       << "\"")
            return;
        }

        try
        {
            ptree root = PTreeHelper::parse( filename );
            
            unsigned int numChildren = root.size();
            m_values.reserve(numChildren);
            
            for ( ptree::const_iterator iter = root.begin(); iter != root.end(); ++iter )
            {
                std::string const & type = (*iter).first;
                ptree const & child = (*iter).second;
                
                double value = 0;
                if( type == "c" )
                {
                    if( PTreeHelper::hasAttr( child, "v" ) )
                    {
                        value = atof(PTreeHelper::getAttr( child, "v" ).c_str());
                    }
                }
                
                m_values.push_back(value);
            }
        }
        catch (const std::exception & e)

        {
            FnLogError("Unable to parse XML file \"" << filename
                       << "\" for channel data: " << e.what())
        }
    }
    
    double ChannelsSample::getValue(size_t index)
    {
        if (index >= m_values.size())
        {
            return 0;
        }
        return m_values[index];
    }
    
    
    ChannelRange::ChannelRange(const std::string & baseFilename,
            int startFrame, int endFrame)
    : m_baseFilename(baseFilename)
    , m_startFrame(startFrame)
    , m_endFrame(endFrame)
    {
    }
    
    
    int ChannelRange::getStartFrame()
    {
        return m_startFrame;
    }
    
    int ChannelRange::getEndFrame()
    {
        return m_endFrame;
    }
    
    
    double ChannelRange::getValue(size_t index, int frame)
    {
        double value = 0;
        
        if (frame < m_startFrame) 
        {
            frame = m_startFrame;
        }
        
        if (frame > m_endFrame)
        {
            frame = m_endFrame;
        }
        
        ChannelsSampleMap::iterator I = m_samples.find(frame);
        
        
        ChannelsSampleRcPtr sample;
        
        if (I == m_samples.end())
        {
            std::ostringstream buffer;
            
            buffer << m_baseFilename << ".chan.";
            buffer << std::setfill( '0' ) << std::setw(4) << frame;
            buffer << ".xml";
            
            sample = ChannelsSampleRcPtr(new ChannelsSample(buffer.str()));
            m_samples[frame] = sample;
        }
        else
        {
            sample = (*I).second;
        }
        
        if (sample)
        {
            value = sample->getValue(index);
        }
        
        return value;
    }
    
}


