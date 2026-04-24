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

out vec4 FragColor;
void main() { 
    vec4 normal_color = texture(normal_tex,interp_UV);
    FragColor = normal_color;
    return;

    vec3 norm = norm_out * normal_color.r ;
    // simple lighting
    float light = clamp((dot(norm_out, camera_dir_out) + 1.0) * 0.5, 0.0, 1.0);
    

    // vec4 surfaceColor = texture(normal_tex, UV);
    // base color from world position 
    vec3 color = ((object_pos_out + area_out) * 0.5) / area_out; //roughly mapped to [0,1]
    // multiple red channel by "light"
    color.r *= light;
    // divide green channel by "light"
    color.g /= (light ==0.0 ? 1e-6 : light);
    color *= normal_color.g;
    // color *= light; //* length(norm);
    // color *= light * length(norm);

    color = fract(color * 1000.0);
    // color = surfaceColor.rgb;
    vec4 v_color = vec4(color,1.0);



    if (fill >=0.9){
        float r = fract((object_id * 2.94) );
        float g = fract((object_id * 8.38) );
        float b = fract((object_id * 4.333));
        FragColor = vec4(r,g,b,1.0);
        return; 
    }
    // float r = fract(v_color.r * (object_id * 2.94) );
    // float g = fract(v_color.g * (object_id * 8.38) );
    // float b = fract(v_color.b * (object_id * 4.333));
    // float r = fract(v_color.r * () );
    // float g = fract(v_color.g * (object_id * 8.38) );
    // float b = fract(v_color.b * (object_id * 4.333));
    FragColor = v_color;
    // FragColor = vec4(r,g,b,1.0);
    // FragColor = texture(normal_tex,interp_UV);
    // FragColor = v_color;
}