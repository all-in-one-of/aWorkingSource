#pragma once

//#include <algorithm>
#include <string>

namespace kt{

struct shapeid
{
	std::string shape;
	int id;
	shapeid() : shape(""),id(0){ }
	shapeid(std::string shape_in,int id_in) : shape(shape_in),id(id_in){ }
	int value()
	{
		return id;
	};
};

std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;
	for (unsigned int i = 0; i<str.size(); i++)
	{
		pos = str.find(pattern, i);
		if (pos<str.size())
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

std::string replace(std::string str, const std::string& from, const std::string& to) 
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

char* stoChar(std::string argument)
{
	std::string str = argument;
	char *cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());
	// do stuff
	//delete[] cstr;
	return cstr;
}



} //ending namaspace kt