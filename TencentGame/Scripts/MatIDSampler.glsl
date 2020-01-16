float mID=trunc(matID*256);
if(mID<=0){
    return Texture2DSample(tex0, tex0Sampler, uv);
}
else if(mID<=1){
    return Texture2DSample(tex1, tex1Sampler, uv);
}
else if(mID<=2){
    return Texture2DSample(tex2, tex2Sampler, uv);
}
else if(mID<=3){
    return Texture2DSample(tex3, tex3Sampler, uv);
}
return 0;