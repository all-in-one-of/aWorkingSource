#include <iostream>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/op/FnGeolibOp.h>

#include <FnGeolib/util/Path.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>
#include <FnGeolibServices/FnMaterialResolveUtil.h>

namespace { //anonymous

// This creates scenegraph from the attr representation used by
// RuntimeUtils.GetScenegraphAsAttr

class AttributeSetOp : public Foundry::Katana::GeolibOp
{
private:
    typedef std::vector<FnAttribute::GroupAttribute> AttributeSetGroups;

public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        AttributeSetGroups attributeSetGroups;
        // support top-level arg definition
        attributeSetGroups.push_back(interface.getOpArg());

        // add batch arg definition(s)
        FnAttribute::GroupAttribute batchAttr = interface.getOpArg("batch");
        if (batchAttr.isValid())
        {
            int64_t numBatchAttr = batchAttr.getNumberOfChildren();
            for (int64_t i=0; i<numBatchAttr; ++i)
            {
                FnAttribute::GroupAttribute batchChild = batchAttr.getChildByIndex(i);
                if (batchChild.isValid())
                {
                    attributeSetGroups.push_back(batchChild);
                }
            }
        }

        // TODO: low-hanging fruit for optimization:
        // 1) combine arg definitions with the same locationPath/CEL
        // 2) prune arg definitions as we walk down and they can no longer
        //    affect children.

        bool continueToChildren = false;
        for (AttributeSetGroups::iterator iter=attributeSetGroups.begin();
             iter!=attributeSetGroups.end(); ++iter)
        {
            const FnAttribute::GroupAttribute & args = (*iter);
            if (!args.isValid()) continue;

            FnAttribute::GroupAttribute setAttrs =
                args.getChildByName("setAttrs");
            FnAttribute::StringAttribute deleteAttrNames =
                args.getChildByName("deleteAttrNames");
            FnAttribute::GroupAttribute execOps =
                args.getChildByName("execOps");

            if(!setAttrs.isValid() &&
                !deleteAttrNames.isValid() &&
                !execOps.isValid()) continue;

            FnAttribute::StringAttribute locationsAttr =
                args.getChildByName("locationPaths");

            FnAttribute::StringAttribute celAttr =
                args.getChildByName("CEL");

            if(!locationsAttr.isValid() && !celAttr.isValid()) continue;

            // Are we at any of the matching locationPaths
            bool canMatchChildren = false;
            bool localMatch = false;

            if (locationsAttr.isValid())
            {
                FnGeolibUtil::Path::FnMatchInfo matchInfo;
                FnAttribute::StringConstVector locationPaths =
                    locationsAttr.getNearestSample(0.0);
                for(unsigned int i=0; i<locationPaths.size(); ++i)
                {
                    FnGeolibUtil::Path::FnMatch(matchInfo,
                        interface.getOutputLocationPath(), locationPaths[i]);
                    if(matchInfo.match) localMatch = true;
                    if(matchInfo.canMatchChildren) canMatchChildren = true;
                    if(localMatch && canMatchChildren) break;
                }
            }
            else
            {
                FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo matchInfo;
                FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(
                        matchInfo, interface, celAttr);

                if(matchInfo.matches) localMatch = true;
                if(matchInfo.canMatchChildren) canMatchChildren = true;
            }

            if(localMatch)
            {
                if (setAttrs.isValid())
                {
                    for(int i=0; i<setAttrs.getNumberOfChildren(); ++i)
                    {
                        FnAttribute::GroupAttribute groupAttr = setAttrs.getChildByIndex(i);
                        if (!groupAttr.isValid()) continue;

                        FnAttribute::StringAttribute nameAttr = groupAttr.getChildByName("name");
                        FnAttribute::Attribute attr = groupAttr.getChildByName("attr");
                        if (!nameAttr.isValid() || !attr.isValid()) continue;


                        bool groupInherit = FnAttribute::IntAttribute(
                                groupAttr.getChildByName("inherit")).getValue(
                                    1, false);
                        


                        std::string name = nameAttr.getValue("", false);
                        if (name.empty()) continue;

                        interface.setAttr(name, attr, groupInherit);
                    }
                }

                if (deleteAttrNames.isValid())
                {
                    FnAttribute::StringConstVector deleteAttrNameValues =
                            deleteAttrNames.getNearestSample(0);

                    for (FnAttribute::StringConstVector::const_iterator
                            I = deleteAttrNameValues.begin(),
                                    E = deleteAttrNameValues.end(); I != E; ++I)
                    {
                        interface.deleteAttr(*I);
                    }
                }

                if (execOps.isValid())
                {
                    for(int i=0; i<execOps.getNumberOfChildren(); ++i)
                    {
                        FnAttribute::GroupAttribute groupAttr =
                                execOps.getChildByIndex(i);
                        if (!groupAttr.isValid()) {continue;}


                        FnAttribute::StringAttribute opType =
                                groupAttr.getChildByName("opType");
                
                        FnAttribute::GroupAttribute opArgs =
                                groupAttr.getChildByName("opArgs");

                        if (!opType.isValid() || !opArgs.isValid())
                        {
                            continue;
                        }

                        interface.execOp(opType.getValue("", false), opArgs);
                    }
                }
            }

            if (canMatchChildren) continueToChildren = true;
        }

