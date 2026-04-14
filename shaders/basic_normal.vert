#version 410 core

// vertex position in world coordinates
// the number used for the location in the layout qualifier is the position of the vertex attribute
// as defined in the Mesh class
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 position;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;
// the transformed normal is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 N;

void main()
{
    // transformations are applied to each vertex
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    // the normal is transformed using the normal matrix and passed to the fragment shader
    N = normalize(normalMatrix * normal);
}
