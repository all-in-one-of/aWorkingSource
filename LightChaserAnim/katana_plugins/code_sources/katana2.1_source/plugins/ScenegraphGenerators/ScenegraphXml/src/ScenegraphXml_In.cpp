// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include "ScenegraphXml_In.h"
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>
#include <iostream>
#include <string.h>
#include <sstream>


namespace instance_scene
{

/////////
/// Scenegraph XML In - Generator
///
ScenegraphXml_In::ScenegraphXml_In() {}

ScenegraphXml_In::~ScenegraphXml_In() {}

FnKat::ScenegraphGenerator* ScenegraphXml_In::create()
{
    return (FnKat::ScenegraphGenerator*)new ScenegraphXml_In();
}

bool ScenegraphXml_In::setArgs(FnKat::GroupAttribute args)
{
    /// Since only the procedural itself has access to getArgs,
    /// we grab what we need here and store it in the ref-counted
    /// structure passed down to all of the iterators.
    shared_args = ProcArgsRcPtr(new ProcArgs);

    if( !checkArgs(args) ) return false;

    FnKat::FloatAttribute inTimeAttr = args.getChildByName("system.timeSlice.currentTime");
    FnKat::IntAttribute numSamplesAttr = args.getChildByName("system.timeSlice.numSamples");
    FnKat::StringAttribute filepathAttr = args.getChildByName("filepath");

    /// systemArgs is always present and lets us know what frame
    /// we should load
    FnKat::FloatConstVector values_inTime = inTimeAttr.getNearestSample(0);
    if( values_inTime.size() > 0 )
    {
        shared_args->frame = (int)values_inTime[0];
    }

    /// We're also told here how many samples to load.
    /// The GUI typically only loads one sample per frame but
    /// a render can ask for more. Since our simple format doesn't
    /// store sub-frame samples, we just interpret anything over 1
    /// as needing animation.
    FnKat::IntConstVector values_numSamples = numSamplesAttr.getNearestSample(0);
    if( values_numSamples.size() > 0 && values_numSamples[0] > 1 )
    {
        shared_args->animated = true;
    }

    /// grab the file path to our root-level Scene
    FnKat::StringConstVector values_filepath = filepathAttr.getNearestSample(0);
    if( values_filepath.size() > 0 )
    {
        filepath = std::string(values_filepath[0]);
    }

    return true;
}

FnKat::GroupAttribute ScenegraphXml_In::getArgumentTemplate()
{
    FnKat::GroupBuilder gb;

    gb.set("filepath", FnKat::StringAttribute(""));

    return gb.build();
}

FnKat::ScenegraphContext *ScenegraphXml_In::getRoot()
{
    return new RootIteratorContext(shared_args, filepath);
}

void ScenegraphXml_In::flush()
{
    // Iterate over the scenes and flush the caches
    std::vector<Scene*> scenes = Scene::getAllScenes();
    for (unsigned int i=0; i < scenes.size(); i++)
    {
        scenes.at(i)->flushCache();
    }
}

/// Utility functions for converting our common data objects into attributes
/// with sampling appropriate to the arguments
namespace
{
#ifdef D_UNUSED
FnKat::Attribute ProceduralArgsToAttr(ProcArgsRcPtr args, std::map<std::string, std::string> procArgs)
{
    if ( !procArgs.empty() )
    {
        FnKat::GroupBuilder gb;
        std::vector<std::string> samples;

        std::map<std::string, std::string>::iterator arg;
        for( arg = procArgs.begin(); arg != procArgs.end(); arg++ )
        {
           samples.push_back(arg->second);
        }

        gb.set("args", FnKat::StringAttribute(samples));
        return gb.build();
    }

    return FnKat::Attribute();
}
#endif //D_UNUSED

FnKat::Attribute BoundsToAttr(ProcArgsRcPtr args, BoundsRcPtr bounds)
    {
        if ( bounds )
        {
            FnKat::DoubleBuilder samples;
            int frames = ( args->animated && bounds->isAnimated() ) ? 2 : 1;
            for ( int i = 0; i < frames; i++) {
                samples.push_back(bounds->getMinX(args->frame+i),(float)i);
                samples.push_back(bounds->getMaxX(args->frame+i),(float)i);
                samples.push_back(bounds->getMinY(args->frame+i),(float)i);
                samples.push_back(bounds->getMaxY(args->frame+i),(float)i);
                samples.push_back(bounds->getMinZ(args->frame+i),(float)i);
                samples.push_back(bounds->getMaxZ(args->frame+i),(float)i);
            }
            return samples.build();

        }

        return FnKat::Attribute();
    }

FnKat::Attribute XformToAttr(ProcArgsRcPtr args, XformRcPtr xform)
    {
        if ( xform )
        {
            FnKat::GroupBuilder gb;
            FnKat::DoubleBuilder samples(4);
            xform->getValues(args->frame, samples.get() );
            if (args->animated && xform->isAnimated())
            {
                xform->getValues(args->frame+1, samples.get(1.f) );
            }
            gb.set("matrix", samples.build() );
            gb.setGroupInherit(false);
            return gb.build();
        }

        return FnKat::Attribute();
    }

FnKat::Attribute ProxyListToAttr(ProcArgsRcPtr args, ProxyListRcPtr proxyList)
    {
        if ( proxyList )
        {
            FnKat::GroupBuilder gb;

            size_t numProxies = proxyList->getNumberOfProxies();
            for ( size_t i = 0; i < numProxies; i++ )
            {
                ProxyDataRcPtr curProxy = proxyList->getProxy(i);

                FnKat::GroupBuilder opGb;
                opGb.set("opType", FnKat::StringAttribute("AlembicIn"));

                // Build the attributes for the AlembicIn Op:
                opGb.set("opArgs.fileName", FnKat::StringAttribute(
                         curProxy->getReferencePath()));

                std::stringstream groupName;
                groupName << "viewer.Proxy_Op" << i;
                gb.set(groupName.str(), opGb.build());
            }

            return gb.build();
        }

        return FnKat::Attribute();
    }

FnKat::Attribute ArbitraryListToAttr(ProcArgsRcPtr args, ArbitraryListRcPtr arbitraryList)
    {
        if ( arbitraryList )
        {
            FnKat::GroupBuilder gb;

            size_t numAttrs = arbitraryList->getNumberOfAttrs();
            for ( size_t i = 0; i < numAttrs; i++ )
            {
                AttrDataBaseRcPtr curAttrData = arbitraryList->getAttrData(i);

                if (AttrDataFloatRcPtr attrDataFloat = AttrDataFloat::getAs(curAttrData))
                {
                    FnKat::DoubleBuilder samples;
                    samples.push_back(attrDataFloat->getValue(args->frame));
                    if (args->animated && attrDataFloat->isAnimated())
                    {
                        samples.push_back(attrDataFloat->getValue(args->frame+1), 1.f);
                    }
                    gb.set(attrDataFloat->getName(), samples.build());
                }
                else if (AttrDataFloatListRcPtr attrDataFloatList = AttrDataFloatList::getAs(curAttrData))
                {
                    FnKat::DoubleBuilder samples;
                    attrDataFloatList->getValues(args->frame, samples.get() );
                    if (args->animated && attrDataFloatList->isAnimated())
                    {
                        attrDataFloatList->getValues(args->frame+1, samples.get(1.f) );
                    }
                    gb.set(attrDataFloatList->getName(), samples.build());
                }
                else if (AttrDataStringRcPtr attrDataString = AttrDataString::getAs(curAttrData))
                {
                    gb.set(attrDataString->getName(), FnKat::StringAttribute(attrDataString->getValue()));
                }
                else
                {
                    // silent for now, but should trap for case that no valid match for
                    // attribute type found
                }
            }

            return gb.build();
        }

        return FnKat::Attribute();
    }

FnKat::Attribute LodDataToAttr(ProcArgsRcPtr args, LodDataRcPtr lodData)
    {
        if ( lodData )
        {
            FnKat::GroupBuilder gb;

            gb.set("componentLodTag", FnKat::StringAttribute(lodData->getTag()));

            FnKat::FloatBuilder samples;
            samples.push_back( lodData->getWeight(args->frame) );
            if( args->animated && lodData->isAnimated() )
            {
                samples.push_back( lodData->getWeight(args->frame+1), 1.f );
            }

            gb.set("componentLodWeight", samples.build());

            return gb.build();
        }

        return FnKat::Attribute();
    }