        if (!continueToChildren)
        {
            interface.stopChildTraversal();
        }
    }
};

DEFINE_GEOLIBOP_PLUGIN(AttributeSetOp)

///////////////////////////////////////////////////////////////////////////////

bool matchesLocation(Foundry::Katana::GeolibCookInterface &interface,
        bool & outCanMatchChildren)
{
    bool localMatch = false;
    bool canMatchChildren = false;

    FnAttribute::StringAttribute locationsAttr =
            interface.getOpArg("locationPaths");

    FnAttribute::StringAttribute celAttr =
            interface.getOpArg("CEL");

    if (!locationsAttr.isValid() && !celAttr.isValid())
    {
        outCanMatchChildren = false;
        return false;
    }

    if (locationsAttr.isValid())
    {
        FnGeolibUtil::Path::FnMatchInfo matchInfo;
        FnAttribute::StringConstVector locationPaths =
            locationsAttr.getNearestSample(0.0);
        for(unsigned int i=0; i<locationPaths.size(); ++i)
        {
            FnGeolibUtil::Path::FnMatch(matchInfo,
                interface.getOutputLocationPath(), locationPaths[i]);
            if(matchInfo.match) localMatch = true;
            if(matchInfo.canMatchChildren) canMatchChildren = true;
            if(localMatch && canMatchChildren) break;
        }
    }
    else
    {
        FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo matchInfo;
        FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(
                matchInfo, interface, celAttr);

        if(matchInfo.matches) localMatch = true;
        if(matchInfo.canMatchChildren) canMatchChildren = true;
    }

    outCanMatchChildren = canMatchChildren;
    return localMatch;
}

///////////////////////////////////////////////////////////////////////////////


class AttributeSetIfNotPresentOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
                Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    
    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {

        bool canMatchChildren = false;
        bool localMatch = matchesLocation(interface, canMatchChildren);

        if (!canMatchChildren)
        {
            interface.stopChildTraversal();
        }

        if (!localMatch)
        {
            return;
        }
        
        FnAttribute::GroupAttribute setAttrs = interface.getOpArg("setAttrs");
        for (int64_t i = 0, e = setAttrs.getNumberOfChildren(); i < e; ++i)
        {
            FnAttribute::GroupAttribute groupAttr =
                    setAttrs.getChildByIndex(i);
            
            if (!groupAttr.isValid())
            {
                continue;
            }

            FnAttribute::StringAttribute nameAttr =
                    groupAttr.getChildByName("name");
            FnAttribute::Attribute attr = groupAttr.getChildByName("attr");
            if (!nameAttr.isValid() || !attr.isValid())
            {
                continue;
            }

            bool groupInherit = FnAttribute::IntAttribute(
                    groupAttr.getChildByName("inherit")).getValue(1, false);
            
            std::string name = nameAttr.getValue("", false);
            if (name.empty())
            {
                continue;
            }

            bool queryGlobal = FnAttribute::IntAttribute(
                    groupAttr.getChildByName("queryGlobal")).getValue(
                            0, false);
            

            bool attrExists = false;

            if (queryGlobal)
            {
                 attrExists = Foundry::Katana::GetGlobalAttr(
                        interface, name).isValid();
            }
            else
            {
                attrExists = interface.getAttr(name).isValid();
            }


            if (!attrExists)
            {
                interface.setAttr(name, attr, groupInherit);
            }
        }
    }




};

