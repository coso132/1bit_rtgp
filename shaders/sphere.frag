#version 410 core
out vec4 colorFrag;

in vec3 vTexCoords;
uniform samplerCube uCubemap;

vec3 sphereToCube(vec3 s) {
    vec3 s2 = s * s;
    // Approximation of the inverse mapping
    float rx = s.x / sqrt(1.0 - (s2.y + s2.z)/3.0 + (s2.y * s2.z)/5.0);
    float ry = s.y / sqrt(1.0 - (s2.z + s2.x)/3.0 + (s2.z * s2.x)/5.0);
    float rz = s.z / sqrt(1.0 - (s2.x + s2.y)/3.0 + (s2.x * s2.y)/5.0);
    return normalize(vec3(rx, ry, rz));
}
void main()
{
    vec3 dir = normalize(vTexCoords);
    vec3 correctedDir = sphereToCube(dir);
    colorFrag = texture(uCubemap, correctedDir);
    // colorFrag = vec4(dir,1.0);
    // colorFrag = vec4(0.0,0.0,0.0,1.0);
}