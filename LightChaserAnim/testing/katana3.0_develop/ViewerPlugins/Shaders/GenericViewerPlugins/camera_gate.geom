#version 400

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

flat out int Horizontal;

void main()
{
    // We want to pass down to the fragment shader whether the line is
    // horizontal or vertical.
    Horizontal = int(gl_in[0].gl_Position.x != gl_in[1].gl_Position.x);

    // Pass-through the line.
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    EndPrimitive();
}