    /// Three instance types doesn't warrant a fancy factory so we'll
    /// just dispatch with dynamic_casts here.
    /// This is used by Scenes and InstanceGroups
    FnKat::ScenegraphContext * InstanceBaseToContext(ProcArgsRcPtr args, InstanceGroupRcPtr parentGroup, size_t index)
    {
        //exit early if there's no parent or we're at the end of the list
        //of advertised children
        if (!parentGroup || index >= parentGroup->getNumberOfChildren())
        {
            return 0;
        }

        InstanceBaseRcPtr instance = parentGroup->getChild(index);

        if (InstanceGroupRcPtr instanceGroup = InstanceGroup::getAs(instance))
        {
            return new InstanceGroupIteratorContext(args, instanceGroup, parentGroup, index);
        }
        else if (InstanceReferenceRcPtr instanceReference = InstanceReference::getAs(instance))
        {
            return new InstanceReferenceIteratorContext(args, instanceReference, parentGroup, index);
        }

        //if we couldn't make it for some reason, go onto the next index
        return InstanceBaseToContext(args, parentGroup, index+1);
    }
}


/////////
/// Root Iterator
///
RootIteratorContext::RootIteratorContext(ProcArgsRcPtr args, const std::string & filepath)
: m_args(args)
, m_filepath(filepath)
{
}

RootIteratorContext::RootIteratorContext(const RootIteratorContext & rhs)
: m_args(rhs.m_args)
, m_filepath(rhs.m_filepath)
{
}

FnKat::ScenegraphContext *RootIteratorContext::getFirstChild() const
{
    return new SceneIteratorContext(m_args, m_filepath);
}

void RootIteratorContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();
    names->push_back("type");
}

