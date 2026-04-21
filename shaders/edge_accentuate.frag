// //this shader (edge_accentuate.vert and edge_accentuate.frag) emulates how return of the obra dinn accentuate edges by coloring fragments using the normal, randomly generated values, and manually set adjustment colors.
// // the image rendered by this shader is then processed by a simple post-processing edge detection shader to create the actual edges.
// // this fragment shader creates flatshaded accentuated colors to make edges more visible.
#version 410 core

out vec4 colorFrag;
flat in vec3 N; // normal (should be flat per face)
in vec3 pos; // view space coordinates of entire object (should be flat per face)
in float object_id; // unique integer ID per mesh/object
in float fill;// if the object has a complex model, set a solid color for each fragment of this model

void main() {
    float floatynumber = 3.023; // just a random float to use in the randomization
    float floatynumber2 = 2.738; // just a random float to use in the randomization
    float f = 1.0 - fill;
    vec3 p = abs(pos)*3.2; 
    vec3 n = normalize(N)+1.2; // make normal values positive to avoid issues with sin() and fract()
    n = n *f; // neutralize effect of normal on complex objects
    // p = p *f;
    // p += vec3(0.0001);
    // n += vec3(0.0001);

    // randomize color channels based on normal, position, and object_id
    float r = fract((fract(n.x * 0.17986 + p.x * 0.1123457 + n.z * 0.13798)* floatynumber2 ) * floatynumber);
    float g = fract((fract(n.z * 0.1348792 + p.z * 0.1478963 + object_id* floatynumber2) * floatynumber2 ) * floatynumber);
    float b = fract((fract(p.y * 0.122374 + n.y * 0.1398712 + p.z * 0.1998636+ object_id* floatynumber2)*floatynumber) * floatynumber);

    vec3 color = vec3(r, g, b);
    
    colorFrag = vec4(clamp(color, 0.0, 1.0), 1.0);
}
