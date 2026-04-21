#pragma once

#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/light.h>
#include <utils/sceneobject.h>

using UniformValue = std::variant<bool, int, float, glm::vec2, glm::vec3, glm::mat3, glm::mat4>;

// alot of things are done here
/////////////////// SCENE class ///////////////////////
class Scene {
public:
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    static constexpr float FOV = 45.0f;
    static constexpr float ASPECT_RATIO = 16.0f/9.0f;
    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float FAR_PLANE = 10000.0f;
    static glm::mat4 projection_matrix;

    // to clean up, this should be in main
    Shader edge_accentuation_shader = Shader("shaders/edge_accentuate.vert","shaders/edge_accentuate.frag");
    Shader lighting_shader = Shader("shaders/lighting_dither.vert","shaders/lighting_dither.frag");
    vector<Object> objects;
    Shader* current_shader;
    Camera camera;
    // vector<Light> lights; // multiple lights not implemented yet
    DirectionalLight light; // singular directional light for now

    // different possible render passes
    enum RenderMode {
        LIGHTING,
        EDGE_ACCENTUATION,
        SHADOWMAP
    };

    // constructor
    Scene(vector<Object>&& objects, Camera camera, DirectionalLight light)
    :objects(std::move(objects)), camera(camera), light(light){}


    // rendering of the whole scene based on render mode
    void full_render(const std::map<std::string, UniformValue>& uniform_values, RenderMode mode, GLuint buffer, int render_width, int render_height) {
        if (mode == LIGHTING)// if lighting needs to be calculated then render shadowmap first
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
        // render objects
        for (GLuint i = 0; i < objects.size(); i++) {
            Object* object = &objects[i];
            // if we are accentuating edges we need to feed the shader the object_id, position, and whether it should be filled in or not
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

Scene load_test_scene();