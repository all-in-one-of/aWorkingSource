#pragma once
#include <vector>
#include <string>

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

}//ending namespace kt