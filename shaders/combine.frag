#version 410 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D lightingTexture;
uniform sampler2D edgeTexture;
void main() {
    vec4 light_color = texture(lightingTexture, TexCoord);
    float lighting = light_color.r;
    float texture_color = light_color.g;
    vec4 edge_color = texture(edgeTexture, TexCoord);
    float edge = edge_color.r;
    float final_color = 0.0;
    if (edge < 0.5) { // not in an edge
        final_color = lighting * texture_color;
    } else // if in an edge
    { 
        if (lighting > 0.5) { // if lit, use black for edge 
            final_color = 0.0; // invert edge color to get black
        } else { // if unlit, use normal edge color
            final_color = 1.0;
        }
    }
    FragColor = vec4(vec3(final_color),1.0);
    // debugging purposes 
    // FragColor = edge_color;
    // FragColor = light_color;
}