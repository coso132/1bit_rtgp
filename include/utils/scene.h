#pragma once

// using namespace std;

// Std. Includes
// #include <vector>
#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/light.h>
#include <utils/sceneobject.h>
// #include <glm/glm.hpp>
// #include <matrix_inverse.hpp>
using UniformValue = std::variant<float, glm::vec2, glm::vec3, glm::mat3, glm::mat4>;

/////////////////// SCENE class ///////////////////////
class Scene {
public:
    static Scene load_test_scene();

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    static constexpr float FOV = 45.0f;
    static constexpr float ASPECT_RATIO = 16.0f/9.0f;
    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float FAR_PLANE = 10000.0f;
    static glm::mat4 projection_matrix;

    // Shader edge_accentuation_shader = ;
    Shader edge_accentuation_shader = Shader("shaders/edge_accentuate.vert","shaders/edge_accentuate.frag");
    Shader lighting_shader = Shader("shaders/basic_normal.vert","shaders/basic_normal.frag");
    vector<Object> objects;
    Shader* current_shader;
    Camera camera;
    // vector<Light> lights;
    DirectionalLight light;

    enum RenderMode {
        LIGHTING,
        EDGE_ACCENTUATION,
        SHADOWMAP
    };

    Scene(vector<Object>&& objects, Camera camera, DirectionalLight light)
    :objects(std::move(objects)), camera(camera), light(light){
    }
    // rendering of the whole scene
    void full_render(const std::map<std::string, UniformValue>& uniform_values, RenderMode mode, GLuint buffer, int render_width, int render_height) {
        if (mode == LIGHTING)// render shadowmap first
            full_render({}, SHADOWMAP, light.shadow_map_fb,light.shadow_map_resolution,light.shadow_map_resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (mode == LIGHTING){
            current_shader = &lighting_shader;
            current_shader->Use();
            light.set_shader_uniforms(current_shader);
        } else if (mode == SHADOWMAP){
            current_shader = &light.shadow_map_shader;
            current_shader->Use();
            glm::mat4 light_space_matrix = light.get_light_space_matrix();
            current_shader->set_uniformMatrix4fv("lightSpaceMatrix", light_space_matrix);
        } else if (mode == EDGE_ACCENTUATION){
            current_shader = &edge_accentuation_shader;
            current_shader->Use();
        }
        // set all the custom uniform values in the shader
        /*for (const auto& [name, value] : uniform_values) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, float>)
                    current_shader->set_uniform1f(name, arg);
                else if constexpr (std::is_same_v<T, glm::vec2>)
                    current_shader->set_uniform2fv(name, arg);
                else if constexpr (std::is_same_v<T, glm::vec3>)
                    current_shader->set_uniform3fv(name, arg);
                else if constexpr (std::is_same_v<T, glm::mat3>)
                    current_shader->set_uniformMatrix3fv(name, arg);
                else if constexpr (std::is_same_v<T, glm::mat4>)
                    current_shader->set_uniformMatrix4fv(name, arg);
            }, value);
        }*/
        // render objects
        for (GLuint i = 0; i < objects.size(); i++) {
            Object* object = &objects[i];
            if (mode == EDGE_ACCENTUATION) {
                if (objects[i].material == COMPLEX)
                    {current_shader->set_uniform1f("fill_in", 1.0f);}
                else 
                    {current_shader->set_uniform1f("fill_in", 0.0f);}
                current_shader->set_uniform1f("object_id_in", (float)(i+1));
                current_shader->set_uniform1f("object_id_in", (float)(i+1));
                current_shader->set_uniform3fv("object_pos_in", objects[i].pos);
            }
            glm::mat4 model_matrix = object->model_matrix;
            current_shader->set_uniformMatrix4fv("modelMatrix", object->model_matrix);
            if (mode != SHADOWMAP){
                glm::mat4 view_matrix = camera.GetViewMatrix();
                current_shader->set_uniformMatrix4fv("viewMatrix", view_matrix);
                glm::mat4 projection_matrix = this->projection_matrix;
                current_shader->set_uniformMatrix4fv("projectionMatrix", projection_matrix);
                glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
                current_shader->set_uniformMatrix3fv("normalMatrix", normal_matrix);
            }
            object->model.Draw();
        }
    }
private:

};

