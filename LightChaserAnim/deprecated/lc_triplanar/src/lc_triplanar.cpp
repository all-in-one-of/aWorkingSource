#include "Remap.h"
#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(LCUVTriplanarMtd)

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


enum Params 
{
    p_input,
    p_space,
    p_normal,
    p_frequency,
    p_offset,
    p_scale,
    p_rotate,
    p_rotjitter,
    p_blend_softness,
};

node_parameters 
{
    AiParameterRGBA("input", 1.0f, 1.0f, 1.0f, 1.0f);
    AiParameterEnum("space", 0, triplanarSpaceNames);
    AiParameterEnum("normal", 0, triplanarNormalNames);
    AiParameterFlt("frequency", 1.0f);
    AiParameterVec("offset",0.0f,0.0f,0.0f);
    AiParameterVec("scale",1.0f,1.0f,1.0f);
    AiParameterVec("rotate",0.0f,0.0f,0.0f);
    AiParameterVec("rotjitter",1.0f,1.0f,1.0f);
    AiParameterFlt("blend_softness", 0.1);
}


node_initialize 
{
}

node_finish 
{
}

node_update 
{
}


shader_evaluate 
{
    // get shader parameters

    int space = AiShaderEvalParamInt(p_space);

    int normal = AiShaderEvalParamInt(p_normal);

    float frequency = AiShaderEvalParamFlt(p_frequency);

    float blend_softness = clamp(AiShaderEvalParamFlt(p_blend_softness), 0.f, 1.f);

    AtVector scale = AiShaderEvalParamVec(p_scale);

    AtVector rotate = AiShaderEvalParamVec(p_rotate);

    AtVector offset = AiShaderEvalParamVec(p_offset);

    AtVector rotjitter = AiShaderEvalParamVec(p_rotjitter);

    SGCache SGC;
    SGC.initCache(sg);

    // set up P and blend weights
    AtVector P;
    AtVector N;
    AtVector dPdx;
    AtVector dPdy;


    getProjectionGeometry(node, sg, space, normal, &P, &N, &dPdx, &dPdy);
    float weights[3];
    computeBlendWeights(N, space, blend_softness, weights);
    int blends[3];

    P *= frequency;
    // compute texture values

    AtRGBA colorResult[3];

    // lookup X
    AtVector projP;
    
    projP.x = (P.z + 123.94 + offset.x) * scale.x;
    projP.y = (P.y + 87.22 + offset.x) * scale.x;
    projP.z = 0.;
    rotateUVs(projP, rotate.x);

    sg->u = projP.x;
    sg->v = projP.y;
    sg->dudx = dPdx.z * scale.x;
    sg->dudy = dPdy.z * scale.x;
    sg->dvdx = dPdx.y * scale.x;
    sg->dvdy = dPdy.y * scale.x;

    // lookup X
    AtRGBA result;
    if (weights[0] > 0.) 
    {
        //sg->out.RGB() = AiShaderEvalParamRGBA(p_input).rgb();
        colorResult[0] = AiShaderEvalParamRGBA(p_input);
        blends[0] = 1;
    }
    else
    {
        colorResult[0] = AI_RGBA_ZERO;
        blends[0] = 0;
    }
    // lookup Y
    projP.x = (P.x + 74.1 + offset.y) * scale.y;
    projP.y = (P.z + 9.2 + offset.y) * scale.y;
    projP.z = 0.;
    rotateUVs(projP, rotate.y);

    sg->u = projP.x;
    sg->v = projP.y;
    sg->dudx = dPdx.x * scale.y;
    sg->dudy = dPdy.x * scale.y;
    sg->dvdx = dPdx.z * scale.y;
    sg->dvdy = dPdy.z * scale.y;

    if (weights[1] > 0.) 
    {
        colorResult[1] = AiShaderEvalParamRGBA(p_input);
        blends[1] = 1;
    } 
    else 
    {
        colorResult[1] = AI_RGBA_ZERO;
        blends[1] = 0;
    }

    // lookup Z
    projP.x = (P.x + 123.94 + offset.z) * scale.z;
    projP.y = (P.y + 87.22 + offset.z) * scale.z;
    projP.z = 0.;
    rotateUVs(projP, rotate.z);

    sg->u = projP.x;
    sg->v = projP.y;
    sg->dudx = dPdx.x * scale.z;
    sg->dudy = dPdy.x * scale.z;
    sg->dvdx = dPdx.y * scale.z;
    sg->dvdy = dPdy.y * scale.z;

    if (weights[2] > 0.) 
    {
        colorResult[2] = AiShaderEvalParamRGBA(p_input);
        blends[2] = 1;
    } 
    else 
    {
        colorResult[2] = AI_RGBA_ZERO;
        blends[2] = 0;
    }

    colorResult[0] = AI_RGBA_RED;
    colorResult[1] = AI_RGBA_GREEN;
    colorResult[2] = AI_RGBA_BLUE;
    // sg->out.RGBA() = colorResult[0]*blends[0] + colorResult[1]*blends[1] + colorResult[2]*blends[2];
    //sg->out.RGBA() = lerp(lerp(lerp(colorResult[2],colorResult[0],weights[0]),colorResult[1],weights[1]),colorResult[2],weights[2]);
    // sg->out.RGBA() = lerp(lerp(lerp(colorResult[1]*weights[1],colorResult[0]*weights[0],weights[0]),colorResult[1]*weights[1],weights[1]),colorResult[2]*weights[2],weights[2]);
    // sg->out.RGBA() = colorResult[0]
    // colorResult[0] = AtRGBA(weights[0],weights[0],weights[0],1.0f);
    // colorResult[1] = AtRGBA(weights[1],weights[1],weights[1],1.0f);
    // colorResult[2] = AtRGBA(weights[2],weights[2],weights[2],1.0f);
    AtRGBA first_layer = lerp(colorResult[1]*weights[1],colorResult[0]*weights[0],weights[0]);
    AtRGBA second_layer = lerp(colorResult[2]*weights[2], first_layer,weights[1]);
    AtRGBA third_layer = lerp(colorResult[2]*weights[2], first_layer,weights[2]);
    // sg->out.RGBA() = lerp(lerp(lerp(colorResult[1],colorResult[0],weights[0]),colorResult[1],weights[1]),colorResult[2],weights[2]);
    // sg->out.RGBA() = lerp(colorResult[0]*weights[0],second_layer, weights[2]);
    sg->out.RGBA() = AtRGBA(sg->Ns.x,sg->Ns.y,sg->Ns.z,1.0f);
    SGC.restoreSG(sg);
}


node_loader 
{
    if (i > 0)
        return 0;
    node->methods = LCUVTriplanarMtd;
    node->output_type = AI_TYPE_RGBA;
    node->name = "lc_uv_triplanar";
    node->node_type = AI_NODE_SHADER;
    strcpy(node->version, AI_VERSION);
    return true;
}
