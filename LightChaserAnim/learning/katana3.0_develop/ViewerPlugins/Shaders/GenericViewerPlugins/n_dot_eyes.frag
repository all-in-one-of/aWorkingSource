#version 420
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4 FragColor;

uniform vec3 Position;
uniform vec3 Normal;
uniform vec3 CameraPos;
uniform vec3 Color;

void main()
{
    float dot_product = clamp(dot(Normal, Normal), 0, 1);
    // FragColor = dot_product*vec4(0.5, 0.5, 1, 1.0);
    FragColor = vec4(Normal, 1.0);
}