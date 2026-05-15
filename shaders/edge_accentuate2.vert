//this shader (edge_accentuate.vert and edge_accentuate.frag) emulates how return of the obra dinn accentuate edges by coloring fragments using the normal, randomly generated values, and manually set adjustment colors.
// the image rendered by this shader is then processed by a simple post-processing edge detection shader to create the actual edges.
// this vertex shader only needs to pass relevant data to the fragment shader, which will do most of the work
#version 410 core

//TODO COMMENT
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 UV;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
// layout (location = 1) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform float object_id_in;
// uniform vec3 object_pos_in;
uniform float fill_in; // if the object has a complex model, set a solid color for each fragment of this model

// flat out vec3 N;
// out vec3 pos;
out vec4 v_color;
out float object_id;
out float fill;
out vec2 interp_UV;
// out float distance;

uniform int dust;

out vec3 norm_out;
out vec3 camera_dir_out;
out vec3 area_out;
out vec3 object_pos_out;

vec3 getScaleFromMatrix(mat4 m) {
    return vec3(length(m[0].xyz), length(m[1].xyz), length(m[2].xyz));
}

float random(uint x) {
    x = (x ^ 61) ^ (x >> 16);
    x = x * 0x9e3779b9;
    x = x ^ (x >> 14);
    x = x * 0x85ebca6b;
    x = x ^ (x >> 13);
    return float(x & 0xffff) / 65536.0;
}

void main(){
    if (dust == 0){
        interp_UV = UV;
        mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        gl_Position = MVP * vec4(position, 1.0);
    } else {
        // Object space data
        vec3 objPos = position;          // vertex position (sphere radius = 1 usually)
        vec3 objNorm = normalize(normal); // already normalized
        vec3 objTang = normalize(tangent);
        vec3 objBitang = normalize(cross(objNorm, objTang)); // force orthonormal

        // uint seed = uint(gl_VertexID) ^ (uint(gl_InstanceID) << 16);

        // float randRadial = float(seed % 10000) / 10000.0;
        // float randTang   = float((seed * 1597) % 10000) / 10000.0;
        // float randBitang = float((seed * 2861) % 10000) / 10000.0;
        // Random values per instance (or vertex)
        // float randRadial = fract(float(gl_InstanceID) * 13.56788);
        // float randTang   = fract(float(gl_InstanceID) * 29.34567);
        // float randBitang = fract(float(gl_InstanceID) * 46.89123);

        uint id = uint(gl_VertexID) * 1597 + uint(gl_InstanceID);
        float randRadial = random(id);
        float randTang   = random(id + 12345);
        float randBitang = random(id + 67890);
        // Range: radial offset moves inward (0 = at surface, 1 = at center)
        float radialAmount = 1.0-(pow(randRadial, 1.0/3.0));              // 0..1, adjust max if needed
        // Perpendicular deviation range (in object units, sphere radius = 1)
        float devAmount = 0.3;                        // tweak this for cloud thickness

        // Convert to symmetric range [-devAmount/2, devAmount/2]
        float tOffset = (randTang - 0.5) * devAmount;
        float bOffset = (randBitang - 0.5) * devAmount;

        // New object space position
        vec3 newObjPos = objPos
                        - objNorm * radialAmount      // move inward
                        + objTang * tOffset           // random tangent
                        + objBitang * bOffset;        // random bitangent

        // Transform to clip space as usual
        vec4 worldPos = modelMatrix * vec4(newObjPos, 1.0);
        gl_Position = projectionMatrix * viewMatrix * worldPos;
    }

    // // object position in world space
    // vec3 object_pos = (modelMatrix * vec4(0.0,0.0,0.0,1.0)).xyz; 
    // object_pos_out = object_pos;

    // vec3 area = vec3(100.0,100.0,100.0); // you gotta make due with what you got
    // area_out = area;

    // // camera direction transformed by view matrix's rotation part
    // // (from dev's unity code)
    // vec3 camera_dir = mat3(viewMatrix) * vec3(0.0,0.0,1.0);
    // camera_dir_out = camera_dir;

    // // world space normal
    // vec3 norm = (modelMatrix * vec4(normal,0.0)).xyz;
    // norm_out = norm;
    // // norm *= vertex_color_r (we dont have that with .obj files!)


    // object_id = object_id_in;
    // fill = fill_in;
}
