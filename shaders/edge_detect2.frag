#version 410 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler2D wireframeTexture;

//TODO COMMENT
uniform sampler2D dust;
uniform sampler2D blue_noise;

uniform float depthThreshold = 0.05;
uniform float normalThreshold = 0.1;
// edge detection kernel 
// const float kernel[9] = float[](
//     -1, -1, -1,
//     -1,  8, -1,
//     -1, -1, -1
// );
float linearizeDepth(float depth) {
    float near  = 0.02;
    float far  = 1.0;
    // Convert to NDC: [0,1] -> [-1,1]
    float z_ndc = depth * 2.0 - 1.0;
    // Perspective projection: z_view = (2.0 * near * far) / (far + near - z_ndc * (far - near))
    return (2.0 * near * far) / (far + near - z_ndc * (far - near));
}
float getDepth(vec2 tc){
    // return 1- (texture(depthTexture,tc).r);
    return 1-linearizeDepth(texture(depthTexture,tc).r);
}


void main() {
    vec3 bluenoise = texture(wireframeTexture,TexCoord).rgb;
    if (bluenoise.b >=1.0 && bluenoise.r <=0.0 && bluenoise.g <=0.0){
        FragColor = vec4(1.0,1.0,1.0,1.0);
        return;
    }
    vec2 texelSize = 1.0/textureSize(depthTexture,0);
    float centerDepth = getDepth(TexCoord);
    // FragColor = vec4(vec3(centerDepth),1.0);
    // return;
    vec3 centerNormal = texture(colorTexture, TexCoord).rgb;
    int off_num = 4;

    bool edge[4];
    vec2 offsets[4] = vec2[](
        vec2(0.0, texelSize.y),
        vec2(0.0, -texelSize.y),
        vec2(texelSize.x, 0.0),
        vec2(-texelSize.x, 0.0)
    );

    for (int i = 0; i < 4; i++) {
        vec2 uv = TexCoord + offsets[i];

        float d = getDepth(uv);
        float depthDiff = (centerDepth-d);

        vec3 n = texture(colorTexture, uv).rgb;
        float normalDiff = length(centerNormal- n);

        edge[i] = false;
        // if (depthDiff > depthThreshold || false) {
        if ((depthDiff > depthThreshold)) {
            edge[i] = true;
            break;
        } else if (normalDiff > normalThreshold) {
            edge[i] = true;
            break;
        }
    }
    vec4 wf_color = texture(wireframeTexture,TexCoord);
    if (((edge[3] && !edge[1])||(edge[0] && !edge[2]))){
    // if ((edge[0] && !edge[2]) && wf_color.r >0.5 ){
    // if ((edge[0] || edge[1] || edge[2] || edge[3]) && wf_color.r >0.5 ){
        FragColor = vec4(vec3(1.0),1.0);}
    else{
        FragColor = vec4(vec3(0.0),1.0);}
}