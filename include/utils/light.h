#pragma once

#include <variant>
#include <map>
#include <utils/shader.h>

class DirectionalLight {
public:
    glm::vec3 direction;
    float range{30.0f}; 
    glm::mat4 light_space_matrix{1.0f};

    DirectionalLight(const glm::vec3& direction, float range) : direction(direction), range(range) {
        set_direction(direction);
    }
    
    glm::mat4 get_light_space_matrix() {
        return light_space_matrix;
    }
    void set_direction(const glm::vec3& new_direction) {
        direction = glm::normalize(new_direction);
        //change light space matrix accordingly using direction and range
        glm::mat4 light_projection = glm::ortho(-range, range, -range, range, 0.01f, range*2.0f);
        glm::mat4 light_view = glm::lookAt(-direction * range, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        light_space_matrix = light_projection * light_view;
        
    }
    void rotate(float angle_degrees, const glm::vec3& axis) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle_degrees), axis);
        set_direction(glm::normalize(glm::vec3(rotation * glm::vec4(direction, 0.0f))));
    }

};

class PointLight {
public:
    glm::vec3 position{0.0f,0.0f,0.0f};
    float intensity{10.0f};
    PointLight(const glm::vec3& position, float intensity): position(position), intensity(intensity) {}

    void set_shader_uniforms(Shader* shader, string dli_string, string pos_string) {
        shader->set_uniform3fv(pos_string, position);
        shader->set_uniform1f(dli_string, intensity);
    }
    // glm::mat4 get_light_space_matrix() const override {
    //     return light_space_matrix;
    // }
    // void set_direction(const glm::vec3& new_direction) {
    //     direction = glm::normalize(new_direction);
    //     //change light space matrix accordingly using direction and range
    //     glm::mat4 light_projection = glm::ortho(-range, range, -range, range, 0.01f, range*2.0f);
    //     glm::mat4 light_view = glm::lookAt(-direction * range, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //     light_space_matrix = light_projection * light_view;
        
};