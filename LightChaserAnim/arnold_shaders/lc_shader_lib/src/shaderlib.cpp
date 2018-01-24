#include "shaderlib.h"

#include <ai.h>
#include <cstring>

#if AI_VERSION_ARCH_NUM >= 4 && (AI_VERSION_MAJOR_NUM > 0 || AI_VERSION_MINOR_NUM >= 12)
   #define VOLUME_API_SUPPORTED
#endif


typedef void (*NodeRegisterFunc)(AtNodeLib *node);

void registerAbs(AtNodeLib *node);
void registerAdd(AtNodeLib *node);
void registerBlackbody(AtNodeLib *node);
void registerCache(AtNodeLib *node);
void registerClamp(AtNodeLib *node);
void registerColorConvert(AtNodeLib *node);
void registerColorCorrect(AtNodeLib *node);
void registerComplement(AtNodeLib *node);
void registerCross(AtNodeLib *node);
void registerDivide(AtNodeLib *node);
void registerDot(AtNodeLib *node);
void registerExp(AtNodeLib *node);
void registerLength(AtNodeLib *node);
void registerMax(AtNodeLib *node);
void registerMin(AtNodeLib *node);
void registerMix(AtNodeLib *node);
void registerMultiply(AtNodeLib *node);
void registerNormalize(AtNodeLib *node);
void registerPow(AtNodeLib *node);
void registerRandom(AtNodeLib *node);
void registerRange(AtNodeLib *node);
void registerReciprocal(AtNodeLib *node);
void registerRGBToFloat(AtNodeLib *node);
void registerRGBToVector(AtNodeLib *node);
void registerSubtract(AtNodeLib *node);
void registerUserDataFloat(AtNodeLib *node);
void registerUserDataInt(AtNodeLib *node);
void registerUserDataRgb(AtNodeLib *node);
void registerUserDataRgba(AtNodeLib *node);
void registerUserDataString(AtNodeLib *node);
void registerVectorToRGB(AtNodeLib *node);
#ifdef VOLUME_API_SUPPORTED
void registerVolumeCollector(AtNodeLib *node);
#endif

static NodeRegisterFunc registry[] =
{
   &registerAbs,
   &registerAdd,
   &registerBlackbody,
   &registerCache,
   &registerClamp,
   &registerColorConvert,
   &registerColorCorrect,
   &registerComplement,
   &registerCross,
   &registerDivide,
   &registerDot,
   &registerExp,
   &registerLength,
   &registerMax,
   &registerMin,
   &registerMix,
   &registerMultiply,
   &registerNormalize,
   &registerPow,
   &registerRandom,
   &registerRange,
   &registerReciprocal,
   &registerRGBToFloat,
   &registerRGBToVector,
   &registerSubtract,
   &registerUserDataFloat,
   &registerUserDataInt,
   &registerUserDataRgb,
   &registerUserDataRgba,
   &registerUserDataString,
   &registerVectorToRGB
#ifdef VOLUME_API_SUPPORTED
   , &registerVolumeCollector
#endif
};

static const int num_nodes = sizeof(registry) / sizeof(NodeRegisterFunc);


node_loader
{
   if (i >= num_nodes) return false;

   strcpy(node->version, AI_VERSION);
   (*registry[i])(node);

   return true;
}

