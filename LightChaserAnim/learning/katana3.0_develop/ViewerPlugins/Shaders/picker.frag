#version 420
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4 FragColor;

uniform int pickerID;

void main()
{
    int r = pickerID >> 24 & 0xFF;
    int g = pickerID >> 16 & 0xFF;
    int b = pickerID >> 8 & 0xFF;
    int a = pickerID & 0xFF;
    FragColor = vec4(r / 255.0, g / 255.0,
                     b / 255.0, a / 255.0);
}
