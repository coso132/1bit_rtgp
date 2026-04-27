#pragma once

#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/misc.h>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

enum Material {
    SIMPLE,
    COMPLEX,
    SOMETHING_ELSE,
};
enum NoiseType{
    BAYER,
    BLUE_NOISE,
};
// different possible render passes
enum RenderMode {
    LIGHTING,
    EDGE_ACCENTUATION,
    EDGE_ACCENTUATION2,
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
    float scale;

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
    // Delete copy semantics - Object is MOVE-ONLY
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    // Move semantics
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    void set_position(glm::vec3 new_pos) {
        this->pos = new_pos;
        update_model_matrix();
    }

    void update_model_matrix() {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
        // glm::mat4 rot = glm::mat4_cast(orientation); // if using quaternion
        glm::mat4 sca = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        this->model_matrix = trans * sca; // world translation first (leftmost)
    }

    void draw(RenderMode mode, Shader* shader){
        if (mode == LIGHTING){
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture);
            shader->set_uniform1i("tex",1);
            shader->set_uniform1f("repeat",1.0);
            shader->set_uniform1f("textured",1.0);
            shader->set_uniform1i("noise_type",noise_type);
        } else if (mode == EDGE_ACCENTUATION2) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normal_texture);
            shader->set_uniform1i("normal_tex",1);
            shader->set_uniform1f("repeat",1.0);
            shader->set_uniform1f("textured",1.0);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
            shader->set_uniform1f("textured",0.0);
        }
        this->model.Draw();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // should add rotation and other stuff
    void LoadTexture(const char* path){
        textured=true;
        // GLuint textureImage;
        int w, h, channels;
        unsigned char* image;
        image = stbi_load(path, &w, &h, &channels, STBI_rgb);

        if (image == nullptr)
            std::cout << "Failed to load texture!" << std::endl;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // 3 channels = RGB ; 4 channel = RGBA
        if (channels==3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels==4)
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        // we set how to consider UVs outside [0,1] range
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // we set the filtering for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

        // we free the memory once we have created an OpenGL texture
        stbi_image_free(image);

        // we set the binding to 0 once we have finished
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // should add rotation and other stuff
    void LoadNormalTexture(const char* path){
        textured=true;
        // GLuint textureImage;
        int w, h, channels;
        unsigned char* image;
        image = stbi_load(path, &w, &h, &channels, STBI_rgb);

        if (image == nullptr)
            std::cout << "Failed to load texture!" << std::endl;

        glGenTextures(1, &normal_texture);
        glBindTexture(GL_TEXTURE_2D, normal_texture);
        // 3 channels = RGB ; 4 channel = RGBA
        if (channels==3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels==4)
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        // we set how to consider UVs outside [0,1] range
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // we set the filtering for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

        // we free the memory once we have created an OpenGL texture
        stbi_image_free(image);

        // we set the binding to 0 once we have finished
        glBindTexture(GL_TEXTURE_2D, 0);
    }


};

