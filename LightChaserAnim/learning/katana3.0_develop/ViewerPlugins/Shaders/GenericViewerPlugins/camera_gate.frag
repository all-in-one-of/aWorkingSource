#version 420
#extension GL_ARB_shading_language_420pack : enable

layout ( location = 0 ) out vec4 FragColor;

flat in int Horizontal;

uniform vec4 Color;
uniform int Pattern;
uniform int LineWidth;

void main()
{
    float value = bool(Horizontal) ? gl_FragCoord.x : gl_FragCoord.y;
    int adjustedValue = (int(value) / LineWidth) % 32;
    int colorIt = (Pattern >> adjustedValue) & 0x1;
    FragColor = colorIt * Color;
}