DEFINE_GEOLIBOP_PLUGIN(AttributeSetIfNotPresentOp)

//////////////////////////////////////////////////////////////////////////////

class OverrideDefineOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
                Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }



    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        bool canMatchChildren = false;
        bool localMatch = matchesLocation(interface, canMatchChildren);

        if (!canMatchChildren)
        {
            interface.stopChildTraversal();
        }

        if (!localMatch)
        {
            return;
        }
        

        std::string attrPrefix;

        FnAttribute::GroupAttribute typeSpecificPrefixes =
                interface.getOpArg("typePrefixes");        
        FnAttribute::StringAttribute typeSpecificPrefixAttr =
                typeSpecificPrefixes.getChildByName(
                        Foundry::Katana::GetInputLocationType(interface));
        
        //used only for layeredMaterialOverride case
        FnAttribute::GroupAttribute inputMaterial;

        if (typeSpecificPrefixAttr.isValid())
        {
            attrPrefix = typeSpecificPrefixAttr.getValue("", false);
            
            //special case for layeredMaterialOverride
            if (FnAttribute::StringAttribute(interface.getOpArg(
                    "defaultPrefix")).getValue(
                        "", false) == "layeredMaterialOverride.")
            {
                inputMaterial = FnGeolibServices::FnMaterialResolveUtil::
                        resolveMaterialReferences(
                                Foundry::Katana::GetGlobalAttr(
                                        interface, "material"), false);
            }
        }
        else
        {
            attrPrefix = FnAttribute::StringAttribute(
                interface.getOpArg("defaultPrefix")).getValue("", false);
        }

        FnAttribute::GroupAttribute setAttrs = interface.getOpArg("setAttrs");
        for (int64_t i = 0, e = setAttrs.getNumberOfChildren(); i < e; ++i)
        {
            FnAttribute::GroupAttribute groupAttr =
                    setAttrs.getChildByIndex(i);
            
            if (!groupAttr.isValid())
            {
                continue;
            }

            FnAttribute::StringAttribute nameAttr =
                    groupAttr.getChildByName("name");
            FnAttribute::Attribute attr = groupAttr.getChildByName("attr");
            if (!nameAttr.isValid() || !attr.isValid())
            {
                continue;
            }

            bool groupInherit = FnAttribute::IntAttribute(
                    groupAttr.getChildByName("inherit")).getValue(1, false);
            
            std::string name = nameAttr.getValue("", false);
            if (name.empty())
            {
                continue;
            }

            if (inputMaterial.isValid())
            {
                //will only be reached in the layeredMaterialOverride case
                FnAttribute::GroupBuilder gb;
                gb.set(attrPrefix + name, attr);

                setLeafAttrs(interface,
                        FnGeolibServices::FnMaterialResolveUtil::
                                combineLayeredMaterialOverrides(
                                        inputMaterial,
                                        FnAttribute::GroupAttribute(true),
                                        gb.build()),
                                                groupInherit);

            }
            else
            {
                interface.setAttr(attrPrefix + name, attr, groupInherit);
            }
        }
    }
    
    static void setLeafAttrs(Foundry::Katana::GeolibCookInterface &interface,
            FnAttribute::GroupAttribute groupAttr, bool groupInherit,
                    const std::string & attrPath=std::string())
    {
        int64_t numberOfChildren = groupAttr.getNumberOfChildren();

        for (int64_t i = 0; i < numberOfChildren; ++i)
        {
            std::string childName = groupAttr.getChildName(i);
            
            std::string childPath;
            if (attrPath.empty())
            {
                childPath = childName;
            }
            else
            {
                childPath = attrPath + "." + childName;
            }

            FnAttribute::Attribute attr = groupAttr.getChildByIndex(i);

            FnAttribute::GroupAttribute childGroup(attr);
            if (childGroup.isValid())
            {
                setLeafAttrs(interface, childGroup, groupInherit, childPath);
            }
            else
            {
                interface.setAttr(childPath, attr);
            }
        }
    }

};

DEFINE_GEOLIBOP_PLUGIN(OverrideDefineOp)


} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(AttributeSetOp, "AttributeSet", 0, 1);
    REGISTER_PLUGIN(AttributeSetIfNotPresentOp, "AttributeSetIfNotPresent",
            0, 1);
    REGISTER_PLUGIN(OverrideDefineOp, "OverrideDefine", 0, 1);
    
}

