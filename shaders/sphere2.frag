#version 410 core
out vec4 colorFrag;

in vec2 vTexCoords;
uniform sampler2D noise;
uniform float tile = 1.0;

void main()
{
    colorFrag = texture(noise, vTexCoords*tile);
}