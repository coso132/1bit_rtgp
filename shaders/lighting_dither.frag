#version 410 core

out vec4 colorFrag;

in vec3 N;
in vec3 L;
in vec4 FragPosLightSpace;
uniform sampler2D shadowMap;

// Shadow calculation using simple PCF (3x3)
float calculateShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // If outside the light frustum, no shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 1.0; // fully lit
    
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normalize(N), normalize(L))), 0.005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // 3x3 PCF
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return 1.0 - shadow;
}

void main(){
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	float ambient = 0.1;

    float rawDiffuse = max(dot(n,l),0.0);
    float shadowFactor=calculateShadow(FragPosLightSpace);
    float diffuse = ambient + (rawDiffuse*0.9) * shadowFactor;
    diffuse = smoothstep(0.0,0.5,diffuse);
    // force full black or white if diffuse value is 0 or 1
    if (diffuse <=0.0) {
        colorFrag = vec4(0.0,0.0,0.0,1.0);
        return;
    }
    if (diffuse >=1.0) {
        colorFrag = vec4(1.0,1.0,1.0,1.0);
        return;
    }
	// quantize diffuse in few different possible light levels
	diffuse = floor(diffuse * 3.0) / 3.0;

    // 4x4 Bayer pattern (normalised to [0,1])
    const float bayer[16] = float[](
        0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
        12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
        3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
        15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
    );

    // get pixel screen coordinates (pixel centre)
    int x = int(mod(gl_FragCoord.x, 4.0));
    int y = int(mod(gl_FragCoord.y, 4.0));
    int idx = y * 4 + x;
    float threshold = bayer[idx];

    // compare diffuse intensity with threshold
    float finalIntensity = (diffuse > threshold) ? 1.0 : 0.0;
    colorFrag = vec4(vec3(finalIntensity), 1.0);
    // debugging purposes
    // colorFrag = vec4(vec3(diffuse), 1.0);
}
