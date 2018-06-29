// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <Instance.h>
#include <iostream>

namespace instance_scene
{
    InstanceBase::InstanceBase(ptree element, ChannelRangeRcPtr channels, std::string basedir)
        : m_channels(channels), m_lookfile("")
    {
        if( element.empty() )
        {
            m_errorReport.addErrorMessage("ScenegraphXML: error XML element for InstanceBase\n");
            return;
        }
        
        if( PTreeHelper::hasAttr( element, "name" ) )
        {
            m_name = PTreeHelper::getAttr( element, "name" );
        }
        
        for ( ptree::const_iterator iter = element.begin(); iter != element.end(); ++iter )
        {
            std::string const & type = (*iter).first;
            ptree const & child = (*iter).second;

            if ( type == "xform" )
            {
                m_xform = XformRcPtr(new Xform(child, m_channels, m_errorReport));
            }
            else if ( type == "bounds" )
            {
                m_bounds = BoundsRcPtr(new Bounds(child, m_channels, m_errorReport));
            }
            else if ( type == "proxyList" )
            {
                m_proxyList = ProxyListRcPtr(new ProxyList(child, basedir, m_errorReport));
            }
            else if ( type == "arbitraryList" )
            {
                m_arbitraryList = ArbitraryListRcPtr(new ArbitraryList(child, m_channels, m_errorReport));
            }
            else if ( type == "lodData" )
            {
                m_lodData = LodDataRcPtr(new LodData(child, m_channels, m_errorReport));
            }
            else if( type == "lookFile" && PTreeHelper::hasAttr( child, "ref" ) )
            {
                m_lookfile = PTreeHelper::getAttr( child, "ref" );

                // if lookFilePath path isn't an explicit path, construct explicit path using basedir
                if (m_lookfile[0] != '/')
                {
                    m_lookfile = basedir + m_lookfile;
                }
            }
            else if( type == "modifiers" )
            {
                for ( ptree::const_iterator iter = child.begin(); iter != child.end(); ++iter )
                {
                    std::string const & type = (*iter).first;
                    ptree const & childAttr = (*iter).second;

                    // For now, only support attributeFile tag
                    if( type == "attributeFile" )
                    {
                        if( PTreeHelper::hasAttr( childAttr, "ref" ) )
                        {
                            std::string filepath = PTreeHelper::getAttr( childAttr, "ref" ) ;
                            // if filepath isn't an explicit path, construct explicit path using basedir
                            if (filepath[0] != '/')
                            {
                                filepath = basedir + filepath;
                            }

                            m_modifiers.push_back(ModifierRcPtr(new Modifier("AttributeFile", filepath, m_errorReport)));
                        }
                    }
                }
            }
            else if( type == "arg" )
            {
                // Note: Keeping channels here (although currently not used) as animation is probably needed
                args.push_back(ProceduralArgumentRcPtr(new ProceduralArgument(child, m_channels, m_errorReport)));
            }
       }
    }
    
    std::string InstanceBase::getName()
    {
        return m_name;
    }
    
    std::string InstanceBase::getLocationType()
    {
        return m_locationType;
    }

    void InstanceBase::setLocationType(std::string type)
    {
        m_locationType = type;
    }
    
    XformRcPtr InstanceBase::getXform()
    {
        return m_xform;
    }

    BoundsRcPtr InstanceBase::getBounds()
    {
        return m_bounds;
    }
    
    ProxyListRcPtr InstanceBase::getProxyList()
    {
        return m_proxyList;
    }

    ArbitraryListRcPtr InstanceBase::getArbitraryList()
    {
        return m_arbitraryList;
    }

    LodDataRcPtr InstanceBase::getLodData()
    {
        return m_lodData;
    }

    std::string InstanceBase::getLookfile()
    {
        return m_lookfile;
    }

    std::vector<ModifierRcPtr> InstanceBase::getModifiers()
    {
        return m_modifiers;
    }

    std::string InstanceBase::getErrorMessages()
    {
        return m_errorReport.getErrorMessages();
    }

    std::string InstanceBase::getWarningMessages()
    {
        return m_errorReport.getWarningMessages();
    }

    bool InstanceBase::hasErrors()
    {
        return m_errorReport.hasErrors();
    }
    
    bool InstanceBase::hasWarnings()
    {
        return m_errorReport.hasWarnings();
    }

    std::vector<ProceduralArgumentRcPtr> InstanceBase::getArgs()
    {
        return args;
    }
    
///////////////////////////////////////////////////////////////////////////////

