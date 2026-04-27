#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 UV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform vec3 directionalLightDirection;
uniform float directionalLightIntensity;
uniform mat4 lightSpaceMatrix; // from world coordinates to light's clip space

out vec3 N;
out vec3 L;
out vec4 FragPosLightSpace;
out vec2 interp_UV;
// out vec3 pointL;
out vec4 mv;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    mv = viewMatrix * worldPos;
    gl_Position = projectionMatrix * mv;

    N = normalize(normalMatrix * normal);
    vec3 lightDirView = mat3(viewMatrix) * (-directionalLightDirection);
    L = normalize(lightDirView);

    FragPosLightSpace = lightSpaceMatrix * worldPos;
    interp_UV = UV;
}
