#include <iostream>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/op/FnGeolibOp.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include <pystring/pystring.h>


//TEMPORARILY INCLUDE A COPY OF THE DECODE FUNCTION


namespace { //anonymous

// This creates scenegraph from the attr representation used by
// RuntimeUtils.GetScenegraphAsAttr

class StaticSceneCreateOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // This prevents us from building errant hierarchy when we've got an
        // input scene. It works because we're creating children with new
        // input args which replace this decision
        if (interface.atRoot())
        {
            interface.stopChildTraversal();
        }

        bool skipLocalActionsIfInputExists =
                FnAttribute::IntAttribute(interface.getOpArg(
                        "skipLocalActionsIfInputExists")).getValue(0, false);

        if (!skipLocalActionsIfInputExists || !interface.doesLocationExist())
        {
            // Set the attrs
            FnAttribute::GroupAttribute a = interface.getOpArg("a");
            if(a.isValid())
            {
                
                if (FnAttribute::IntAttribute(interface.getOpArg(
                        "setAttrsAtLeaf")).getValue(0, false))
                {
                    setLeafAttrs(interface, a);
                }
                else
                {
                    for (int i=0; i<a.getNumberOfChildren(); ++i)
                    {
                        interface.setAttr(a.getChildName(i),
                                a.getChildByIndex(i));
                    }
                }
            }
        }

        // Create the children
        FnAttribute::GroupAttribute c = interface.getOpArg("c");
        if(c.isValid())
        {
            for(int childindex=0; childindex<c.getNumberOfChildren(); ++childindex)
            {
                std::string childName = FnAttribute::DelimiterDecode(c.getChildName(childindex));
                FnAttribute::GroupAttribute childArgs = c.getChildByIndex(childindex);

                interface.createChild(childName, "", childArgs);
            }
        }
        


        // Exec some ops?
        if (!skipLocalActionsIfInputExists || !interface.doesLocationExist())
        {
            FnAttribute::GroupAttribute opGroups = interface.getOpArg("x");
            if(opGroups.isValid())
            {

                for(int childindex=0; childindex<opGroups.getNumberOfChildren();
                        ++childindex)
                {
                    FnAttribute::GroupAttribute entry =
                            opGroups.getChildByIndex(childindex);

                    if (!entry.isValid())
                    {
                        continue;
                    }

                    FnAttribute::StringAttribute opType =
                            entry.getChildByName("opType");
                    
                    FnAttribute::GroupAttribute opArgs =
                            entry.getChildByName("opArgs");

                    if (!opType.isValid() || !opArgs.isValid())
                    {
                        continue;
                    }

                    interface.execOp(opType.getValue("", false), opArgs);
                }
            }
        }
    }


    static void setLeafAttrs(Foundry::Katana::GeolibCookInterface &interface,
            FnAttribute::GroupAttribute groupAttr,
            const std::string & attrPath=std::string(),
            bool groupInherit=true)
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
                // Acquire groupInherit from first group level only. This will
                // only be obeyed by setAttr() in the case of group creation.
                setLeafAttrs(interface, childGroup, childPath,
                        groupInherit && attrPath.empty() ?
                                childGroup.getGroupInherit() :
                                groupInherit);
            }
            else
            {
                interface.setAttr(childPath, attr, groupInherit);
            }
        }
    }

};
DEFINE_GEOLIBOP_PLUGIN(StaticSceneCreateOp)


class HierarchyCollapseOp : public Foundry::Katana::GeolibOp
{

public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // Optional CEL support. if CEL is omitted, we DO want to run
        FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
        if (celAttr.isValid())
        {
            FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
            FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,
                                                                     interface,
                                                                     celAttr);
            if (!info.canMatchChildren)
            {
                interface.stopChildTraversal();
            }
            if (!info.matches)
            {
                return;
            }
        }

        // Pass 1:
        // Do a breadth first traversal of the scene beneath here, filling
        // up deeplocations with all of the child location names
        // 
        // This MUST be done as a preprocess for efficiency
        // This MUST be done breadth first for efficiency

        std::vector<std::string> deepLocations;
        {
            FnAttribute::StringAttribute pcAttr = 
                interface.getPotentialChildren();
            FnAttribute::StringAttribute::array_type pc = 
                pcAttr.getNearestSample(0.f);
            for(unsigned int i=0; i<pc.size(); ++i)
            {
                interface.prefetch(pc[i]);
                deepLocations.push_back(pc[i]);
            }
        }

        for(unsigned int i=0; i<deepLocations.size(); ++i)
        {
            FnAttribute::StringAttribute pcAttr = 
                interface.getPotentialChildren(deepLocations[i]);
            FnAttribute::StringAttribute::array_type pc = 
                pcAttr.getNearestSample(0.f);
            for(unsigned int j=0; j<pc.size(); ++j)
            {
                std::string newlocation = deepLocations[i] + "/" + pc[j];
                interface.prefetch(newlocation);
                deepLocations.push_back(newlocation);
            }
        }

        // Pass 2:
        // All locations beneath us are cooked.  Bundle up the attrs in an
        // Uber groupbuilder

        FnAttribute::GroupBuilder gb;
        if(!FnAttribute::IntAttribute(
            interface.getOpArg("omitLocals")).getValue(0, false))
        {
            gb.set("a", interface.getAttr(""));
            interface.deleteAttrs();
        }

        for(unsigned int i=0; i<deepLocations.size(); ++i)
        {
            // Do a '.' only delimiter encoding. \xfe is a tmp char (does not leak)
            std::string encodedpath = pystring::replace(
                FnAttribute::DelimiterEncode(
                    pystring::replace(deepLocations[i], "/", "\xfe")), "\xfe", "/");
            gb.set(std::string("c.") + pystring::replace(encodedpath, "/", ".c.") + ".a",
                interface.getAttr("", deepLocations[i]));
        }

        interface.deleteChildren();
        interface.setAttr("staticScene", gb.build());
    }
};

DEFINE_GEOLIBOP_PLUGIN(HierarchyCollapseOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(StaticSceneCreateOp, "StaticSceneCreate", 0, 1);
    REGISTER_PLUGIN(HierarchyCollapseOp, "HierarchyCollapse", 0, 1);
}

