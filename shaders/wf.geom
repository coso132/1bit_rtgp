#version 410 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

uniform vec4 uWireColor;   // color of the wireframe lines

void main()
{
    // Clip-space positions of the triangle's three vertices
    vec4 v0 = gl_in[0].gl_Position;
    vec4 v1 = gl_in[1].gl_Position;
    vec4 v2 = gl_in[2].gl_Position;

    // Edge v0 -> v1
    gl_Position = v0; EmitVertex();
    gl_Position = v1; EmitVertex();
    EndPrimitive();

    // Edge v1 -> v2
    gl_Position = v1; EmitVertex();
    gl_Position = v2; EmitVertex();
    EndPrimitive();

    // Edge v2 -> v0
    gl_Position = v2; EmitVertex();
    gl_Position = v0; EmitVertex();
    EndPrimitive();
}
