#version 410 core

out vec4 colorFrag;

in vec3 N;
in vec3 L;
in vec4 FragPosLightSpace;
in vec2 interp_UV;

uniform sampler2D shadowMap;
uniform sampler2D tex;
uniform samplerCube blue_noise;
uniform samplerCube bayer_noise;
uniform mat4 viewMatrix;


uniform float textured;
uniform float repeat;
uniform int noise_type;

uniform float patternScaleBlue = 24.0;
uniform float patternScaleBayer = 48.0;
uniform vec3 cameraWorldPos;

#define MAX_POINT_LIGHTS 8
in vec4 mv;
in vec3 FragWorldPos;
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

float noise_sphere_dither(float diffuse, samplerCube noise, float patternScale){
    float min =0.05;
    float max =0.9;
    if (diffuse <=min){
        return 0.0;}
    if (diffuse >=max){
        return 1.0;}

    vec3 viewDir = normalize(FragWorldPos - cameraWorldPos);
    float threshold = texture(noise, viewDir).r;
    float finalValue = step(threshold, diffuse); 

    return finalValue;

    // float phi = atan(viewDir.y, viewDir.x);   
    // float theta = acos(viewDir.z);            
    // float PI = 3.14159;
    // float u = (phi + PI) / (2.0*PI);
    // float v = (sin(theta) + 1.0) / 2.0;  
    // vec2 uv = vec2(u,v);//

    // // Compute how fast UV changes across screen pixels
    // vec2 duv_dx = dFdx(uv);
    // vec2 duv_dy = dFdy(uv);
    
    // // Estimate the UV-space radius of a pixel's footprint
    // vec2 duv = 0.5 * (abs(duv_dx) + abs(duv_dy));
    // float footprint = length(duv);  // rough measure
    
    // // Number of samples: adaptive or fixed (we'll do fixed 4 samples for clarity)
    // const int samples = 4;
    // vec2 offsets[4];
    // offsets[0] = vec2(-0.25, -0.25);
    // offsets[1] = vec2( 0.25, -0.25);
    // offsets[2] = vec2(-0.25,  0.25);
    // offsets[3] = vec2( 0.25,  0.25);
    
    // float ditherSum = 0.0;
    // for (int i = 0; i < samples; i++) {
    //     // Sample the dither pattern at slightly offset UVs (scaled by footprint)
    //     vec2 sampleUV = uv + offsets[i] * footprint * patternScale;
    //     // Alternatively, multiply footprint by patternScale because UVs are scaled later
    //     // Actually better: apply scaling after offset, or scale footprint
    //     float ditherValue = texture(noise, sampleUV * patternScale).r;
    //     ditherSum += ditherValue;
    // }
    // float ditherValue = ditherSum / float(samples);
    
    // float threshold = ditherValue;
    // // float finalLuma = dot(vec3(diffuse), vec3(0.2126, 0.7152, 0.0722)); 
    // float finalLuma = diffuse;
    // float finalValue = step(threshold, finalLuma); 

    // return finalValue;
}



void main(){
    // vec3 viewDir = normalize(FragWorldPos - cameraWorldPos);
    // float ditherValue = texture(blue_noise, viewDir).r;
    // colorFrag = vec4(vec3(ditherValue),1.0);
    // return;

    float red = 1.0;  //lighting
    float green = 1.0; //texture
    float blue = 0.0; // dust?
    float lighting_value = 0.0;
    float tex_value = 1.0;

	vec3 n = normalize(N);
	vec3 l = normalize(L);

    if(textured>=0.9){
        vec2 repeated_UV = mod(interp_UV*repeat, 1.0);
        vec4 surfaceColor = texture(tex, repeated_UV);
        // colorFrag = surfaceColor;
        // return;
        tex_value = length(surfaceColor.rgb);
        // tex_value = floor(tex_value * 4.0)/4.0;
    }

    //directional light shadowmap
    float shadowFactor=calculateShadow(FragPosLightSpace);
    shadowFactor = smoothstep(0.3,0.6,shadowFactor);
    //directional light diffuse
    float diffDir = max(dot(n,l),0.0);
    float dirLightContrib = diffDir * shadowFactor;

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
    rawDiffuse = smoothstep(0.2,0.8,rawDiffuse);
    float diffuse = rawDiffuse;
    lighting_value = diffuse * tex_value + 0.1;
    lighting_value = floor(lighting_value * 3.0)/3.0;
    // tex_value = floor(tex_value * 3.0)/3.0;
    
    if (noise_type == 0){
        // lighting_value = bayer_dither(diffuse * tex_value);
        lighting_value = noise_sphere_dither(lighting_value, bayer_noise, patternScaleBayer);
        // tex_value = bayer_dither(tex_value);
    }if (noise_type == 1){
        lighting_value = noise_sphere_dither(lighting_value, blue_noise, patternScaleBlue);
        // tex_value = blue_noise_dither(tex_value);
    }
    red = lighting_value;
    green = tex_value;
    blue = 0.0;
    colorFrag = vec4(red,green,blue,1.0);
}
