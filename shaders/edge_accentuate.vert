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
uniform float fill_in; // if the object has a complex model, set a solid color for each fragment of this model

flat out vec3 N;
out vec3 pos;
out float object_id;
out float fill;
out float distance;

void main()
{
    vec4 idk = viewMatrix * modelMatrix * vec4(position,1.0);
    // distance = length(idk.xyz);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    N = normalize(normalMatrix * normal);
    pos = (viewMatrix * modelMatrix * vec4(object_pos_in, 1.0)).xyz;
    object_id = object_id_in;
    fill = fill_in;
}
