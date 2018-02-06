#pragma once
 
 #include <ai.h>
#include <ai_shader_bsdf.h>
#include <ai_shaderglobals.h>
 
 namespace  kt {

    AtBSDF* DiffuseBSDFCreate(const AtShaderGlobals* sg, const AtRGB& weight, const AtVector& N);

 } // ending name space