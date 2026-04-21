// this shader will be used to detect edges in the low-res render, and output a texture where the edges are white and the rest is black. 
// this uses a simple edge detaction algorithm based on the difference in color between neighboring pixels.
#version 410 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D lowResTexture;
// edge detection kernel 
const float kernel[9] = float[](
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
);
void main() {
    vec2 tex_offset = 1.0 / textureSize(lowResTexture, 0); // gets size of single texel
    vec3 result = vec3(0.0);
    // apply the kernel to the neighboring pixels
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            vec2 offset = vec2(float(i), float(j)) * tex_offset;
            result += texture(lowResTexture, TexCoord + offset).rgb * kernel[(i+1)*3 + (j+1)];
        }
    }
    float edge_strength = length(result);
    float edge_threshold = 0.6; // adjust this threshold as needed
    FragColor = vec4(vec3(edge_strength > edge_threshold ? 1.0 : 0.0), 1.0);
    // debugging purposes
    // FragColor = vec4(vec3(result),1.0);
    // FragColor = texture(lowResTexture, TexCoord);
}