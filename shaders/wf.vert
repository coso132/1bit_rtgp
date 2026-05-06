#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    vec4 mv = viewMatrix * worldPos;
    gl_Position = projectionMatrix * mv;
}
