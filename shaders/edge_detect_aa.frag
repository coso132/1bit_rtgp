// this shader will be used to detect edges in the low-res render, and output a texture where the edges are white and the rest is black. 
// this uses a simple edge detaction algorithm based on the difference in color between neighboring pixels.
#version 410 core
//TODO COMMENT
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D edge1_texture;
uniform sampler2D edge2_texture;
uniform sampler2D blue_noise;

// edge detection kernel (Laplacian)
const float kernel[9] = float[](
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
);

void main() {
    // check if fragment is dust and return white if it is, 
    // so that it is not affected by edge detection and remains visible
    vec3 color = texture(edge2_texture, TexCoord).rgb;
    if (color.b >= 1.0 && color.r <=0.0 && color.g <=0.0 ){
        FragColor = vec4(vec3(1.0), 1.0);
        return;
    }

    // apply edge detection to edge1_texture
    vec2 tex_offset = 1.0 / textureSize(edge1_texture, 0);
    vec3 result = vec3(0.0);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            vec2 offset = vec2(float(i), float(j)) * tex_offset;
            result += texture(edge1_texture, TexCoord + offset).rgb * kernel[(i+1)*3 + (j+1)];
        }
    }
    float edge_strength1 = length(result);
    
    // soft threshold with antialiasing 
    float edge_threshold1 = 0.6;
    float alpha1 = smoothstep(edge_threshold1 - 0.1, edge_threshold1 + 0.1, edge_strength1);
    vec4 result1 = vec4(vec3(alpha1), 1.0);   // white on black, intensity varies smoothly
    FragColor = result1;
}