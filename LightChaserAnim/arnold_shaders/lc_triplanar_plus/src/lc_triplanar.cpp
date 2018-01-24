#include "shader_utils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcTriplanarMtd)


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

    //sg->out.RGBA() = colorResult[0]*blends[0] + colorResult[1]*blends[1] + colorResult[2]*blends[2];
    //sg->out.RGBA() = lerp(lerp(lerp(colorResult[2],colorResult[0],weights[0]),colorResult[1],weights[1]),colorResult[2],weights[2]);
    sg->out.RGBA() = lerp(lerp(lerp(colorResult[1]*weights[1],colorResult[0]*weights[0],weights[0]),colorResult[1]*weights[1],weights[1]),colorResult[2]*weights[2],weights[2]);
    SGC.restoreSG(sg);
}

/*
node_loader 
{
    if (i > 0)
        return 0;
    node->methods = LcTriplanarMtd;
    node->output_type = AI_TYPE_RGBA;
    node->name = "lc_triplanar";
    node->node_type = AI_NODE_SHADER;
    strcpy(node->version, AI_VERSION);
    return true;
}
*/