#version 410 core
in vec4 v_color;
in float object_id;
in float fill;
in vec2 interp_UV;

in vec3 norm_out;
in vec3 camera_dir_out;
in vec3 area_out;
in vec3 object_pos_out;

uniform sampler2D normal_tex;

// uniform sampler2D blue_noise;
uniform int dust;

out vec4 FragColor;

// vec4 blue_noise_dither(float diffuse){
//     float min =0.2;
//     float max =0.8;
//     if (diffuse <=min){
//         return vec4(0.0,0.0,0.0,1.0);}
//     if (diffuse >=max){
//         return vec4(1.0,1.0,1.0,1.0);}
//     float threshold = (texture(blue_noise, gl_FragCoord.xy/64.0)).r;
//     float finalIntensity = (diffuse > threshold) ? 1.0 : 0.0;
//     return vec4(vec3(finalIntensity), 1.0);
// }

void main() { 
    vec4 normal_color = texture(normal_tex,interp_UV);
    FragColor = normal_color;
    if (dust == 1){
        FragColor = vec4(0.0,0.0,1.0,1.0);
        //sample blue noise like dither 
        // filter for treshold like dither? maybe based on something
        // discard light pixels
        // return contrasted pixels
    }
    return;
}