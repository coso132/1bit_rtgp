// //this shader (edge_accentuate.vert and edge_accentuate.frag) emulates how return of the obra dinn accentuate edges by coloring fragments using the normal, randomly generated values, and manually set adjustment colors.
// // the image rendered by this shader is then processed by a simple post-processing edge detection shader to create the actual edges.
// // this fragment shader creates flatshaded accentuated colors to make edges more visible.
#version 410 core

out vec4 colorFrag;
flat in vec3 N; // normal (should be flat per face)
in vec3 pos; // view space coordinates of entire object (should be flat per face)
in float object_id; // unique integer ID per mesh/object
in float fill;// if the object has a complex model, set a solid color for each fragment of this model
in float distance;

void main() {
    float floatynumber = 3.3; // just a random float to use in the randomization
    float floatynumber2 = 2.7; // just a random float to use in the randomization
    float f = 1.0 - fill;
    vec3 p = 1+ (pos+100.0)/100.0; 
    vec3 n = abs(normalize(N)) + 1;
    // p += vec3(0.0001);
    // n += vec3(0.0001);
    // p *= 0.4;
    // n *= 0.4;
    // n = 3 + fract(n);
    // p = fract(p);
    n = n *f; // neutralize effect of normal on complex objects
    // n = n *0.6;
    // p = p *f;

    // randomize color channels based on normal, position, and object_id
    // vec3 color = vec3(1.0);
    // color *= n;
    // color *= (1+ object_id) * p.y;
    // color.r *= (1+ object_id) * p.x * n.z;
    // color.g *= (1+ object_id) * n.y * p.z;
    // color = fract(vec3((fract(color.r) * 1.1),(fract(color.g) * 2.2),(fract(color.b) * 3.3)));
    float r = ((fract(n.x * 2.179 + p.x * 2.1123 + n.z * 2.137)/ floatynumber2 ) * floatynumber);
    float g = ((fract(n.z * 2.134 + p.z * 2.1478 + object_id* floatynumber2) / floatynumber2 ) * floatynumber);
    float b = ((fract(p.y * 2.122374 + n.y * 2.1398 + p.z * 2.199+ object_id* floatynumber2)/floatynumber) * floatynumber);

    vec3 color = fract(vec3(r, g, b)*33.3333333333);

    
    colorFrag = vec4(vec3(color), 1.0);
}
