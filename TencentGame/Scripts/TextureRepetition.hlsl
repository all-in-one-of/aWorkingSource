int2 iuv = int2( floor( uv ) );
float2 fuv = frac( uv );

// generate per-tile transform
float2 ofa_2 = iuv + int2(0,0);
float2 ofb_2 = iuv + int2(1,0);
float2 ofc_2 = iuv + int2(0,1);
float2 ofd_2 = iuv + int2(1,1);
float4 ofa = frac(sin(float4( 1.0+dot(ofa_2,float2(37.0,17.0)),2.0+dot(ofa_2,float2(11.0,47.0)),3.0+dot(ofa_2,float2(41.0,29.0)),4.0+dot(ofa_2,float2(23.0,31.0))))*103.0);
float4 ofb = frac(sin(float4( 1.0+dot(ofb_2,float2(37.0,17.0)),2.0+dot(ofb_2,float2(11.0,47.0)),3.0+dot(ofb_2,float2(41.0,29.0)),4.0+dot(ofb_2,float2(23.0,31.0))))*103.0);
float4 ofc = frac(sin(float4( 1.0+dot(ofc_2,float2(37.0,17.0)),2.0+dot(ofc_2,float2(11.0,47.0)),3.0+dot(ofc_2,float2(41.0,29.0)),4.0+dot(ofc_2,float2(23.0,31.0))))*103.0);
float4 ofd = frac(sin(float4( 1.0+dot(ofd_2,float2(37.0,17.0)),2.0+dot(ofd_2,float2(11.0,47.0)),3.0+dot(ofd_2,float2(41.0,29.0)),4.0+dot(ofd_2,float2(23.0,31.0))))*103.0);

float2 ddx2 = ddx( uv );
float2 ddy2 = ddy( uv );

// transform per-tile uvs
ofa.zw = sign( ofa.zw-0.5 );
ofb.zw = sign( ofb.zw-0.5 );
ofc.zw = sign( ofc.zw-0.5 );
ofd.zw = sign( ofd.zw-0.5 );

// uv's, and derivatives (for correct mipmapping)
float2 uva = uv*ofa.zw + ofa.xy; 
float2 ddxa = ddx2*ofa.zw; 
float2 ddya = ddy2*ofa.zw;

float2 uvb = uv*ofb.zw + ofb.xy; 
float2 ddxb = ddx2*ofb.zw; 
float2 ddyb = ddy2*ofb.zw;

float2 uvc = uv*ofc.zw + ofc.xy; 
float2 ddxc = ddx2*ofc.zw; 
float2 ddyc = ddy2*ofc.zw;

float2 uvd = uv*ofd.zw + ofd.xy; 
float2 ddxd = ddx2*ofd.zw; 
float2 ddyd = ddy2*ofd.zw;


// fetch and blend
float2 b = smoothstep( 0.25,0.75, fuv );
sampler2D samp = texSampler;
return lerp( lerp( tex2Dgrad ( samp, uva, ddxa, ddya ),tex2Dgrad( samp, uvb, ddxb, ddyb ), b.x ),lerp( tex2Dgrad ( samp, uvc, ddxc, ddyc ),tex2Dgrad ( samp, uvd, ddxd, ddyd ), b.x), b.y );