#version 420
#extension GL_ARB_shading_language_420pack : enable

in vec4 Color;

layout(location=0) out vec4 FragColor;

void main()
{
    FragColor = Color;
}
