#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Normal;

uniform dmat4 ViewMatrix;
uniform dmat4 WorldMatrix;
uniform dmat4 ProjectionMatrix;

void main()
{
    gl_Position = vec4(ProjectionMatrix * ViewMatrix * WorldMatrix * dvec4(VertexPosition, 1.0f));
    Normal = vec3((ViewMatrix * WorldMatrix * dvec4(VertexNormal, 0.0f)).xyz);
}
