#pragma once
#include <vector>
#include <string>
#include <math.h>

#include "assert.h"

#define REGISTER_AOVS \
data->aovs.clear(); \
data->aovs.push_back(params[p_aov_diffuse_color].STR); \
data->aovs.push_back(params[p_aov_ao_color].STR); \
assert(data->aovs.size() == 2 && "NUM_AOVs does not match size of aovs array!"); \
for (size_t i=0; i < data->aovs.size(); ++i) \
	AiAOVRegister(data->aovs[i].c_str(), AI_TYPE_RGB, AI_AOV_BLEND_OPACITY); \

enum AovIndices
{
	k_diffuse_color = 0,
	k_ao_color,
};

struct ShaderData
{
	std::vector<std::string> aovs;
};


namespace kt
{

template<typename genCType>

genCType invertColor(genCType color)
{
	genCType result;
	result.r = 1.0f - color.r;
	result.g = 1.0f - color.g;
	result.b = 1.0f - color.b;
	return result;
}

template<typename genDType>

inline genDType log(genDType c)
{
    c.r = logf(c.r);
    c.g = logf(c.g);
    c.b = logf(c.b);
    return c;
}

inline AtRGB max(const AtRGB& c1, const AtRGB& c2)
{
    AtRGB c;
    c.r = std::max(c1.r, c2.r);
    c.g = std::max(c1.g, c2.g);
    c.b = std::max(c1.b, c2.b);
    return c;
}

inline AtRGB min(const AtRGB& c1, const AtRGB& c2)
{
    AtRGB c;
    c.r = std::min(c1.r, c2.r);
    c.g = std::min(c1.g, c2.g);
    c.b = std::min(c1.b, c2.b);
    return c;
}

inline AtVector max(const AtVector& c1, const AtVector& c2)
{
    AtVector c;
    c.x = std::max(c1.x, c2.x);
    c.y = std::max(c1.y, c2.y);
    c.z = std::max(c1.z, c2.z);
    return c;
}

inline AtVector min(const AtVector& c1, const AtVector& c2)
{
    AtVector c;
    c.x = std::min(c1.x, c2.x);
    c.y = std::min(c1.y, c2.y);
    c.z = std::min(c1.z, c2.z);
    return c;
}

inline int clamp(int a, int mn, int mx)
{
    return std::min(std::max(a, mn), mx);
}

inline float clamp(float a, float mn, float mx)
{
    return std::min(std::max(a, mn), mx);
}

inline AtRGB clamp(const AtRGB& a, const AtRGB& mn, const AtRGB& mx)
{
    return kt::min(kt::max(a, mn), mx);
}

inline AtVector clamp(const AtVector& a, const AtVector& mn, const AtVector& mx)
{
    return kt::min(kt::max(a, mn), mx);
}

inline float maxh(const AtRGB& c)
{
   return std::max(std::max(c.r, c.g), c.b);
}

inline float minh(const AtRGB& c)
{
   return std::min(std::min(c.r, c.g ), c.b);
}


}//ending namespace kt