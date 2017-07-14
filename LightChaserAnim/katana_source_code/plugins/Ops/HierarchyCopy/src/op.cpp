#include <iostream>

#include <set>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <FnPluginSystem/FnPlugin.h>

#include <FnGeolib/op/FnGeolibOp.h>

#include <pystring/pystring.h>

namespace { //anonymous

class HierarchyCopyOp : public Foundry::Katana::GeolibOp
{
public:

    class ArgsUpdater
    {
    public:
        ArgsUpdater(Foundry::Katana::GeolibCookInterface &interface)
        : m_interface(interface)
        , m_argsUpdated(false)
        {}

        FnAttribute::GroupBuilder & get()
        {
            if (!m_argsUpdated)
            {
                m_gb.update(m_interface.getOpArg());
                m_argsUpdated = true;
            }
            return m_gb;
        }

        ~ArgsUpdater()
        {
            if (m_argsUpdated)
            {
                //std::cerr << "updating to " <<  m_gb.build().getXML();
                m_interface.replaceChildTraversalOp(m_interface.getOpType(),
                            m_gb.build());
            }
        }

    private:
        Foundry::Katana::GeolibCookInterface & m_interface;
        bool m_argsUpdated;
        FnAttribute::GroupBuilder m_gb;

    };

    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        //std::cerr << interface.getOutputLocationPath();
        //std::cerr " " << interface.getOpArg().getXML() << std::endl;

        //will update child arguments if necessary upon destruction
        ArgsUpdater argsUpdater(interface);

        
        bool canMatchChildren = false;
        

        FnAttribute::GroupAttribute entries = interface.getOpArg("entries");

        Foundry::Katana::CreateLocationInfo createLocationInfo;


        for (int64_t i = 0, e = entries.getNumberOfChildren(); i < e; ++i)
        {
            FnAttribute::GroupAttribute entry = entries.getChildByIndex(i);
            std::string entryName = "entries." + entries.getChildName(i);


            FnAttribute::StringAttribute src = entry.getChildByName("src");

            // The index of the node inputs to copy the hierarchy from
            FnAttribute::IntAttribute srcIndexAttr = entry.getChildByName("srcIndex");

            bool entryCanMatchChildren = false;


            if (!src.isValid())
            {
                argsUpdater.get().del(entryName);
                continue;
            }

            FnAttribute::StringAttribute dst = entry.getChildByName("dst");
            if (dst.isValid())
            {
                std::set<size_t> omitIndices;

                FnAttribute::StringConstVector dstValues =
                        dst.getNearestSample(0);

                size_t index = 0;
                for (FnAttribute::StringConstVector::const_iterator I =
                        dstValues.begin(), E = dstValues.end(); I != E;
                                ++I, ++index)
                {

                    // noticed that an empty string will keep building
                    // forever. Probably something that should be fixed
                    // in CreateLocation. For now, let's validate here.
                    if (!pystring::startswith((*I),
                            interface.getRootLocationPath()))
                    {
                        omitIndices.insert(index);
                        continue;
                    }


                    Foundry::Katana::CreateLocation(
                            createLocationInfo, interface, (*I));

                    if (createLocationInfo.atLeaf)
                    {
                        const int srcIndex = srcIndexAttr.getValue(kFnKatGeolibDefaultInput, false);

                        interface.replaceAttrs(src.getValue("", false), srcIndex);
                        interface.replaceChildren(src.getValue("", false), srcIndex);
                        omitIndices.insert(index);
                    }
                    else if (createLocationInfo.canMatchChildren)
                    {
                        entryCanMatchChildren = true;
                    }
                    else
                    {
                        omitIndices.insert(index);
                    }
                }
                

                if (entryCanMatchChildren)
                {
                    if (!omitIndices.empty())
                    {
                        if (omitIndices.size() < dstValues.size())
                        {
                            std::vector<std::string> newValues;
                            newValues.reserve(
                                    dstValues.size() - omitIndices.size());

                            for (FnAttribute::StringConstVector::const_iterator
                                    I = dstValues.begin(), E = dstValues.end();
                                            I != E; ++I, ++index)
                            {

                                if (omitIndices.find(index) ==
                                        omitIndices.end())
                                {
                                    newValues.push_back((*I));
                                }
                            }

                            argsUpdater.get().set(entryName + ".dst",
                                    FnAttribute::StringAttribute(newValues));
                        }
                        else
                        {
                            //shouldn't be necessary as we won't get an
                            //entryCanMatchChildren if we've removed everything
                            //being safe never hurts.
                            argsUpdater.get().del(entryName);
                        }
                    }

                    canMatchChildren = true;
                }
                else
                {
                    argsUpdater.get().del(entryName);
                }
            }
        }

        if (!canMatchChildren)
        {
            //std::cerr << "stopping: "
            //std::cerr << interface.getOutputLocationPath() << std::endl;
            interface.stopChildTraversal();
        }
    }

};
DEFINE_GEOLIBOP_PLUGIN(HierarchyCopyOp)

} // anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(HierarchyCopyOp, "HierarchyCopy", 0, 1);
}