FnKat::Attribute RootIteratorContext::getLocalAttr(const std::string & name) const
{
    if (name == "type")
    {
        return FnKat::StringAttribute("group");
    }

    return FnKat::Attribute();
}


/////////
/// Scene Iterator
///
SceneIteratorContext::SceneIteratorContext(ProcArgsRcPtr args, const std::string & filepath)
: m_args(args)
, m_filepath(filepath)
, m_scene(Scene::loadCached(filepath))
{
    if ( m_scene )  // TODO: Review after implementing InstanceBaseToContext
    {
        m_curLocationIterator= IteratorContextRcPtr(InstanceBaseToContext(m_args, m_scene->getInstanceList(), 0));
    }
}

SceneIteratorContext::SceneIteratorContext(const SceneIteratorContext & rhs)
: m_args(rhs.m_args)
, m_filepath(rhs.m_filepath)
, m_scene(rhs.m_scene)
, m_curLocationIterator(rhs.m_curLocationIterator)
{
}

FnKat::ScenegraphContext *SceneIteratorContext::getFirstChild() const
{
    /// no scene for some reason?, no first child
    if (!m_curLocationIterator)
    {
        return 0x0;
    }

    return m_curLocationIterator->getFirstChild();
}

FnKat::ScenegraphContext *SceneIteratorContext::getNextSibling() const
{
    /// no iterator context for the scene's dummy group for some reason?, no next sibling
    if ( !m_curLocationIterator )
    {
        return 0x0;
    }

    return m_curLocationIterator->getNextSibling();
}

void SceneIteratorContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();

    if ( !m_curLocationIterator || !m_scene || m_scene->hasErrors() )
    {
        names->push_back("name");
        names->push_back("type");
        names->push_back("errorMessage");
    }

    if ( m_curLocationIterator )
    {
        m_curLocationIterator->getLocalAttrNames(names);
    }
}

