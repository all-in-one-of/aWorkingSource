#version 400

layout( location = 0 ) in vec3 VertexPosition;

uniform dmat4 ViewMatrix;
uniform dmat4 WorldMatrix;
uniform dmat4 ProjectionMatrix;

void main()
{
    dmat4 xform = ProjectionMatrix * ViewMatrix * WorldMatrix;
    gl_Position =  vec4(xform * dvec4(VertexPosition, 1.0f));
}
