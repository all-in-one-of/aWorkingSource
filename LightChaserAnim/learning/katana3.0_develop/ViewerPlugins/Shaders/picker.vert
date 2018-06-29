#version 400

layout( location = 0 ) in vec3 VertexPosition;
layout( location = 1 ) in vec3 VertexColor;

out vec3 Pos;
out vec3 Color;

uniform dmat4 ViewMatrix;
uniform dmat4 WorldMatrix;
uniform dmat4 ProjectionMatrix;

void main()
{
    Color = VertexColor;
    gl_Position =  vec4(ProjectionMatrix * ViewMatrix * WorldMatrix * dvec4(VertexPosition, 1.0f));
    Pos = vec4(ProjectionMatrix * ViewMatrix * WorldMatrix * dvec4(VertexPosition, 1.0f)).xyz;
}

