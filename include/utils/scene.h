#pragma once

#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/light.h>
#include <utils/sceneobject.h>
#include <utils/misc.h>

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
    Shader edge_accentuation2_shader = Shader("shaders/edge_accentuate2.vert","shaders/edge_accentuate2.frag");
    Shader lighting_shader = Shader("shaders/lighting_dither.vert","shaders/lighting_dither.frag");
    vector<Object> objects;
    Shader* current_shader;
    Camera camera;
    DirectionalLight directional_light; // singular directional light for now
    vector<PointLight> point_lights; // multiple point lights
    
    // blue noise texture
    GLuint blue_noise;

    

    // constructor
    Scene(vector<Object>&& objects, Camera camera, DirectionalLight light, vector<PointLight> point_lights)
    :objects(std::move(objects)), camera(camera), directional_light(light), point_lights(point_lights){
        blue_noise = load_image("textures/blue_noise.png");
    }


    // rendering of the whole scene based on render mode
    void full_render(const std::map<std::string, UniformValue>& uniform_values, RenderMode mode, GLuint buffer, int render_width, int render_height) {
        if (mode == LIGHTING)// if lighting needs to be calculated then render shadowmap first
            full_render({}, SHADOWMAP, directional_light.shadow_map_fb,directional_light.shadow_map_resolution,directional_light.shadow_map_resolution);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (mode == LIGHTING){
            current_shader = &lighting_shader;
            current_shader->Use();
            directional_light.set_shader_uniforms(current_shader);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, blue_noise);
            current_shader->set_uniform1i("blue_noise",2);
            int nl = point_lights.size();
            current_shader->set_uniform1i("numPointLights",nl);
            for (int i = 0; i < nl; i++){
                string si = to_string(i);
                string pli = ("pointLightIntensity[" + si + ']').c_str();
                string plp =("PointLightPos["+si+']').c_str();
                point_lights[i].set_shader_uniforms(current_shader, pli, plp);
            }
            // shader->set_uniform1f("textured",1.0);
        } else if (mode == SHADOWMAP){
            current_shader = &directional_light.shadow_map_shader;
            current_shader->Use();
            glm::mat4 light_space_matrix = directional_light.get_light_space_matrix();
            current_shader->set_uniformMatrix4fv("lightSpaceMatrix", light_space_matrix);
        } else if (mode == EDGE_ACCENTUATION ){
            current_shader = &edge_accentuation_shader;
            current_shader->Use();
        }else if (mode == EDGE_ACCENTUATION2 ){
            current_shader = &edge_accentuation2_shader;
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
                // current_shader->set_uniform1f("object_id_in", (float)(i+1));
                // current_shader->set_uniform3fv("object_pos_in", objects[i].pos);
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
            object->draw(mode, current_shader);
        }
    }
private:

};

Scene load_test_scene();
Scene load_cottage1_scene();
Scene load_cottage2_scene();