FnKat::Attribute SceneIteratorContext::getLocalAttr(const std::string & name) const
{
    if ( m_curLocationIterator )
    {
        return m_curLocationIterator->getLocalAttr(name);
    }

    // This will always be an error case, since m_curLocationIterator was null above.
    if (name == "name")
    {
        return FnKat::StringAttribute("error");
    }
    if (name == "type")
    {
        return FnKat::StringAttribute("error");
    }
    if (name == "errorMessage")
    {
        std::string errMsg = "Unknown error";
        if ( m_scene )
        {
            if ( m_scene->hasErrors() )
            {
                errMsg = m_scene->getErrorMessages();
            }
        }
        else
        {
            errMsg = "Cannot load: " + m_filepath;
        }
        return FnKat::StringAttribute(errMsg);
    }

    return FnKat::Attribute();
}


/////////
/// Instance - Base
///
InstanceBaseIteratorContext::InstanceBaseIteratorContext(ProcArgsRcPtr args, InstanceBaseRcPtr baseInstance, InstanceGroupRcPtr parentGroup, size_t index)
: m_args(args)
, m_parentGroup(parentGroup)
, m_baseInstance(baseInstance)
, m_index(index)
{
}

InstanceBaseIteratorContext::InstanceBaseIteratorContext(const InstanceBaseIteratorContext & rhs)
: m_args(rhs.m_args)
, m_parentGroup(rhs.m_parentGroup)
, m_baseInstance(rhs.m_baseInstance)
, m_index(rhs.m_index)
{
}

FnKat::ScenegraphContext *InstanceBaseIteratorContext::getNextSibling() const
{
    /// our next sibling is just our index + 1.
    return InstanceBaseToContext(m_args, m_parentGroup, m_index + 1);
}

void InstanceBaseIteratorContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();
    names->push_back("name");
    names->push_back("type");

    if ( !m_baseInstance || m_baseInstance->hasErrors() )
    {
        names->push_back("errorMessage");
    }

    if( m_baseInstance )
    {
        if ( m_baseInstance->getXform() )
        {
            names->push_back("xform");
        }

        if ( m_baseInstance->getBounds() )
        {
            names->push_back("bound");
        }

        if ( m_baseInstance->getProxyList() )
        {
            names->push_back("proxies");
        }

        if ( m_baseInstance->getArbitraryList() )
        {
            names->push_back("scenegraphXmlAttrs");
        }

        if ( m_baseInstance->getLodData() )
        {
            names->push_back("info");
        }

        if ( !m_baseInstance->getLookfile().empty() )
        {
            names->push_back("lookfile");
        }

        if ( m_baseInstance->getModifiers().size() )
        {
            names->push_back("attributeModifiers");
        }
    }
}

FnKat::Attribute InstanceBaseIteratorContext::getLocalAttr(const std::string & name) const
{
    if (name == "name")
    {
        if ( m_baseInstance )
        {
            return FnKat::StringAttribute(m_baseInstance->getName());
        }
        else
        {
            std::ostringstream buffer;
            buffer << "instanceError" << m_index;

            return FnKat::StringAttribute(buffer.str());
        }
    }

    if (name == "type")
    {
        if ( m_baseInstance && !m_baseInstance->hasErrors() )
        {
            return FnKat::StringAttribute(m_baseInstance->getLocationType());
        }
        else
        {
            return FnKat::StringAttribute("error");
        }
    }

    if (name == "errorMessage")
    {
        if ( m_baseInstance )
        {
            if ( m_baseInstance->hasErrors() )
            {
                return FnKat::StringAttribute(m_baseInstance->getErrorMessages());
            }
        }
        else
        {
            return FnKat::StringAttribute("invalid instance");
        }
    }

    if (name == "xform" && m_baseInstance )
    {
        return XformToAttr(m_args, m_baseInstance->getXform());
    }

    if (name == "bound" && m_baseInstance )
    {
        return BoundsToAttr(m_args, m_baseInstance->getBounds());
    }

    if (name == "proxies" && m_baseInstance )
    {
        return ProxyListToAttr(m_args, m_baseInstance->getProxyList());
    }

    if (name == "scenegraphXmlAttrs" && m_baseInstance )
    {
        return ArbitraryListToAttr(m_args, m_baseInstance->getArbitraryList());
    }

    if (name == "info" && m_baseInstance )
    {
        return LodDataToAttr(m_args, m_baseInstance->getLodData());
    }

    if (name == "lookfile" && !m_baseInstance->getLookfile().empty() )
    {
        FnKat::GroupBuilder gb;
        gb.set("asset", FnKat::StringAttribute(m_baseInstance->getLookfile()));

        return gb.build();
    }

    if (name == "attributeModifiers" && m_baseInstance )
    {
        FnKat::GroupBuilder gb;

        size_t numMods = m_baseInstance->getModifiers().size();
        for ( size_t i = 0; i < numMods; i++ )
        {
            ModifierRcPtr modifier = m_baseInstance->getModifiers().at(i);

            FnKat::GroupBuilder modGroup;
            modGroup.set("type", FnKat::StringAttribute(modifier->getType()));
            modGroup.set("args.filepath", FnKat::StringAttribute(modifier->getFilepath()));
            modGroup.set("recursiveEnable", FnKat::IntAttribute(1));
            std::stringstream groupName;
            groupName << modifier->getType() << i;
            gb.set(groupName.str(), modGroup.build());
        }

        return gb.build();
    }


    return FnKat::Attribute();
}


