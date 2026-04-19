//this shader (edge_accentuate.vert and edge_accentuate.frag) emulates how return of the obra dinn accentuate edges by coloring fragments using the normal, randomly generated values, and manually set adjustment colors.
// the image rendered by this shader is then processed by a simple post-processing edge detection shader to create the actual edges.
// this vertex shader only needs to pass relevant data to the fragment shader, which will do most of the work
#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform float object_id_in;
uniform vec3 object_pos_in;
uniform float fill_in;

flat out vec3 N;
out vec3 pos;
out float object_id;
out float fill;

void main()
{
    // transformations are applied to each vertex
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    // the normal is transformed using the normal matrix and passed to the fragment shader
    N = normalize(normalMatrix * normal);
    // pass the view space position of the whole object to the fragment shader
    pos = (viewMatrix * modelMatrix * vec4(object_pos_in, 1.0)).xyz;
    object_id = object_id_in;
    fill = fill_in;
}