    InstanceGroup::InstanceGroup(ptree element,
            ChannelRangeRcPtr channels, std::string basedir)
    : InstanceBase(element, channels, basedir)
    {
        if( element.empty() )
        {
            m_errorReport.addErrorMessage("ScenegraphXML: error reading XML element for InstanceGroup\n");
            return;
        }
        
        // if groupType explicitly set use this to set the Katana location type
        if( PTreeHelper::hasAttr( element, "groupType" ) )
        {
            m_locationType = PTreeHelper::getAttr( element, "groupType" );

            // over-ride for lodGroup type to get set location name to "level-of-detail group", because this is what UI expects
            if (m_locationType == "lodGroup")
            {
                m_locationType = "level-of-detail group";
            }

            // over-ride for lodNode type to get set location name to "level-of-detail", because this is what UI expects
            if (m_locationType == "lodNode")
            {
                m_locationType = "level-of-detail";
            }
        }
        else
        {
            m_locationType = "group";
        }

        // loop through instanceList to get data for the children
        for ( ptree::const_iterator iter = element.begin(); iter != element.end(); ++iter )
        {
            std::string const & type = (*iter).first;
            ptree const & child = (*iter).second;

            if ( type == "instanceList" )
            {
                readInstanceList(child, basedir);
            }
        }
    }
    
    void InstanceGroup::readInstanceList(ptree element, std::string basedir)
    {
        m_children.clear();
        
        //if( element.empty() )
        //{
        //   m_errorReport.addErrorMessage("ScenegraphXML: error reading XML data in readInstanceList\n");
        //    return;
        //}
        
        unsigned int numChildren = element.size();
        m_children.reserve(numChildren);
        
        for ( ptree::const_iterator iter = element.begin(); iter != element.end(); ++iter )
        {
            std::string const & type = (*iter).first;
            ptree const & child = (*iter).second;

            if ( type == "instance" )
            {
                if( !PTreeHelper::hasAttr( child, "type" ) )
                {
                    m_errorReport.addErrorMessage("scenegraphXML: XML element in instance list with no type\n");
                    return;
                }
                
                InstanceBaseRcPtr newInstance;
                
                std::string instanceType = PTreeHelper::getAttr( child, "type" );
                
                if (instanceType == "group")
                {
                    newInstance = InstanceBaseRcPtr(new InstanceGroup(child, m_channels, basedir));
                }
                else if (instanceType == "reference")
                {
                    newInstance = InstanceBaseRcPtr(new InstanceReference(child, m_channels, basedir));
                }
                else
                {
                    m_errorReport.addWarningMessage("scenegraphXML: XML element in instance list with unknown type " + instanceType + "\n");
                }
                
                if (newInstance)
                {
                    m_children.push_back(newInstance);
                }
            }
        }
    }
    
    size_t InstanceGroup::getNumberOfChildren()
    {
        return m_children.size();
    }
    
    InstanceBaseRcPtr InstanceGroup::getChild(size_t index)
    {
        if (index < m_children.size())
        {
            return m_children[index];
        }
        
        return InstanceBaseRcPtr();
    }

    InstanceGroupRcPtr InstanceGroup::getAs(InstanceBaseRcPtr p)
    {
        return boost::dynamic_pointer_cast<InstanceGroup>(p);
    }

///////////////////////////////////////////////////////////////////////////////
    
    InstanceReference::InstanceReference(ptree element, ChannelRangeRcPtr channels, std::string basedir)
    : InstanceBase(element, channels, basedir)
    {
        if( element.empty() )
        {
            m_errorReport.addErrorMessage("scenegraphXML: error reading XML element for InstanceReference\n");
        }
        
        // get reference type from XML, using type "xml" if not explicitly stated
        if( PTreeHelper::hasAttr( element, "refType" ) )
        {
            m_referenceType = PTreeHelper::getAttr( element, "refType" );
        }
        else
        {
            m_referenceType == "xml";
        }

        // set location type based on reference type
        if (m_referenceType == "xml")
        {
            m_locationType = "assembly";
        }
        else if (m_referenceType == "tako")
        {
            m_locationType = "component";
        }
        else if (m_referenceType == "abc")
        {
            m_locationType = "component";
        }
        else if (m_referenceType == "procedural")
        {
            m_locationType = "group";
        }
        else
        {
            m_errorReport.addErrorMessage("ScenegraphXML: unknown reference type:" + m_referenceType + "\n");
        }

        // get reference file from XML
        if( PTreeHelper::hasAttr( element, "refFile" ) )
        {
            m_referencePath = PTreeHelper::getAttr( element, "refFile" );

            // if m_reference path isn't an explicit path, construct explicit path using basedir
            if (m_referencePath[0] != '/')
            {
                m_referencePath = basedir + m_referencePath;
            }
        }
        else
        {
            m_errorReport.addErrorMessage("ScenegraphXML: no refFile specified for reference element\n");
        }
    }

    InstanceReferenceRcPtr InstanceReference::getAs(InstanceBaseRcPtr p)
    {
        return boost::dynamic_pointer_cast<InstanceReference>(p);
    }
    
    std::string InstanceReference::getReferenceType()
    {
        return m_referenceType;
    }

    std::string InstanceReference::getReferencePath()
    {
        return m_referencePath;
    }
    
}
