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
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"
#include "kt.h"


namespace { //anonymous

const std::string DEFAULT_RIGEDITATTR = "";


class JsonCache
{
public:
    JsonCache(int num)
    {
        code = num;
    }
private:
    int code;
    std::string path;
    std::vector<kt::shapeid> map;
public:
    int getValue()
    {
        return code;
    }
    void next()
    {
        code++;
    }
    std::string getPath()
    {
        return path;
    }
    void setPath(std::string json_path)
    {
        path = json_path;
        if(json_path.compare("*.json") == 0)
        {
            std::cout << "[WARNING] Please set ShapeID.regedit json file :" << std::endl;
            return;   
        }
        
        std::string output_path = json_path;
        FILE* fp = fopen(kt::stoChar(output_path), "r"); // non-Windows use "r"
        if(fp == NULL)// check if we had load json file,katana will go crash without this!
        {
            std::cout << "[ERROR] Failed to open json file :" << output_path << std::endl;
            return;
        }
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document document;
        document.ParseStream(inputStream);
        fclose(fp);
        assert(document.IsObject());
        for (rapidjson::Value::ConstMemberIterator item = document.MemberBegin(); item != document.MemberEnd(); ++item)
        {
            std::string key_name = item->name.GetString();
            int key_value = item->value.GetInt();
            kt::shapeid temp = kt::shapeid(key_name,key_value);
            map.push_back(temp);
        }            
    }
    int find(std::string location)
    {
        for(unsigned int i = 0;i < map.size();i++)
        {
            if(map[i].shape.compare(location) == 0)
                return map[i].id;
        }
        return 0;
    }
    void print()
    {
        std::cout << "---------------" << std::endl;
        for(unsigned int i = 0;i < map.size();i++)
        {
            std::cout << map[i].shape << ":" << map[i].id << std::endl;
        }
    }
    void clear()
    {
        map.clear();
    }
};

JsonCache g_cache(0);

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

        if(interface.atRoot())
        {
            FnAttribute::StringAttribute regeditAttr = interface.getOpArg("regedit");
            g_cache.setPath(regeditAttr.getValue("",false));
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


        FnAttribute::StringAttribute regeditAttr =
            interface.getOpArg("regedit");

        const std::string regedit =
            regeditAttr.getValue(DEFAULT_RIGEDITATTR, false);
        std::string currentLocation = interface.getInputLocationPath();

        //int id = g_cache.find(currentLocation);
        int id = 1234567890123;
        //std::cout << "\n\n" << "current: " << currentLocation << "--" << id << std::endl; 
        //g_cache.print();
        interface.setAttr("geometry.arbitrary.shape_id.scope", FnAttribute::StringAttribute("primitive"),false);
        interface.setAttr("geometry.arbitrary.shape_id.value", FnAttribute::IntAttribute(id),false);            


        //int index = interface.getInputIndex();
        //std::string name = interface.getInputName();
        //std::cout << index << " : " << name << regedit << std::endl;


    }

    static void flush()
    {
        g_cache.clear();
    }

};


DEFINE_GEOLIBOP_PLUGIN(ShapeIDOp)

} // anonymous

void registerPlugins()
{
    std::cout << "\n+++++\nRegister ShapeID beta v0.1\n+++++\n" << std::endl;
    REGISTER_PLUGIN(ShapeIDOp, "ShapeID", 0, 1);
}
