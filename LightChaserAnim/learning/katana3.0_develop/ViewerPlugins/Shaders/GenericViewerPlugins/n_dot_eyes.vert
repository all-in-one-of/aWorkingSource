#version 400

layout( location = 0 ) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout( location = 2 ) in vec3 VertexColor;

out vec3 Position;
out vec3 Normal;
out vec3 CameraPos;
out vec3 Color;

uniform dmat4 ViewMatrix;
uniform dmat4 WorldMatrix;
uniform dmat4 ProjectionMatrix;

void main()
{
    vec3 cameraPosition = vec3(-transpose(mat3(ViewMatrix)) * ViewMatrix[3].xyz);
    dmat4 xform = ProjectionMatrix * ViewMatrix * WorldMatrix;
    gl_Position =  vec4(xform * dvec4(VertexPosition, 1.0f));
    Position = vec4(xform * dvec4(VertexPosition, 1.0f)).xyz;
    Normal = vec3((ViewMatrix * WorldMatrix * dvec4(VertexNormal, 0.0f)).xyz);
    CameraPos = cameraPosition;
    Color = VertexColor;
}
