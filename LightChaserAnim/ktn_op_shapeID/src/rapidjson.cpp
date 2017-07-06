#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include <cstdio>


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