#pragma once

#include <ai.h>

#include <iostream>
#include <cstdlib>

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>

#include "tinyxml/tinyxml2.h"
struct cell
{
  float m_id,m_data[10];

  cell(float id, float data[10]): m_id(id){
    for(unsigned i = 0; i < 10; i ++)
      m_data[i] = data[i];
  }


  float getID()
  {
    return m_id;
  }
  float* getData()
  {
    return m_data;
  }
};

struct ShaderData
{
    bool ktOverride;
    std::string ktConfigParh;
    AtString ktParameter;
    std::vector<cell> ktData;
};

namespace kt{

const std::string file_name = "test.";
const std::string file_format = ".json";

inline char* stoChar(std::string argument)
{
  std::string str = argument;
  char *cstr = new char[str.length() + 1];
  strcpy(cstr, str.c_str());
  // do stuff
  //delete[] cstr;
  return cstr;
}

template <typename T>
std::string NumberToString ( T Number )
{
   std::ostringstream ss;
   ss << Number;
   return ss.str();
}

inline bool isexistsfile(const std::string& name) 
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}


inline float lerp(const float a, const float b, const float t)
{
   return (1-t)*a + t*b;
}

inline AtRGB lerp(const AtRGB& a, const AtRGB& b, const float t)
{
   AtRGB r;
   r.r = lerp( a.r, b.r, t );
   r.g = lerp( a.g, b.g, t );
   r.b = lerp( a.b, b.b, t );
   return r;
}

inline AtVector lerp(const AtVector& a, const AtVector& b, const float t)
{
   AtVector r;
   r.x = lerp( a.x, b.x, t );
   r.y = lerp( a.y, b.y, t );
   r.z = lerp( a.z, b.z, t );
   return r;
}

inline AtRGBA lerp(const AtRGBA& a, const AtRGBA& b, const float t)
{
   AtRGBA r;
   r.r = lerp( a.r, b.r, t );
   r.g = lerp( a.g, b.g, t );
   r.b = lerp( a.b, b.b, t );
   r.a = lerp( a.a, b.a, t );
   return r;
}

}// ending namespace kt