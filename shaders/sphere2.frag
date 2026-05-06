#version 410 core
out vec4 colorFrag;

in vec2 vTexCoords;
uniform sampler2D noise;
uniform float tile = 1.0;

void main()
{
    // vec3 dir = normalize(vTexCoords);
    // vec3 correctedDir = sphereToCube(dir);
    colorFrag = texture(noise, vTexCoords*tile);
    // colorFrag = vec4(dir,1.0);
    // colorFrag = vec4(0.0,0.0,0.0,1.0);
}