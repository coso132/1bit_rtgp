// this shader will be used to detect edges in the low-res render, and output a texture where the edges are white and the rest is black. 
// this uses a simple edge detaction algorithm based on the difference in color between neighboring pixels.
#version 410 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D edge1_texture;
uniform sampler2D edge2_texture;

// Edge detection kernel (Laplacian)
const float kernel[9] = float[](
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
);

void main() {
    // ----- First edge source -----
    vec2 tex_offset = 1.0 / textureSize(edge1_texture, 0);
    vec3 result = vec3(0.0);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            vec2 offset = vec2(float(i), float(j)) * tex_offset;
            result += texture(edge1_texture, TexCoord + offset).rgb * kernel[(i+1)*3 + (j+1)];
        }
    }
    float edge_strength1 = length(result);
    
    // Soft threshold with anti‑aliasing (transition band = 0.1)
    float edge_threshold1 = 0.6;
    float alpha1 = smoothstep(edge_threshold1 - 0.1, edge_threshold1 + 0.1, edge_strength1);
    vec4 result1 = vec4(vec3(alpha1), 1.0);   // white on black, intensity varies smoothly

    // ----- Second edge source -----
    tex_offset = 1.0 / textureSize(edge2_texture, 0);
    result = vec3(0.0);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            vec2 offset = vec2(float(i), float(j)) * tex_offset;
            result += texture(edge2_texture, TexCoord + offset).rgb * kernel[(i+1)*3 + (j+1)];
        }
    }
    float edge_strength2 = length(result);
    
    float edge_threshold2 = 0.90;
    float alpha2 = smoothstep(edge_threshold2 - 0.1, edge_threshold2 + 0.1, edge_strength2);
    vec4 result2 = vec4(vec3(alpha2), 1.0);

    // Combine both edges (use max to avoid double brightening, but add works too)
    vec4 finalEdge = max(result1, result2);
    // Optional: blend with original colour? For now, white outline on black.
    FragColor = finalEdge;
}