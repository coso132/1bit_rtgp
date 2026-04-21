/*
19_shadowmap.vert: vertex shader for the creation of the shadow map

It applies to the vertex the projection and view transformations of the light (used as a camera)

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2025/2026
Master degree in Computer Science
Universita' degli Studi di Milano
*/


#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 lightSpaceMatrix;

uniform mat4 modelMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(position, 1.0f);
}
