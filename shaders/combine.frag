#version 410 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D lightingTexture;
uniform sampler2D edgeTexture;
void main() {
    vec4 light_color = texture(lightingTexture, TexCoord);
    vec4 edge_color = texture(edgeTexture, TexCoord);

    if (edge_color.r < 0.5) { // not in an edge
        FragColor = light_color;
    } else { // if in an edge
        if (light_color.r > 0.5) { // if lit, use black for edge 
            FragColor = 1 - edge_color; // invert edge color to get black
        } else { // if unlit, use normal edge color
            FragColor = edge_color;
        }
    }
    // debugging purposes 
    // FragColor = edge_color;
    // FragColor = light_color;
}