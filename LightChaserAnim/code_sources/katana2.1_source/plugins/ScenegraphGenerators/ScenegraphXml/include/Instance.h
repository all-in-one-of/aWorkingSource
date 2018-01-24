// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_INSTANCE_H
#define INCLUDED_INSTANCE_SCENE_INSTANCE_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"
#include "Xform.h"
#include "Modifier.h"
#include "Bounds.h"
#include "ProxyList.h"
#include "ArbitraryList.h"
#include "LodData.h"
#include "ProceduralArgument.h"

namespace instance_scene
{
    class InstanceBase
    {
    public:
        InstanceBase(ptree element, ChannelRangeRcPtr channels, std::string basedir);
        
        std::string getName();
        std::string getLocationType();
        void setLocationType(std::string type);
        XformRcPtr getXform();
        BoundsRcPtr getBounds();
        ProxyListRcPtr getProxyList();
        ArbitraryListRcPtr getArbitraryList();
        LodDataRcPtr getLodData();
        std::string getLookfile();
        std::vector<ModifierRcPtr> getModifiers();
        std::string getErrorMessages();
        std::string getWarningMessages();
        std::vector<ProceduralArgumentRcPtr> getArgs();
        bool hasErrors();
        bool hasWarnings();
        //without a vtable, we can't dynamic_cast
        virtual ~InstanceBase(){}
        
    protected:
        ChannelRangeRcPtr m_channels;
        ErrorReport m_errorReport;
        std::string m_locationType;
     
    private:
        std::string m_name;
        XformRcPtr m_xform;
        BoundsRcPtr m_bounds;
        ProxyListRcPtr m_proxyList;
        ArbitraryListRcPtr m_arbitraryList;
        LodDataRcPtr m_lodData;
        std::string m_lookfile;
        std::vector<ModifierRcPtr> m_modifiers;
        std::vector<ProceduralArgumentRcPtr> args;
    };
    
    typedef boost::shared_ptr<InstanceBase> InstanceBaseRcPtr;
    
    ///////////////////////////////////////////////////////////////////////////
    
    class InstanceGroup;
    typedef boost::shared_ptr<InstanceGroup> InstanceGroupRcPtr;
    
    class InstanceGroup : public InstanceBase
    {
    public:
        InstanceGroup(ptree element, ChannelRangeRcPtr channels, std::string basedir);
        static InstanceGroupRcPtr getAs(InstanceBaseRcPtr p);      
        size_t getNumberOfChildren();
        InstanceBaseRcPtr getChild(size_t index);       
        typedef std::vector<InstanceBaseRcPtr> InstanceList;      
        void readInstanceList(ptree element, std::string basedir);
        
    private:
        std::vector<InstanceBaseRcPtr> m_children;
    };
    
    
    ///////////////////////////////////////////////////////////////////////////
    
    class InstanceReference;
    typedef boost::shared_ptr<InstanceReference> InstanceReferenceRcPtr;

    class InstanceReference : public InstanceBase
    {
    public:
        InstanceReference(ptree element, ChannelRangeRcPtr channels, std::string basedir);
        static InstanceReferenceRcPtr getAs(InstanceBaseRcPtr p);     
        std::string getReferenceType();
        std::string getReferencePath();
    
    private:
        std::string m_referenceType;
        std::string m_referencePath;
    };
    
}


#endif
