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
	colorFrag = vec4(vec3(diffuse), 1.0);
}
