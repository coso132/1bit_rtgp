#version 410 core

// vertex position in world coordinates
// the number used for the location in the layout qualifier is the position of the vertex attribute
// as defined in the Mesh class
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

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

void main()
{
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * viewMatrix * worldPos;

    N = normalize(normalMatrix * normal);
    vec3 lightDirView = mat3(viewMatrix) * (-directionalLightDirection);
    L = normalize(lightDirView);

    FragPosLightSpace = lightSpaceMatrix * worldPos;
}
