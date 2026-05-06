#version 410 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;

out vec3 vTexCoords;
void main()
{
    vTexCoords = aPos;
    gl_Position = mvp * vec4(aPos, 1.0);
}

