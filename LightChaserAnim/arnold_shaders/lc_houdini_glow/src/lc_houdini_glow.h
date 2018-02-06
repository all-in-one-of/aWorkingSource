#include <ai.h>

namespace kt{


AtRGB emisson(AtShaderGlobals* sg,AtRGB color,float power)
{
        // caculate normal falloff,catually is N dot Eye
        AtRGB result;
        AtVector incident_ray = sg->Ro - sg->P;
        AtVector shading_normal = sg->N;
        float rim = 1 - AiClamp(AiV3Dot(AiV3Normalize(-incident_ray),shading_normal), 0.0f, 1.0f);
        result = color * powf(rim, power);
        return result;
}
    
}; // ending namespace kt
