// Blackbody shader taken from Houdini's Pyro 2 vex shader

#include "shaderlib.h"

#include <ai.h>

#include <cmath>


AI_SHADER_NODE_EXPORT_METHODS(BlackbodyMethods);


namespace
{

#define PLANCK_C  2.99792458e+08  // Speed of light (m/s)
#define PLANCK_H  6.62606896e-34  // Planck's constant (J/s)
#define PLANCK_K  1.38064880e-23  // Boltzmann's constant (J/K)
#define PLANCK_S  5.67037321e-08  // Stefan-Boltzmann constant (W/m^2)
#define PLANCK_c1 3.74177e-16     // 2 * PI * PLANCK_H * PLANCK_C^2
#define PLANCK_c2 0.0143878       // (PLANCK_H * PLANCK_C) / PLANCK_K
#define PLANCK_a5 4.965114231744  // Solution to x = 5 * (1 - e^-x)

inline float fit(float t, float in_min, float in_max, float out_min, float out_max)
{
   return (t - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
}

// Calculate a very close approxumation to the chromaticity (hue) of a
// blackbody emitter at the given temperature 'T' (in Kelvin).
// This version is an approximation using 3 cubic splines. Not the
// exact result, but many times faster to compute, so it's the default.
// The result is in linear color space(!!) and is modified to have equal
// luminance across the entire output range.
AtRGB bbspline(float T)
{
   static const float Tfirst = 1667.0f;
   static const float Tlast = 25000.0f;

   AtRGB xyz = AI_RGB_BLACK;

   if (T < Tfirst)
   {
      xyz = AiColorLerp(fit(T, 0.0f, Tfirst, 0.0f, 1.0f),
                        AiColorCreate(0.73469f, 0.26531f, 3.92832e-21f),
                        AiColorCreate(0.564639f, 0.402887f, 0.0324743f));
   }
   else if (T > Tlast)
   {
      xyz = AiColorLerp(fit(T, Tlast, 1e6f, 0.0f, 1.0f),
                        AiColorCreate(0.252473f, 0.252255f, 0.495272f),
                        AiColorCreate(0.238815f, 0.239092f, 0.522093f));
   }
   else
   {
      float T2 = T * T;
      float T3 = T2 * T;
      float c1 = 1e9f / T3;
      float c2 = 1e6f / T2;
      float c3 = 1e3f / T;
      float x;

      if (T >= 1667.0f && T <= 4000.0f)
         x = -0.2661239f * c1 - 0.2343580f * c2 + 0.8776956f * c3 + 0.179910f;
      else if (T > 4000.f && T <= 25000.f)
         x = -3.0258469f * c1 + 2.1070379f * c2 + 0.2226347f * c3 + 0.240390f;

       float x2 = x * x;
      float x3 = x2 * x;
      float y;

      if (T >= 1667.0f && T <= 2222.0f)
         y = -1.1063814f * x3 - 1.34811020f * x2 + 2.18555832f * x - 0.20219683f;
      else if (T > 2222.0f && T <= 4000.0f)
         y = -0.9549476f * x3 - 1.37418593f * x2 + 2.09137015f * x - 0.16748867f;
      else if (T > 4000.0f && T <= 25000.0f)
         y = +3.0817580f * x3 - 5.87338670f * x2 + 3.75112997f * x - 0.37001483f;

      xyz.r = x;
      xyz.g = y;
      xyz.b = 1.0f - x - y;
   }

   return xyz;
}

// Plank's law modified to units of (W/m^2/sr/nm)
inline float planck(float T, float wl)
{
   double wl5 = wl * wl * wl * wl * wl;
   double den;

   if (T < 344.0f)
      den = 6e49; // FIXME float overflow
   else
      den = exp(1.4387768275291327e7 / (T * wl));

   return float(1.191042758493456e20 / (wl5 * (den - 1.0)));
}

// Returns the wavelength (in nm) with highest radiant power (W/nm^2) at the
// given temperature T -- the wavelength with the peak output.
// The actual function is (10^9*PLANCK_H*PLANCK_C) / (PLANCK_a5*PLANCK_K*T),
// but here all the constants were collapsed to a single factor.
inline float planckPeak(float T)
{
   return 2.89777e6f / T;
}

// The total spectral radiant power (W/m^2) for a surface (i.e: total
// hemispherical emittance incl. Lambert's cos term) at temperature T
// for lambda in (0,+inf].
inline float planckPower(float T)
{
   return PLANCK_S * T * T * T * T;
}

// The total spectral radiant power (W/m^2) for a band of temperatures [T1,T2]
inline float planckBandPower(float T1, float T2)
{
   return PLANCK_S * (T2 * T2 * T2 * T2 * T2 - T1 * T1 * T1 * T1 * T1) / 5.0f;
}

// Average spectral radiant power (W/m^2) for a band of temperatures [T1,T2]
inline float planckBandPowerAvg(float T1, float T2)
{
   return planckBandPower(T1, T2) / (T2 - T1);
}

// Total log spectral radiant power log(W/m^2) for a band of temperatures [T1,T2]
inline float planckBandLogPower(float T1, float T2)
{
   return 4.0f * T2 * logf(T2) - 4.0f * T1 * logf(T1) - (T1 - T2) * (logf(PLANCK_S) - 4.0f);
}

// Average log spectral radiant power log(W/m^2) for a band of temperatures [T1,T2]
inline float planckBandLogPowerAvg(float T1, float T2)
{
   return ((4.0f * T1 * logf(T1) - 4.0f * T2 * logf(T2)) / (T1 - T2)) - 4.0f + logf(PLANCK_S);
}

#define CMF_WLSTART  380.0f
#define CMF_WLEND    780.0f
#define CMF_WLSTEP   5.0f
#define CMF_NSTEPS   81.0f

AtRGB cmf(int ndx)
{
   static const AtRGB stdobserver_1964_10deg[] =
   {
      { 0.000159952000f, 0.000017364000f, 0.000704776000f }, // 380 nm
      { 0.000662440000f, 0.000071560000f, 0.002927800000f }, // 385 nm
      { 0.002361600000f, 0.000253400000f, 0.010482200000f }, // 390 nm
      { 0.007242300000f, 0.000768500000f, 0.032344000000f }, // 395 nm
      { 0.019109700000f, 0.002004400000f, 0.086010900000f }, // 400 nm
      { 0.043400000000f, 0.004509000000f, 0.197120000000f }, // 405 nm
      { 0.084736000000f, 0.008756000000f, 0.389366000000f }, // 410 nm
      { 0.140638000000f, 0.014456000000f, 0.656760000000f }, // 415 nm
      { 0.204492000000f, 0.021391000000f, 0.972542000000f }, // 420 nm
      { 0.264737000000f, 0.029497000000f, 1.282500000000f }, // 425 nm
      { 0.314679000000f, 0.038676000000f, 1.553480000000f }, // 430 nm
      { 0.357719000000f, 0.049602000000f, 1.798500000000f }, // 435 nm
      { 0.383734000000f, 0.062077000000f, 1.967280000000f }, // 440 nm
      { 0.386726000000f, 0.074704000000f, 2.027300000000f }, // 445 nm
      { 0.370702000000f, 0.089456000000f, 1.994800000000f }, // 450 nm
      { 0.342957000000f, 0.106256000000f, 1.900700000000f }, // 455 nm
      { 0.302273000000f, 0.128201000000f, 1.745370000000f }, // 460 nm
      { 0.254085000000f, 0.152761000000f, 1.554900000000f }, // 465 nm
      { 0.195618000000f, 0.185190000000f, 1.317560000000f }, // 470 nm
      { 0.132349000000f, 0.219940000000f, 1.030200000000f }, // 475 nm
      { 0.080507000000f, 0.253589000000f, 0.772125000000f }, // 480 nm
      { 0.041072000000f, 0.297665000000f, 0.570060000000f }, // 485 nm
      { 0.016172000000f, 0.339133000000f, 0.415254000000f }, // 490 nm
      { 0.005132000000f, 0.395379000000f, 0.302356000000f }, // 495 nm
      { 0.003816000000f, 0.460777000000f, 0.218502000000f }, // 500 nm
      { 0.015444000000f, 0.531360000000f, 0.159249000000f }, // 505 nm
      { 0.037465000000f, 0.606741000000f, 0.112044000000f }, // 510 nm
      { 0.071358000000f, 0.685660000000f, 0.082248000000f }, // 515 nm
      { 0.117749000000f, 0.761757000000f, 0.060709000000f }, // 520 nm
      { 0.172953000000f, 0.823330000000f, 0.043050000000f }, // 525 nm
      { 0.236491000000f, 0.875211000000f, 0.030451000000f }, // 530 nm
      { 0.304213000000f, 0.923810000000f, 0.020584000000f }, // 535 nm
      { 0.376772000000f, 0.961988000000f, 0.013676000000f }, // 540 nm
      { 0.451584000000f, 0.982200000000f, 0.007918000000f }, // 545 nm
      { 0.529826000000f, 0.991761000000f, 0.003988000000f }, // 550 nm
      { 0.616053000000f, 0.999110000000f, 0.001091000000f }, // 555 nm
      { 0.705224000000f, 0.997340000000f, 0.000000000000f }, // 560 nm
      { 0.793832000000f, 0.982380000000f, 0.000000000000f }, // 565 nm
      { 0.878655000000f, 0.955552000000f, 0.000000000000f }, // 570 nm
      { 0.951162000000f, 0.915175000000f, 0.000000000000f }, // 575 nm
      { 1.014160000000f, 0.868934000000f, 0.000000000000f }, // 580 nm
      { 1.074300000000f, 0.825623000000f, 0.000000000000f }, // 585 nm
      { 1.118520000000f, 0.777405000000f, 0.000000000000f }, // 590 nm
      { 1.134300000000f, 0.720353000000f, 0.000000000000f }, // 595 nm
      { 1.123990000000f, 0.658341000000f, 0.000000000000f }, // 600 nm
      { 1.089100000000f, 0.593878000000f, 0.000000000000f }, // 605 nm
      { 1.030480000000f, 0.527963000000f, 0.000000000000f }, // 610 nm
      { 0.950740000000f, 0.461834000000f, 0.000000000000f }, // 615 nm
      { 0.856297000000f, 0.398057000000f, 0.000000000000f }, // 620 nm
      { 0.754930000000f, 0.339554000000f, 0.000000000000f }, // 625 nm
      { 0.647467000000f, 0.283493000000f, 0.000000000000f }, // 630 nm
      { 0.535110000000f, 0.228254000000f, 0.000000000000f }, // 635 nm
      { 0.431567000000f, 0.179828000000f, 0.000000000000f }, // 640 nm
      { 0.343690000000f, 0.140211000000f, 0.000000000000f }, // 645 nm
      { 0.268329000000f, 0.107633000000f, 0.000000000000f }, // 650 nm
      { 0.204300000000f, 0.081187000000f, 0.000000000000f }, // 655 nm
      { 0.152568000000f, 0.060281000000f, 0.000000000000f }, // 660 nm
      { 0.112210000000f, 0.044096000000f, 0.000000000000f }, // 665 nm
      { 0.081260600000f, 0.031800400000f, 0.000000000000f }, // 670 nm
      { 0.057930000000f, 0.022601700000f, 0.000000000000f }, // 675 nm
      { 0.040850800000f, 0.015905100000f, 0.000000000000f }, // 680 nm
      { 0.028623000000f, 0.011130300000f, 0.000000000000f }, // 685 nm
      { 0.019941300000f, 0.007748800000f, 0.000000000000f }, // 690 nm
      { 0.013842000000f, 0.005375100000f, 0.000000000000f }, // 695 nm
      { 0.009576880000f, 0.003717740000f, 0.000000000000f }, // 700 nm
      { 0.006605200000f, 0.002564560000f, 0.000000000000f }, // 705 nm
      { 0.004552630000f, 0.001768470000f, 0.000000000000f }, // 710 nm
      { 0.003144700000f, 0.001222390000f, 0.000000000000f }, // 715 nm
      { 0.002174960000f, 0.000846190000f, 0.000000000000f }, // 720 nm
      { 0.001505700000f, 0.000586440000f, 0.000000000000f }, // 725 nm
      { 0.001044760000f, 0.000407410000f, 0.000000000000f }, // 730 nm
      { 0.000727450000f, 0.000284041000f, 0.000000000000f }, // 735 nm
      { 0.000508258000f, 0.000198730000f, 0.000000000000f }, // 740 nm
      { 0.000356380000f, 0.000139550000f, 0.000000000000f }, // 745 nm
      { 0.000250969000f, 0.000098428000f, 0.000000000000f }, // 750 nm
      { 0.000177730000f, 0.000069819000f, 0.000000000000f }, // 755 nm
      { 0.000126390000f, 0.000049737000f, 0.000000000000f }, // 760 nm
      { 0.000090151000f, 0.000035540500f, 0.000000000000f }, // 765 nm
      { 0.000064525800f, 0.000025486000f, 0.000000000000f }, // 770 nm
      { 0.000046339000f, 0.000018338400f, 0.000000000000f }, // 775 nm
      { 0.000033411700f, 0.000013249000f, 0.000000000000f }  // 780 nm
   };

   return stdobserver_1964_10deg[ndx];
}

AtRGB blackbodynm(float tempkelvin, float& norm)
{
   float bb;
   int i;
   float T = tempkelvin;

   norm = 1.0f / planck(T, CLAMP(planckPeak(T), CMF_WLSTART, CMF_WLEND));

   float lum = 0.0f;
   float n = 0.0f;
   AtRGB xyz = AI_RGB_BLACK;

   if (T < 345.0f)
   {
      xyz = AiColorLerp(fit(T, 0.0f, 345.0f, 0.0f, 1.0f),
                        //AiColorCreate(3.982e-76f, 1.43797e-76f, 2.12914e-96f), // FIXME float overflow
                        AI_RGB_BLACK,
                        AiColorCreate(3.63784e-21f, 1.32305e-21f, 1.2667e-26f));
   }
   else
   {
      float wl = CMF_WLSTART;
      AtRGB cie;
      for (i = 0; i < CMF_NSTEPS; ++i)
      {
         bb = planck(T, wl);
         cie = cmf(i);
         n += cie.g;
         cie *= bb;
         lum += cie.g;
         xyz += cie;
         wl += CMF_WLSTEP;
      }
   }

   return xyz;
}

#define PLANCK_LPA_D58  13.977f   // = planckbandlogpoweravg(0,5800)
#define PLANCK_LPA_D10  6.94559f  // = planckbandlogpoweravg(0,1000)
#define PLANCK_TMNORM   568.0f    // unit^-2

inline float luma(AtRGB color)
{
   // Haeberli
   return color.r * 0.3086f + color.g * 0.6094f + color.b * 0.0820f;
}

// XYZ -> Chromaticity[3] (xyz)
inline AtRGB xyzToChroma(AtRGB xyz)
{
   float d = xyz.r + xyz.g + xyz.g;

   if (d == 0.f)
      return AI_RGB_BLACK;
   else
      return xyz / d;
}

inline float log(float x, float base)
{
   return logf(MAX(1.0e-37f, x)) / logf(MAX(1.0e-37f, base));
}

AtRGB tonemap(AtRGB rgb, float avgloglum, float key, float burn)
{
   float rhk = key / avgloglum;
   float rhb = (burn != 0.0f) ? powf(2.0f, burn) : 1.0f;
   float Lp = luma(rgb) * rhk;
   return rgb * Lp * (1.0f + Lp * rhb) / (1.0f + Lp);
}

void blackbody(float T,         // Temperature (K)
               bool dotm,       // enable tone mapping
               float adapt,     // tome mapping adaptation
               float burn,      // tone mapping burn
               AtRGB& out_col,  // output: chromaticity
               float& out_val)  // output: power/intensity
{
   AtRGB xyz = bbspline(T);
   AtRGB chr = xyzToChroma(xyz);
   chr /= (AI_PI * luma(chr));
   AtRGB rgb = chr;

   if (dotm)
   {
      float k = MAX(1.0e-3f, adapt);
      rgb *= 580.0f * log(1.0f + planckPower(k * T), 1.0f + planckPower(k * 5800.0f));
      rgb = tonemap(rgb, PLANCK_LPA_D10, 0.18f, burn) / PLANCK_TMNORM;
   }
   else
      rgb *= planckPower(T) * 1e-6f;

   out_col = chr;
   out_val = luma(rgb) / luma(chr);
}


enum BlackbodyParams
{
   p_temperature,
   p_mode,
   p_tone_map,
   p_adaptation,
   p_burn
};

enum BlackbodyMode
{
   BLACKBODY_MODE_COLOR = 0,
   BLACKBODY_MODE_CHROMA
};

static const char* blackbody_mode_strings[] = { "color", "chroma", NULL };

} // namespace anonymous

node_parameters
{
   AiParameterFlt ("temperature", 6500.0f);
   AiParameterEnum("mode",        BLACKBODY_MODE_COLOR, blackbody_mode_strings);
   AiParameterBool("tone_map",    true);
   AiParameterFlt ("adaptation",  0.18f);
   AiParameterFlt ("burn",        0.0f);
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
   float temperature = AiShaderEvalParamFlt(p_temperature);
   int mode = AiShaderEvalParamEnum(p_mode);
   bool tone_map = AiShaderEvalParamBool(p_tone_map);
   float adaptation = AiShaderEvalParamFlt(p_adaptation);
   float burn = AiShaderEvalParamFlt(p_burn);

   AtRGB chroma;
   float intensity;
   blackbody(temperature, tone_map, adaptation, burn, chroma, intensity);

   switch (mode)
   {
   case BLACKBODY_MODE_COLOR:
      sg->out.RGB = chroma * intensity;
      sg->out.RGBA.a = 1.0f;
      break;

   case BLACKBODY_MODE_CHROMA:
      sg->out.RGB = chroma;
      sg->out.RGBA.a = intensity;
      break;
   }
}


void registerBlackbody(AtNodeLib *node)
{
   node->methods     = BlackbodyMethods;
   node->output_type = AI_TYPE_RGBA;
   node->name        = SHADERLIB_PREFIX "blackbody";
   node->node_type   = AI_NODE_SHADER;
}

