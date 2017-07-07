#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <string>
#include <vector>
#include <cstdio>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolib/util/Path.h>
#include <FnPluginSystem/FnPlugin.h>

#include <pystring/pystring.h>

#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "kt.h"


namespace { //anonymous

const std::string DEFAULT_RIGEDITATTR = "";


class ShapeIDOp : public Foundry::Katana::GeolibOp
{
public:
    static void setup(Foundry::Katana::GeolibSetupInterface &interface)
    {
        interface.setThreading(
            Foundry::Katana::GeolibSetupInterface::ThreadModeConcurrent);
    }

    static void cook(Foundry::Katana::GeolibCookInterface &interface)
    {
        // In order to run the Op we need a valid CEL statement
        FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
        if (!celAttr.isValid())
        {
            interface.stopChildTraversal();
            return;
        }
        std::vector<kt::shapeid> regeditMap;
/*        if(interface.atRoot())
        {          
            std::string celInfo = celAttr.getValue();
            celInfo = kt::replace(celInfo,"(","");
            celInfo = kt::replace(celInfo,")","");
            celInfo = kt::replace(celInfo,"+","");
            std::vector<std::string> temp_split = kt::split(celInfo," ");
            int id = 0;
            for(unsigned int j=0; j<temp_split.size(); ++j)
            {
                if(temp_split[j].find_first_not_of(" ") != std::string::npos)
                {
                    kt::shapeid temp = kt::shapeid(temp_split[j],id);
                    regeditMap.push_back(temp);
                    //std::cout << temp_split[j] << std::endl;
                }
                id++;
            }

            rapidjson::Document document;
            rapidjson::Value author;
            rapidjson::Document::AllocatorType& alloc = document.GetAllocator();
            document.SetObject();
            for(unsigned int i=0;i<regeditMap.size();++i)
            {
                rapidjson::Value tempStr(regeditMap[i].shape.c_str(), alloc);
                document.AddMember(tempStr, regeditMap[i].id, alloc);
            }
            std::string output_path = "/home/xukai/Documents/TestProgramFactory/ktn_op_shapeID/out/output.json";
            FILE* fp = fopen(kt::stoChar(output_path), "w"); // non-Windows use "w"
            char writeBuffer[65536];
            rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
            document.Accept(writer);
            fclose(fp);

        }//ending atRoot*/
        if(interface.atRoot())
        {
            std::cout << celAttr.getValue() << std::endl;
            interface.createChild("shapeid","");
        }    

        // If a CEL attribute was provided (and so it's valid), check
        // our current output location against the CEL. If it doesn't match
        // the CEL, then don't continue cooking at this location.
        // Use the utility function matchesCEL() to populate a
        // MatchesCELInfo object that contains useful attributes we can
        // query
        FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo info;
        FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(info,interface,celAttr);

        // If there's no chance that the CEL expression matches any child
        // locations, stop the Op from continuing its recursion past this
        // point in the hierarchy. This isn't required, but improves
        // efficiency.
        if (!info.canMatchChildren)
        {
            interface.stopChildTraversal();
        }

        // If the CEL doesn't match the current location, stop cooking
        if (!info.matches)
        {
            return;
        }
        std::cout << "Current Location:" << interface.getInputLocationPath() << std::endl;

        FnAttribute::StringAttribute regeditAttr =
            interface.getOpArg("regedit");

        const std::string regedit =
            regeditAttr.getValue(DEFAULT_RIGEDITATTR, false);

        interface.setAttr("LCA.name", FnAttribute::StringAttribute("Light Chaser Animation Studio"),false);
        //int index = interface.getInputIndex();
        //std::string name = interface.getInputName();
        //std::cout << index << " : " << name << regedit << std::endl;


    }

protected:

};

DEFINE_GEOLIBOP_PLUGIN(ShapeIDOp)

} // anonymous

void registerPlugins()
{
    std::cout << "\n+++++\nRegister ShapeID beta v0.1\n+++++\n" << std::endl;
    REGISTER_PLUGIN(ShapeIDOp, "ShapeID", 0, 1);
}
