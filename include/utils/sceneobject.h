#pragma once

#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>

enum Material {
    SIMPLE,
    COMPLEX,
    SOMETHING_ELSE,
};


// data structure for scene objects
class Object {
public:
    // world coordinates
    glm::vec3 pos;
    glm::mat4 model_matrix;
    // modea of the object
    // shader program to render this object with
    // Shader shader;//not needed?
    Material material;
    Model model;

    //constructor takes position, string for model filepath, and material
    Object(glm::vec3 pos, const string& filepath, Material material) 
        : pos(pos), material(material), model(filepath) {
        this->model_matrix = glm::translate(glm::mat4(1.0f), this->pos);
    }

    Object(glm::vec3 pos, const string& filepath, Material material, float scale) 
        : pos(pos), material(material), model(filepath) {
        this->model_matrix = glm::translate(glm::mat4(1.0f), this->pos);
        this->model_matrix = glm::scale(this->model_matrix, glm::vec3(scale));
    }
    // Delete copy semantics - Object is MOVE-ONLY
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    // Move semantics
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    void update_position(glm::vec3 new_pos) {
        this->pos = new_pos;
        this->model_matrix = glm::translate(glm::mat4(1.0f), this->pos);
    }
};

