#version 420
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4 FragColor;

uniform vec4 Color;

void main()
{
    FragColor = vec4(0.5, 0.5, 0.5, 1.0);
}