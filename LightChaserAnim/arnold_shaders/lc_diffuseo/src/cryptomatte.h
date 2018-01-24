#pragma once
#include <vector>
#include <string>
#include <math.h>
#include <cassert>

#define REGISTER_AOVS_CUSTOM \
data->aovs_custom.clear(); \
data->aovs_custom.push_back(params[p_aov_diffuse_color].STR); \
data->aovs_custom.push_back(params[p_aov_ao_color].STR); \
assert(data->aovs_custom.size() == 2 && "NUM_AOVs does not match size of aovs array!"); \
for (size_t i=0; i < data->aovs_custom.size(); ++i) \
   AiAOVRegister(data->aovs_custom[i].c_str(), AI_TYPE_RGB, AI_AOV_BLEND_OPACITY); \

enum AovIndices
{
   k_diffuse_color = 0,
   k_ao_color,
};

struct ShaderData
{
    // AOV names
    std::vector<std::string> aovs;
    std::vector<std::string> aovs_rgba;
    std::vector<std::string> aovs_custom;
};