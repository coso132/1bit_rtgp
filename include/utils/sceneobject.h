#pragma once

#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/misc.h>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// TODO EXPLAIN WHOLE FILE GANG
enum Material {
    SIMPLE,
    COMPLEX,
    DUST,
    DITHER_SPHERE,  // unused
    SOMETHING_ELSE, // unused
};
enum NoiseType{
    BAYER,
    BLUE_NOISE,
    NONE,   // unused
};
// different possible render passes
// TODO I REALLY DO NOT THINK THIS SHOULD BE HERE AT THIS POINT IS THIS EVEN USEFULE
enum RenderMode {
    LIGHTING,
    EDGE_ACCENTUATION,
    EDGE_ACCENTUATION2,
    WIREFRAME,
    SPHERE_CUBEMAP,
    SHADOWMAP
};

// data structure for scene objects
class Object {
public:
    // world coordinates
    glm::vec3 pos;
    glm::mat4 model_matrix;
    Material material;
    GLuint texture;
    GLuint normal_texture;
    bool textured{false};
    Model model;
    NoiseType noise_type{BAYER};
    float scale{1.0f};

    Object(glm::vec3 pos, const string& model_filepath, Material material, const char* texture_filepath, NoiseType noise_type, float scale, glm::vec3 rotate = glm::vec3(0.0f,1.0f,0.0f), float radians=0.f) 
        : pos(pos), material(material), model(model_filepath), noise_type(noise_type) {
        LoadTexture(texture_filepath);
        this->model_matrix = glm::translate(glm::mat4(1.0f), this->pos);
        this->model_matrix = glm::scale(this->model_matrix, glm::vec3(scale));
        this->model_matrix = glm::rotate(this->model_matrix, radians, rotate);
        this->scale = scale;
    }
    Object(glm::vec3 pos, const string& filepath, Material material) 
        : pos(pos), material(material), model(filepath) {
        texture = load_image("textures/white.png");
        this->model_matrix = glm::translate(glm::mat4(1.0f), this->pos);
    }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    void set_position(glm::vec3 new_pos) {
        this->pos = new_pos;
        update_model_matrix();
    }

    void update_model_matrix() {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 sca = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        this->model_matrix = trans * sca; 
    }

    void draw(){
        if (material == DUST)
            this->model.DrawPoints((int)(6*this->scale));
        else 
            this->model.Draw();
    }
    // should add rotation and other stuff
    void LoadTexture(const char* path){
        textured=true;
        texture = load_image(path);
    }
    // should add rotation and other stuff
    void LoadNormalTexture(const char* path){
        textured=true;
        normal_texture = load_image(path);
    }


};

