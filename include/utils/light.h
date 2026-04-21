#pragma once

#include <variant>
#include <map>
#include <utils/shader.h>


// THIS IS VERY MESSY AND WILL BE CLEANED UP !!!!
// THIS IS VERY MESSY AND WILL BE CLEANED UP !!!!
// THIS IS VERY MESSY AND WILL BE CLEANED UP !!!!
// THIS IS VERY MESSY AND WILL BE CLEANED UP !!!!

class Light {
public:
    virtual ~Light() = default;
    virtual void set_shader_uniforms(Shader* shader) const = 0;
    virtual glm::mat4 get_light_space_matrix() const = 0;
    // glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity{1.0f};
    bool casts_shadows{true};
    GLuint shadow_map_fb;
    GLuint shadow_map_depth_map;
    GLuint shadow_map_tex;
    Shader shadow_map_shader;
    int shadow_map_resolution{2048};
    glm::mat4 light_space_matrix{1.0f};
    
    Light(Shader shadow_map_shader): shadow_map_shader(shadow_map_shader){
        create_buffers();
    }
    void create_buffers() {
        glGenFramebuffers(1, &shadow_map_fb);
        glGenTextures(1, &shadow_map_depth_map);
        glBindTexture(GL_TEXTURE_2D, shadow_map_depth_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_map_resolution, shadow_map_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_depth_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;
    float range{30.0f}; 
    DirectionalLight(const glm::vec3& direction, float range, Shader shadow_map_shader) : Light(shadow_map_shader), direction(direction), range(range) {
        set_direction(direction);
    }
    void set_shader_uniforms(Shader* shader) const override {
        shader->set_uniform3fv("directionalLightDirection", direction);
        shader->set_uniform1f("directionalLightIntensity", intensity);
        glActiveTexture(GL_TEXTURE0);
        // maybe should be a ttexture o rs oemting
        glBindTexture(GL_TEXTURE_2D, shadow_map_depth_map);
        shader->set_uniform1i("shadowMap", 0);
        shader->set_uniformMatrix4fv("lightSpaceMatrix", light_space_matrix);
    }
    glm::mat4 get_light_space_matrix() const override {
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