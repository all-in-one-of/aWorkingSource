#pragma once

#include <ai.h>
#include <Remap.h>
#include <cstring>

enum TriplanarSpaceEnum 
{
    NS_WORLD = 0,
    NS_OBJECT,
    NS_PREF,
};

static const char* triplanarSpaceNames[] 
    = {"World", "Object", "Pref", NULL};

enum TriplanarNormalEnum 
{
    N_GEOMETRIC = 0,
    N_SMOOTH,
    N_SMOOTHNOBUMP,
};

static const char* triplanarNormalNames[] 
    = {"Geometric", "Smooth","SmoothNoBump", NULL};


struct SGCache 
{
    void initCache(const AtShaderGlobals* sg) 
    {
        u = sg->u;
        v = sg->v;
        dudx = sg->dudx;
        dudy = sg->dudy;
        dvdx = sg->dvdx;
        dvdy = sg->dvdy;
    }

    void restoreSG(AtShaderGlobals* sg) 
    {
        sg->u = u;
        sg->v = v;
        sg->dudx = dudx;
        sg->dudy = dudy;
        sg->dvdx = dvdx;
        sg->dvdy = dvdy;
    }

    float u;
    float v;
    float dudx;
    float dudy;
    float dvdx;
    float dvdy;
};

inline void getProjectionGeometry(const AtNode* node, const AtShaderGlobals* sg, int space, int normal, AtVector* P, AtVector* N, AtVector* dPdx, AtVector* dPdy) 
{
    AtVector baseN;
    switch (normal) 
    {
    case N_GEOMETRIC:
        baseN = sg->Ng;
        break;
    case N_SMOOTH:
        baseN = sg->N;
        break;
    case N_SMOOTHNOBUMP:
        baseN = sg->Ns;
        break;
    default:
        baseN = sg->N;
        break;
    }

    switch (space) 
    {
    case NS_WORLD:
        *P = sg->P;
        *N = baseN;
        *dPdx = sg->dPdx;
        *dPdy = sg->dPdy;
        break;
    case NS_OBJECT:
        *P = sg->Po;
        *N = AiShaderGlobalsTransformNormal(sg, baseN, AI_WORLD_TO_OBJECT);
        *dPdx =
            AiShaderGlobalsTransformVector(sg, sg->dPdx, AI_WORLD_TO_OBJECT);
        *dPdy =
            AiShaderGlobalsTransformVector(sg, sg->dPdy, AI_WORLD_TO_OBJECT);
        break;
    case NS_PREF:
        static AtString str_Pref("Pref");
        if (!AiUDataGetVec(str_Pref, *P)) 
        {
            AiMsgWarning("[lc_triplanar_plus] could not get Pref");
            // TODO: Output warning about not finding the correct data.
            *P = sg->Po;
            *N = AiShaderGlobalsTransformNormal(sg, baseN, AI_WORLD_TO_OBJECT);
            *dPdx = AiShaderGlobalsTransformVector(sg, sg->dPdx,
                                                   AI_WORLD_TO_OBJECT);
            *dPdy = AiShaderGlobalsTransformVector(sg, sg->dPdy,
                                                   AI_WORLD_TO_OBJECT);
        } 
        else 
        {
            AiMsgWarning("[lc_triplanar_plus got Pref]");
            AiUDataGetDxyDerivativesVec(str_Pref, *dPdx, *dPdy);
            *N = AiV3Normalize(AiV3Cross(*dPdx, *dPdy));
        }
        break;
    default:
        *P = sg->P;
        *N = baseN;
        *dPdx = sg->dPdx;
        *dPdy = sg->dPdy;
        break;
    }
}

inline void computeBlendWeights(const AtVector N, int space, float blendSoftness, float* weights) 
{
    weights[0] = fabsf(N.x);
    weights[1] = fabsf(N.y);
    weights[2] = fabsf(N.z);
    float weightsum = 0.f;
    for (int i = 0; i < 3; ++i) 
    {
        weights[i] = weights[i] - (1.f - blendSoftness) / 2.f;
        weights[i] = std::max(weights[i], 0.00f);
        weightsum += weights[i];
    }
    if (weightsum) 
    {
        for (int i = 0; i < 3; ++i)
            weights[i] /= weightsum;
    }
}

inline void rotateUVs(AtVector& P, float degrees) 
{
    AtVector orientVectorX;
    const double d2r = 1. / 360. * AI_PI * 2;
    double phi = d2r * degrees;
    orientVectorX.x = cosf(phi);
    orientVectorX.y = sinf(phi);
    orientVectorX.z = 0.f;

    AtVector orientVectorZ;
    orientVectorZ.x = 0.f;
    orientVectorZ.y = 0.f;
    orientVectorZ.z = 1.f;

    AtVector orientVectorY = AiV3Cross(orientVectorX, orientVectorZ);

    AiV3RotateToFrame(P, orientVectorX, orientVectorY, orientVectorZ);
}