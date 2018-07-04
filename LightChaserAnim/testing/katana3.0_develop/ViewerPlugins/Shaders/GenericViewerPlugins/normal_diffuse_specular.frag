#version 420
#extension GL_ARB_shading_language_420pack : enable

in vec3 Normal;

uniform vec4 DiffuseColor;
uniform vec4 SpecularColor;

layout(location=0) out vec4 FragColor;

void main()
{
    // Ia Ka + Id Kd N.L + Is Ks (R.V)^n
    // Ia = (0,0,0,1)
    // Id = (1,1,1,1)
    // Is = (1,1,1,1)
    // n = 0
    float NdotL = dot(normalize(Normal), vec3(0.0, 0.0, 1.0));
    vec4 primaryColor = DiffuseColor * max(NdotL, 0.0) + SpecularColor;
    FragColor = vec4(primaryColor.rgb, DiffuseColor.a);
}
