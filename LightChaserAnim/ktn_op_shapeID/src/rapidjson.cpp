#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include <cstdio>


/*
std::vector<kt::shapeid> regeditMap;
if(interface.atRoot())
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

}
if(interface.atRoot())
{
    std::string output_path = "/home/xukai/Documents/TestProgramFactory/ktn_op_shapeID/out/output.json";
    FILE* fp = fopen(kt::stoChar(output_path), "r"); // non-Windows use "r"
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document document;
    document.ParseStream(inputStream);
    fclose(fp);


    //const rapidjson::Value& map = document;
    assert(document.IsObject());
    for (rapidjson::Value::ConstMemberIterator item = document.MemberBegin(); item != document.MemberEnd(); ++item)
    {
        std::string key_name = item->name.GetString();
        int key_value = item->value.GetInt();
        //std::cout << key_name << key_value << std::endl;
        kt::shapeid temp = kt::shapeid(key_name,key_value);
        regeditMap->add(temp);
    }

}// ending if atRoot()   
*/

int main()
{
	rapidjson::Document document;
	rapidjson::Value author;
	rapidjson::Document::AllocatorType& alloc = document.GetAllocator();
	document.SetObject();
	document.AddMember("FromEmail", 123, alloc);
	document.AddMember("FromName", 456, alloc);
	document.AddMember("Subject", 789, alloc);


	FILE* fp = fopen("/home/xukai/Documents/TestProgramFactory/ktn_op_shapeID/out/output.json", "w"); // non-Windows use "w"
	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
	document.Accept(writer);
	fclose(fp);
}