/////////
/// Instance - Group
///
InstanceGroupIteratorContext::InstanceGroupIteratorContext(ProcArgsRcPtr args, InstanceGroupRcPtr instanceGroup, InstanceGroupRcPtr parentGroup, size_t index)
: InstanceBaseIteratorContext(args, instanceGroup, parentGroup, index)
, m_instanceGroup(instanceGroup)
{
}

InstanceGroupIteratorContext::InstanceGroupIteratorContext(const InstanceGroupIteratorContext & rhs)
: InstanceBaseIteratorContext(rhs)
, m_instanceGroup(rhs.m_instanceGroup)
{
}

FnKat::ScenegraphContext *InstanceGroupIteratorContext::getFirstChild() const
{
    return InstanceBaseToContext(m_args, m_instanceGroup, 0);
}


/////////
/// Instance - Reference
///
InstanceReferenceIteratorContext::InstanceReferenceIteratorContext( ProcArgsRcPtr args,
                                                                    InstanceReferenceRcPtr instanceReference,
                                                                    InstanceGroupRcPtr parentGroup,
                                                                    size_t index):
    InstanceBaseIteratorContext(args, instanceReference, parentGroup, index)
    , m_instanceReference(instanceReference)
{
}

InstanceReferenceIteratorContext::InstanceReferenceIteratorContext(const InstanceReferenceIteratorContext & rhs)
: InstanceBaseIteratorContext(rhs)
, m_instanceReference(rhs.m_instanceReference)
{
}

FnKat::ScenegraphContext *InstanceReferenceIteratorContext::getFirstChild() const
{
    if ( m_instanceReference )
    {
        if ( m_instanceReference->getReferenceType() == "xml" )
            return new SceneIteratorContext(m_args, m_instanceReference->getReferencePath());
        else if ( m_instanceReference->getReferenceType() == "tako" )
            return new HDFIteratorContext(m_args, m_instanceReference->getReferencePath());
        else if ( m_instanceReference->getReferenceType() == "abc" )
            return new ABCIteratorContext(m_args, m_instanceReference->getReferencePath());
        else if ( m_instanceReference->getReferenceType() == "procedural" )
            return new ProceduralIteratorContext(m_args, m_instanceReference->getReferencePath(), m_instanceReference->getArgs());
    }

    return 0x0;
}


/////////
/// HDF (tako)
///
HDFIteratorContext::HDFIteratorContext(ProcArgsRcPtr args, const std::string & filepath)
: m_args(args)
, m_filepath(filepath)
{
}

HDFIteratorContext::HDFIteratorContext(const HDFIteratorContext & rhs)
: m_args(rhs.m_args)
, m_filepath(rhs.m_filepath)
{
}

void HDFIteratorContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    /// we're going to describe an HDF_In procedural as a scenegraph
    names->clear();
    names->push_back("name");
    names->push_back("type");
    names->push_back("path"); //dso name
    names->push_back("hdfFile"); //expected by HDF_In
}

