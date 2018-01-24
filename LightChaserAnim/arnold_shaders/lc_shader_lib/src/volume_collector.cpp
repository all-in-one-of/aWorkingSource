#include "shaderlib.h"

#include <ai.h>


#if AI_VERSION_ARCH_NUM >= 4 && (AI_VERSION_MAJOR_NUM > 0 || AI_VERSION_MINOR_NUM >= 12)
   #define VOLUME_API_SUPPORTED
#endif

#ifdef VOLUME_API_SUPPORTED


AI_SHADER_NODE_EXPORT_METHODS(VolumeCollectorMethods);


enum VolumeCollectorParams
{
   p_scattering,
   p_absorption,
   p_emission
};

node_parameters
{
   AiParameterRGB("scattering", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("absorption", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("emission",   0.0f, 0.0f, 0.0f);
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


shader_evaluate
{
   AtColor scattering = AiShaderEvalParamRGB(p_scattering);
   AiColorClipToZero(scattering);
   AiShaderGlobalsSetVolumeScattering(sg, scattering);
   
   AtColor absorption = AiShaderEvalParamRGB(p_absorption);
   AiColorClipToZero(absorption);
   AiShaderGlobalsSetVolumeAbsorption(sg, absorption);
   
   AtColor emission = AiShaderEvalParamRGB(p_emission);
   AiColorClipToZero(emission);
   if (!AiColorIsZero(emission))
      AiShaderGlobalsSetVolumeEmission(sg, emission);
}


void registerVolumeCollector(AtNodeLib *node)
{
   node->methods     = VolumeCollectorMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = SHADERLIB_PREFIX "volume_collector";
   node->node_type   = AI_NODE_SHADER;
}


#else
   #pragma warning "Volume API not supported; disabling compilation of volume_collector node"
#endif // VOLUME_API_SUPPORTED

