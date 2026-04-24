//this shader (edge_accentuate.vert and edge_accentuate.frag) emulates how return of the obra dinn accentuate edges by coloring fragments using the normal, randomly generated values, and manually set adjustment colors.
// the image rendered by this shader is then processed by a simple post-processing edge detection shader to create the actual edges.
// this vertex shader only needs to pass relevant data to the fragment shader, which will do most of the work
#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 UV;
// layout (location = 1) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
// uniform mat3 normalMatrix;

uniform float object_id_in;
// uniform vec3 object_pos_in;
uniform float fill_in; // if the object has a complex model, set a solid color for each fragment of this model

// flat out vec3 N;
// out vec3 pos;
out vec4 v_color;
out float object_id;
out float fill;
out vec2 interp_UV;
// out float distance;

out vec3 norm_out;
out vec3 camera_dir_out;
out vec3 area_out;
out vec3 object_pos_out;
void main(){
    interp_UV = UV;
    mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
    gl_Position = MVP * vec4(position, 1.0);

    // object position in world space
    vec3 object_pos = (modelMatrix * vec4(0.0,0.0,0.0,1.0)).xyz; 
    object_pos_out = object_pos;

    vec3 area = vec3(100.0,100.0,100.0); // you gotta make due with what you got
    area_out = area;

    // camera direction transformed by view matrix's rotation part
    // (from dev's unity code)
    vec3 camera_dir = mat3(viewMatrix) * vec3(0.0,0.0,1.0);
    camera_dir_out = camera_dir;

    // world space normal
    vec3 norm = (modelMatrix * vec4(normal,0.0)).xyz;
    norm_out = norm;
    // norm *= vertex_color_r (we dont have that with .obj files!)


    object_id = object_id_in;
    fill = fill_in;
}
