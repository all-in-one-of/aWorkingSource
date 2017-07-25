#pragma once

const int DEFUALT_LAYER = INT_MAX;                 // when no layer is assigned
const int SUPER_LAYER = -4095;                     // the layer that blend all layer.
static const float WEIGHT_ADD = 0.05f;             // weight threshold to become 1

struct ShaderData{
   bool  enable;
   float radius, min_blend_dist, weight_curve, adv_offsetScale;
   int   nsamples;
   AtSampler* sampler;
};

struct SGCache{
   
   void initCache(const AtShaderGlobals *sg){
      N = sg->N;
      Nf = sg->Nf;
      Ng = sg->Ng;
      Ngf = sg->Ngf;
      Ns = sg->Ns;
   }

   void restoreSG(AtShaderGlobals *sg){
      sg->N = N;
      sg->Nf = Nf;
      sg->Ng = Ng;
      sg->Ngf = Ngf;
      sg->Ns = Ns;
   }

   AtVector N;
   AtVector Nf;
   AtVector Ng;
   AtVector Ngf;
   AtVector Ns;
};

enum Params
{
   p_enable=0,
   p_radius,
   p_minBlendDist,
   p_sample,
   p_blend_bias,
   p_advOffsetScale,
};

// concentricSampleDisk and cosineSampleHemisphere lifted from PBRT
/*
Copyright (c) 1998-2012, Matt Pharr and Greg Humphreys.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
inline void concentricSampleDisk(float u1, float u2, float& dx, float& dy) 
{
    float r, theta;
    // Map uniform random numbers to $[-1,1]^2$
    float sx = 2 * u1 - 1;
    float sy = 2 * u2 - 1;

    // Map square to $(r,\theta)$

    // Handle degeneracy at the origin
    if (sx == 0.0 && sy == 0.0) {
        dx = 0.0;
        dy = 0.0;
        return;
    }
    if (sx >= -sy) {
        if (sx > sy) {
            // Handle first region of disk
            r = sx;
            if (sy > 0.0)
                theta = sy / r;
            else
                theta = 8.0f + sy / r;
        } else {
            // Handle second region of disk
            r = sy;
            theta = 2.0f - sx / r;
        }
    } else {
        if (sx <= sy) {
            // Handle third region of disk
            r = -sx;
            theta = 4.0f - sy / r;
        } else {
            // Handle fourth region of disk
            r = -sy;
            theta = 6.0f + sx / r;
        }
    }
    theta *= float(AI_PI) / 4.f;
    dx = r * cosf(theta);
    dy = r * sinf(theta);
}