#version 410 core

out vec4 colorFrag;

in vec3 N;
in vec3 L;
in vec4 FragPosLightSpace;
in vec2 interp_UV;

uniform sampler2D shadowMap;
uniform sampler2D tex;
uniform sampler2D blue_noise;
uniform mat4 viewMatrix;


uniform float textured;
uniform float repeat;
uniform int noise_type;

#define MAX_POINT_LIGHTS 8
in vec4 mv;
uniform int numPointLights;
uniform vec3 PointLightPos[MAX_POINT_LIGHTS];
uniform float pointLightIntensity[MAX_POINT_LIGHTS];
// in vec3 pointL;
// in vec3 positionView;

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
    float bias_min = 0.0010;
    float bias_max = 0.010;
    float bias = max(bias_max * (1.0 - dot(normalize(N), normalize(L))), bias_min);
    
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

vec4 bayer_dither(float diffuse){
    float min =0.2;
    float max =0.8;
    if (diffuse <=0.2) {
        return vec4(0.0,0.0,0.0,1.0);}
    if (diffuse >=max) {
        return vec4(1.0,1.0,1.0,1.0);}
    // BAYER NOISE
    // 4x4 Bayer pattern (normalised to [0,1])
    const float bayer[16] = float[](
        0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
        12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
        3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
        15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
    );
    int x = int(mod(gl_FragCoord.x, 4.0));
    int y = int(mod(gl_FragCoord.y, 4.0));
    int idx = y * 4 + x;
    float threshold = bayer[idx];
    float finalIntensity = (diffuse > threshold) ? 1.0 : 0.0;
    return vec4(vec3(finalIntensity), 1.0);
}


vec4 blue_noise_dither(float diffuse){
    float min =0.2;
    float max =0.8;
    if (diffuse <=min){
        return vec4(0.0,0.0,0.0,1.0);}
    if (diffuse >=max){
        return vec4(1.0,1.0,1.0,1.0);}
    float threshold = (texture(blue_noise, gl_FragCoord.xy/64.0)).r;
    float finalIntensity = (diffuse > threshold) ? 1.0 : 0.0;
    return vec4(vec3(finalIntensity), 1.0);
}

void main(){
	vec3 n = normalize(N);
	vec3 l = normalize(L);

    float tex_factor = 1.0;
    if(textured>=0.9){
        vec2 repeated_UV = mod(interp_UV*repeat, 1.0);
        vec4 surfaceColor = texture(tex, repeated_UV);
        // colorFrag = surfaceColor;
        // return;
        tex_factor = length(surfaceColor.rgb);
    }

    //directional light shadowmap
    float shadowFactor=calculateShadow(FragPosLightSpace);
    shadowFactor = smoothstep(0.3,0.6,shadowFactor);
    //directional light diffuse
    float diffDir = max(dot(n,l),0.0);
    float dirLightContrib = diffDir * shadowFactor;

    //point light diffuse
    // vec4 lightPos = viewMatrix * vec4(PointLightPos[i],1.0);
    // vec3 lightDir = (lightPos.xyz - mv.xyz);
    // float distance = 2*length(lightDir);
    // lightDir = lightDir/distance;
    // float diffPoint = max(dot(lightDir,n),0.0);
    // // attenuation 
    // float attenuation = pointLightIntensity/(distance*distance + 1.0);
    // float pointLightContrib = diffPoint*attenuation;
    // vec3 positionWorld = (modelMatrix * vec4(position, 1.0)).xyz;

    float pointLightContrib = 0.0;
    for (int i = 0; i < numPointLights; i++) {
        vec4 lightPos = viewMatrix * vec4(PointLightPos[i],1.0);
        vec3 lightDir = (lightPos.xyz - mv.xyz);
        float distance = 2*length(lightDir);
        lightDir = lightDir/distance;
        float diffPoint = max(dot(lightDir,n),0.0);
        // attenuation 
        float attenuation = pointLightIntensity[i]/(distance*distance + 1.0);
        float addAmount = (diffPoint*attenuation);
        // addAmount = smoothstep(0.0,0.6,addAmount);
        pointLightContrib += diffPoint*attenuation;
    }


    // combination
	float ambient = 0.1;
    float rawDiffuse = (ambient + dirLightContrib + pointLightContrib);
    // float diffuse = rawDiffuse * tex_factor;
    // rawDiffuse = floor(rawDiffuse * 3.0)/3.0;
    rawDiffuse = smoothstep(0.2,0.8,rawDiffuse);
    // rawDiffuse = clamp(rawDiffuse, 0.0, 1.0);
    float diffuse = rawDiffuse * tex_factor;
    diffuse = floor(diffuse * 4.0)/4.0;
    
    if (noise_type == 0){
        colorFrag = bayer_dither(diffuse);}
    if (noise_type == 1){
        colorFrag = blue_noise_dither(diffuse);}
    // colorFrag = vec4(vec3(finalIntensity), 1.0);
    // debugging purposes
    // colorFrag = vec4(vec3(diffuse), 1.0);
    // colorFrag = vec4(vec3(shadowFactor), 1.0);
}
