//The shader source code:
/*
 * nonlinear noise displacement shader
 */
 
#include <ai.h>
#include <string.h>
 
#define _octaves     (params[0].INT)
#define _freq        (params[1].FLT)
#define _amplitude   (params[2].FLT)
#define _bloom       (params[3].FLT)
#define _type        (params[4].INT)
 
AI_SHADER_NODE_EXPORT_METHODS(NonlinNzMethods);
 
#define ENUM_SCALAR_TYPES { "perlin", "abs_perlin", "recursive", "abs_recursive", NULL };
#define PERLIN        0
#define ABS_PERLIN    1
#define RECURSIVE     2
#define ABS_RECURSIVE 3
const char *types_enum[] = ENUM_SCALAR_TYPES;
 
node_parameters
{
   AiParameterINT ("octaves"  , 3);
   AiParameterFLT ("freq"     , 1);
   AiParameterFLT ("amplitude", 1);
   AiParameterFLT ("bloom"    , 1);  
   AiParameterENUM("type"     , PERLIN, types_enum);
}
 
float scalarfunc(AtPoint P, int type, int octaves)
{
   float doubler = 1;
   float NzAccum = 0;
   switch (type)
   {
      case PERLIN:
         return AiPerlin3(P);
      case ABS_PERLIN:
         return fabs(AiPerlin3(P));
      case RECURSIVE:
         for (int i = 0; i < octaves; i++) {
            NzAccum += AiPerlin3(P*doubler) / doubler;
            doubler *= 2;
         }
         return NzAccum;
      case ABS_RECURSIVE:
         for (int i = 0; i < octaves; i++) {
            NzAccum += fabs(AiPerlin3(P*doubler)) / doubler;
            doubler *= 2;
         }
         return NzAccum;
   }
   return AiPerlin3(P);
}
 
shader_evaluate
{
   const AtParamValue *params = AiNodeGetParams(node);
 
   AtPoint Ploc, Uloc, Vloc; // noise sample location, and over in U and V locations
   float Np, Nu, Nv;         // noise at P, noise at location over in U and V
   float Udelt, Vdelt;       // delta in the noise over in U and V
   float delta = .01;        // distance delta for noise samples
 
   AtVector U, V;
   if (!AiV3IsZero(sg->dPdu) && !AiV3IsZero(sg->dPdv))
   {
      // tangents available, use them
      U = sg->dPdu;
      V = sg->dPdv;
   }
   else
   {
      // no tangents given, compute a pair
      AiBuildLocalFramePolar(&U, &V, &sg->N);
   }
   if (_type > ABS_PERLIN)
   {
      // adjust delta to highest frequency in recursive noise
      delta *= pow(.5,  _octaves) * 2;
   }
   Ploc = sg->Po * _freq;
   Uloc = Ploc + U * delta;
   Vloc = Ploc + V * delta;
 
   // noise sampled at P, and over in U and V
   Np = scalarfunc(Ploc, _type, _octaves);
   Nu = scalarfunc(Uloc, _type, _octaves);
   Nv = scalarfunc(Vloc, _type, _octaves);
   Udelt = (Nu - Np) * _bloom;
   Vdelt = (Nv - Np) * _bloom;
   AtPoint Pstepped = sg->P;  
   int steps = 10;
   float stepscale = _amplitude / steps;
   for (int i = 0; i < steps; i++)
   {
      // stepdir is the cross product of the derivatives
      AtVector stepdir = AiV3Cross(U,V);
      // deflect the derivatives
      U = AiV3Normalize(U + (stepdir * Udelt * stepscale));
      V = AiV3Normalize(V + (stepdir * Vdelt * stepscale));
      Pstepped += stepdir * Np * stepscale;
   }
   sg->out.VEC = Pstepped - sg->P;
}
 
node_initialize
{
}
 
node_update
{
}
 
node_finish
{
}
 
node_loader
{
   if (i > 0) return false;
    
   node->methods      = NonlinNzMethods;
   node->output_type  = AI_TYPE_VECTOR;
   node->name         = "nonlinear_noise";
   node->node_type    = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}