FnKat::Attribute HDFIteratorContext::getLocalAttr(const std::string & name) const
{
    if (name == "type")
    {
        return FnKat::StringAttribute("procedural");
    }

    if (name == "name")
    {
        return FnKat::StringAttribute("geometry");
    }

    if (name == "path")
    {
        return FnKat::StringAttribute("HDF_In");
    }

    if (name == "hdfFile")
    {
        return FnKat::StringAttribute(m_filepath);
    }

    return FnKat::Attribute();
}


/////////
/// Alembic (abc)
///
ABCIteratorContext::ABCIteratorContext(ProcArgsRcPtr args, const std::string & filepath)
: m_args(args)
, m_filepath(filepath)
, m_sgAttr()
{
}

ABCIteratorContext::ABCIteratorContext(const ABCIteratorContext & rhs)
: m_args(rhs.m_args)
, m_filepath(rhs.m_filepath)
, m_sgAttr(rhs.m_sgAttr)
{
}

void ABCIteratorContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();
    names->push_back("name");
    names->push_back("scenegraphGenerator.ops");
}

FnKat::Attribute ABCIteratorContext::getLocalAttr(const std::string & name) const
{
    if (name == "name")
    {
        return FnKat::StringAttribute("geometry");
    }

    if (name == "scenegraphGenerator.ops")
    {
        if (!m_sgAttr.isValid())
        {
            FnKat::GroupBuilder opsGb;

            FnKat::GroupBuilder alembicInGb;
            alembicInGb.set("opType", FnKat::StringAttribute("AlembicIn"));
            alembicInGb.set("opArgs.fileName",
                FnKat::StringAttribute(m_filepath));
            alembicInGb.set("opArgs.fps", FnKat::FloatAttribute(24.0));
            alembicInGb.set("opArgs.addForceExpand", FnKat::IntAttribute(1));
            alembicInGb.set("opArgs.addBounds",
                FnKat::StringAttribute("children"));

            opsGb.set("alembicIn_op", alembicInGb.build());
            opsGb.setGroupInherit(false);

            m_sgAttr = opsGb.build();
        }
        return m_sgAttr;
    }

    return FnKat::Attribute();
}

/////////
/// Renderer Procedural
///
ProceduralIteratorContext::ProceduralIteratorContext(ProcArgsRcPtr args, const std::string & filepath,
                                                   std::vector<ProceduralArgumentRcPtr> procArgs)
: m_args(args)
, m_filepath(filepath)
, m_procArgs(procArgs)
{
}

ProceduralIteratorContext::ProceduralIteratorContext(const ProceduralIteratorContext & rhs)
: m_args(rhs.m_args)
, m_filepath(rhs.m_filepath)
, m_procArgs(rhs.m_procArgs)
{
}

void ProceduralIteratorContext::getLocalAttrNames(std::vector<std::string> *names) const
{
    names->clear();
    names->push_back("name");
    names->push_back("type");
    names->push_back("path"); // so name

    names->push_back("rendererProcedural");
}

FnKat::Attribute ProceduralIteratorContext::getLocalAttr(const std::string & name) const
{
    if (name == "type")
    {
        return FnKat::StringAttribute("renderer procedural");
    }

    if (name == "name")
    {
       return FnKat::StringAttribute("proc");
    }

    if (name == "path")
    {
       return FnKat::StringAttribute(m_filepath);
    }

    if (name == "rendererProcedural")
    {
       FnKat::GroupBuilder gb;

       gb.set("procedural", FnKat::StringAttribute(m_filepath));

       std::vector<ProceduralArgumentRcPtr>::const_iterator arg;
       for( arg = m_procArgs.begin(); arg != m_procArgs.end(); arg++ )
       {
           ProceduralArgument procArg = *(*arg);
           gb.set(procArg.getName(), procArg.getValue());
       }

       return gb.build();
    }

   return FnKat::Attribute();
}

//Plugin Registration code
// The DEFINE_XXXX_PLUGIN macro must be called from inside the
// namespace of the plugin class
DEFINE_SGG_PLUGIN(ScenegraphXml_In)


}

// This must be defined outside the namespace of the plugin classe
void registerPlugins()
{
    REGISTER_PLUGIN(instance_scene::ScenegraphXml_In, "ScenegraphXml", 0, 1);
}
