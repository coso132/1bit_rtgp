// //this shader (edge_accentuate.vert and edge_accentuate.frag) emulates how return of the obra dinn accentuate edges by coloring fragments using the normal, randomly generated values, and manually set adjustment colors.
// // the image rendered by this shader is then processed by a simple post-processing edge detection shader to create the actual edges.
// // this fragment shader creates flatshaded accentuated colors to make edges more visible.
// #version 410 core

// // output shader variable
// out vec4 colorFrag;

// // the transformed normal has been calculated per-vertex in the vertex shader
// in vec3 N;
// in float object_id;

// void main(){
// 	// generate a random color based on the object_id
// 	float r = fract(sin(object_id * 1.2) * 43758);
// 	float g = fract(sin(object_id * 7.8) * 43.758);
// 	float b = fract(sin(object_id * 3.9) * 43.758);
// 	// vec3 random_color = normalize(vec3(r, g, b) * 2.0 - 1.0) * 0.5 + 0.5; // normalize to range [0,1]
// 	vec3 random_color = vec3(r, g, b);
// 	// accentuate edges by using the normal to darken the color
// 	vec3 n = normalize(N);
// 	float edge_accentuation = smoothstep(0.0, 0.4, length(n)) ; // adjust the parameters as needed
// 	vec3 final_color = random_color * edge_accentuation;

// 	colorFrag = vec4(final_color, 1.0);
// }

#version 410 core

out vec4 colorFrag;
flat in vec3 N;           // normal (should be flat per face)
in vec3 pos;           // view space coordinates of entire object (should be flat per face)
in float object_id;  // unique integer ID per mesh/object
in float fill;

void main() {
    float floatynumber = 3.023; // just a random float to use in the randomization
    float floatynumber2 = 2.738; // just a random float to use in the randomization
    float f = 1.0 - fill;
    vec3 p = abs(pos)*3.2; 
    vec3 n = normalize(N)+1.2; // make normal values positive to avoid issues with sin() and fract()
    n = n *f;
    // p = p *f;
    // p += vec3(0.0001);
    // n += vec3(0.0001);

    // randomize red channel based on normal 
    float r = fract((fract(n.x * 0.17986 + p.x * 0.1123457 + n.z * 0.13798)* floatynumber2 ) * floatynumber);
    // randomize green channel based on object_id
    float g = fract((fract(n.z * 0.1348792 + p.z * 0.1478963 + object_id* floatynumber2) * floatynumber2 ) * floatynumber);
    // randomize blue channel based on position
    float b = fract((fract(p.y * 0.122374 + n.y * 0.1398712 + p.z * 0.1998636+ object_id* floatynumber2)*floatynumber) * floatynumber);

    vec3 color = vec3(r, g, b);
    
    colorFrag = vec4(clamp(color, 0.0, 1.0), 1.0);
}
