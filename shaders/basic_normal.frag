#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 N;

// the transformed light direction has been calculated per-vertex in the vertex shader
in vec3 L;

void main(){
	// we use the normal as a color
	// final color (0,1) black and white
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	float ambient = 0.1;
	float diffuse = smoothstep(0.0, 0.4, dot(n,l)) * 0.9 + ambient;
	// quantize diffuse in 4 different possible light levels
	// diffuse = floor(diffuse * 3.0) / 3.0;

	
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
}
