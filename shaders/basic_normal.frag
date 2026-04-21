#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 N;

// the transformed light direction has been calculated per-vertex in the vertex shader
in vec3 L;

in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;

// Shadow calculation using simple PCF (3x3)
float calculateShadow(vec4 fragPosLightSpace) {
    // Perspective division to NDC [-1,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range for texture lookup
    projCoords = projCoords * 0.5 + 0.5;
    
    // If outside the light frustum, no shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 1.0; // fully lit
    
    float currentDepth = projCoords.z;
    // Bias to reduce shadow acne (tune based on your scene)
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
    // if (projCoords.z > 1.0) shadow = 0.0; 
    
    return   1- shadow; // 1.0 = fully lit, 0.0 = fully shadowed
}

void main(){
	// we use the normal as a color
	// final color (0,1) black and white
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	float ambient = 0.1;

    float rawDiffuse = max(dot(n,l),0.0);
    float shadowFactor=calculateShadow(FragPosLightSpace);
    float diffuse = ambient + (rawDiffuse*0.9) * shadowFactor;

	// quantize diffuse in 4 different possible light levels
	// diffuse = floor(diffuse * 4.0) / 4.0;

	
	// colorFrag = vec4(vec3(diffuse), 1.0);
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
    if (diffuse <=0.1) {
        colorFrag = vec4(0.0,0.0,0.0,1.0);
    }
    if (diffuse >=0.6) {
        colorFrag = vec4(1.0,1.0,1.0,1.0);
    }
    // colorFrag = vec4(vec3(diffuse), 1.0);
}
