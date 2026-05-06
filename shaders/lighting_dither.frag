#version 410 core

out vec4 colorFrag;

in vec3 N;
in vec3 L;
in vec4 FragPosLightSpace;
in vec2 interp_UV;

uniform sampler2D shadowMap;

uniform sampler2D tex;
uniform float textured;
uniform float repeat;

uniform mat4 viewMatrix;

uniform sampler2D dither_map;
uniform float width;
uniform float height;

#define MAX_POINT_LIGHTS 8
in vec4 mv;
uniform int numPointLights;
uniform vec3 PointLightPos[MAX_POINT_LIGHTS];
uniform float pointLightIntensity[MAX_POINT_LIGHTS];

float calculateShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
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

float dither(float diffuse, sampler2D map, float width, float height){
    float min =0.2;
    float max =0.8;
    if (diffuse <=min){
        return 0.0;}
    if (diffuse >=max){
        return 1.0;}
    vec2 xy = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
    float threshold = (texture(map, xy)).r;
    float finalIntensity = (diffuse > threshold) ? 1.0 : 0.0;
    return finalIntensity;
}

void main(){
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
        tex_value = length(surfaceColor.rgb);
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
	float ambient = 0.2;
    float rawDiffuse = (ambient + dirLightContrib + pointLightContrib);
    rawDiffuse = smoothstep(0.3,0.8,rawDiffuse);
    float diffuse = rawDiffuse;
    // tex_value = floor(tex_value * 3.0)/3.0;
    lighting_value = diffuse * tex_value;
    lighting_value = floor(lighting_value * 3.0)/3.0;
    // tex_value = floor(tex_value * 3.0)/3.0;
    lighting_value = dither(diffuse * tex_value, dither_map, width, height);

    red = lighting_value;
    green = tex_value;
    blue = 0.0;
    colorFrag = vec4(red,green,blue,1.0);
}
