Shader "ShaderMan/Terrain"
{
    Properties{
        _MainTex("MainTex", 2D) = "white" {}
        _SecondTex("Variation Noise", 2D) = "white" {}
    }
 
        SubShader
    {
    Tags { "RenderType" = "Transparent" "Queue" = "Transparent" }
 
    Pass
    {
    ZWrite Off
    Blend SrcAlpha OneMinusSrcAlpha
 
    CGPROGRAM
    #pragma vertex vert
    #pragma fragment frag
    #include "UnityCG.cginc"
 
    struct VertexInput {
    fixed4 vertex : POSITION;
    fixed2 uv : TEXCOORD0;
    fixed4 tangent : TANGENT;
    fixed3 normal : NORMAL;
    //VertexInput
    };
 
    struct VertexOutput {
    fixed4 pos : SV_POSITION;
    fixed2 uv : TEXCOORD0;
    //VertexOutput
    };
 
    //Variables
    sampler2D _SecondTex;
    float4 _SecondTex_TexelSize;
    sampler2D _MainTex;
 
    // The MIT License
// Copyright © 2015 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, fmodify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
// One simple way to avoid tex2D tile repetition, at the cost of 4 times the amount of
// tex2D lookups (still much better than https://www.shadertoy.com/view/4tsGzf)
//
// More info: http://www.iquilezles.org/www/articles/tex2Drepetition/tex2Drepetition.htm
 
#define USEHASH
 
    fixed4 hash4(fixed2 p)
    {
        return frac(sin(fixed4(
                        1.0 + dot(p,fixed2(37.0,17.0)),
                        2.0 + dot(p,fixed2(11.0,47.0)),
                        3.0 + dot(p,fixed2(41.0,29.0)),
                        4.0 + dot(p,fixed2(23.0,31.0))))*103.0);
    }
 
    float map(float a, float b, float r, float s, float value)
    {
        if (a == b) { return (value <= a) ? r : s; }
 
        float ratio = (value - a) / (b - a);
        return r + (s - r) * ratio;
    }
 
    float mapAndClamp(float a, float b, float s, float t, float value)
    {
        return clamp(map(a, b, s, t, value), s, t);
    }
 
    float4 pointSampleTex2D(sampler2D tex, float2 uv, float4 st)
    {
        float2 snappedUV = ((float2)((int2)(uv * st.zw + float2(1, 1))) - float2(0.5, 0.5)) * st.xy;
        return tex2Dlod(tex, float4(snappedUV.x, snappedUV.y, 0, 0));
    }
    //HLSL compatible adaption of GL_NEAREST_MIPMAP_LINEARto by
    // https://forum.unity.com/threads/filter-mode-point-and-linear-filter-on-a-texture-based-on-the-distance.462621/#post-3009740
    float4 crispMipMapTex2D(sampler2D tex, float2 uv, float4 texelSize)
    {
        float2 _ddx = ddx(uv);
        float2 _ddy = ddy(uv);
        float lod = max(_ddx * texelSize.zw.x, _ddy *  texelSize.zw.x);
        // float lod = sqrt(pow(_ddx * texelSize.zw.x, 2) + pow(_ddy *  texelSize.zw.x, 2));
        float t = mapAndClamp(0.5, 1, 0, 1, lod);
        return lerp(pointSampleTex2D(tex, uv, texelSize), tex2D(tex, uv, _ddx, _ddy), t);
    }
 
    fixed4 tex2DNoTile(sampler2D samp, in fixed2 uv)
    {
        fixed2 iuv = floor(uv);
        fixed2 fuv = frac(uv);
        fixed2 _ddx = ddx(uv);
        fixed2 _ddy = ddy(uv);
 
    #ifdef USEHASH
        // generate per-tile transform (needs GL_NEAREST_MIPMAP_LINEARto work right)
        fixed4 ofa = crispMipMapTex2D(_SecondTex, (iuv + fixed2(0.5,0.5)) / 256.0, _SecondTex_TexelSize);
        fixed4 ofb = crispMipMapTex2D(_SecondTex, (iuv + fixed2(1.5,0.5)) / 256.0, _SecondTex_TexelSize);
        fixed4 ofc = crispMipMapTex2D(_SecondTex, (iuv + fixed2(0.5,1.5)) / 256.0, _SecondTex_TexelSize);
        fixed4 ofd = crispMipMapTex2D(_SecondTex, (iuv + fixed2(1.5,1.5)) / 256.0, _SecondTex_TexelSize);
    #else
        // generate per-tile transform
        fixed4 ofa = hash4(iuv + fixed2(0.0,0.0));
        fixed4 ofb = hash4(iuv + fixed2(1.0,0.0));
        fixed4 ofc = hash4(iuv + fixed2(0.0,1.0));
        fixed4 ofd = hash4(iuv + fixed2(1.0,1.0));
    #endif
 
        // transform per-tile uvs
        ofa.zw = sign(ofa.zw - 0.5);
        ofb.zw = sign(ofb.zw - 0.5);
        ofc.zw = sign(ofc.zw - 0.5);
        ofd.zw = sign(ofd.zw - 0.5);
 
        // uv's, and derivarives (for correct mipmapping)
        fixed2 uva = uv * ofa.zw + ofa.xy; fixed2 ddxa = _ddx * ofa.zw; fixed2 ddya = _ddy * ofa.zw;
        fixed2 uvb = uv * ofb.zw + ofb.xy; fixed2 ddxb = _ddx * ofb.zw; fixed2 ddyb = _ddy * ofb.zw;
        fixed2 uvc = uv * ofc.zw + ofc.xy; fixed2 ddxc = _ddx * ofc.zw; fixed2 ddyc = _ddy * ofc.zw;
        fixed2 uvd = uv * ofd.zw + ofd.xy; fixed2 ddxd = _ddx * ofd.zw; fixed2 ddyd = _ddy * ofd.zw;
 
        // fetch and blend
        fixed2 b = smoothstep(0.25,0.75,fuv);
 
        return lerp(lerp(tex2Dgrad(samp, uva, ddxa, ddya),
                        tex2Dgrad(samp, uvb, ddxb, ddyb), b.x),
                    lerp(tex2Dgrad(samp, uvc, ddxc, ddyc),
                        tex2Dgrad(samp, uvd, ddxd, ddyd), b.x), b.y);
    }
 
    VertexOutput vert(VertexInput v)
    {
        VertexOutput o;
        o.pos = UnityObjectToClipPos(v.vertex);
        o.uv = v.uv;
        //VertexFactory
        return o;
    }
    fixed4 frag(VertexOutput i) : SV_Target
    {
        fixed2 uv = i.uv / 1;
 
        fixed f = smoothstep(0.4, 0.6, sin(_Time.y));
        fixed s = smoothstep(0.4, 0.6, sin(_Time.y*0.5));
 
        uv = (4.0 + 16.0*s)*uv + _Time.y*0.1;
 
        fixed3 cola = tex2DNoTile(_MainTex, uv).xyz;
        fixed3 colb = tex2D(_MainTex, uv).xyz;
 
        fixed3 col = lerp(cola, colb, f);
 
        return fixed4(col, 1.0);
    }
    ENDCG
    }
    }
}