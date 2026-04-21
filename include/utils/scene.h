#pragma once

// using namespace std;

// Std. Includes
// #include <vector>
#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
// #include <glm/glm.hpp>
// #include <matrix_inverse.hpp>

enum Material {
    SIMPLE,
    COMPLEX,
    SOMETHING_ELSE,
};
// enum LightType {
//     DIRECTIONAL,
//     POINT,
//     SPOT,
// };

using UniformValue = std::variant<float, glm::vec2, glm::vec3, glm::mat3, glm::mat4>;

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
    // std::map<std::string, UniformValue> uniforms;
    int shadow_map_resolution{2048};
    glm::mat4 light_space_matrix{1.0f};
    
    Light(Shader shadow_map_shader): shadow_map_shader(shadow_map_shader){
        create_buffers();
    }
    void create_buffers() {
        /////////////////// CREATION OF BUFFER FOR THE  DEPTH MAP /////////////////////////////////////////
        // buffer dimension: too large -> performance may slow down if we have many lights; too small -> strong aliasing
        // we create a Frame Buffer Object: the first rendering step will render to this buffer, and not to the real frame buffer
        glGenFramebuffers(1, &shadow_map_fb);
        // we create a texture for the depth map
        glGenTextures(1, &shadow_map_depth_map);
        glBindTexture(GL_TEXTURE_2D, shadow_map_depth_map);
        // in the texture, we will save only the depth data of the fragments. Thus, we specify that we need to render only depth in the first rendering step
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_map_resolution, shadow_map_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // we set to clamp the uv coordinates outside [0,1] to the color of the border
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // outside the area covered by the light frustum, everything is rendered in shadow (because we set GL_CLAMP_TO_BORDER)
        // thus, we set the texture border to white, so to render correctly everything not involved by the shadow map
        //*************
        GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // we bind the depth map FBO
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_depth_map, 0);
        // we set that we are not calculating nor saving color data
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;
    float range{30.0f}; // for shadow mapping purposes, we can define a range for the directional light to limit the orthographic projection
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
    // constructor (takes ownership of objects via move semantics)
    Scene(vector<Object>&& objects, Camera camera, DirectionalLight light)
    :objects(std::move(objects)), camera(camera), light(light){
        // edge_accentuation_shader = Shader("shaders/edge_accentuate.vert","shaders/edge_accentuate.frag");
        // lighting_shader = Shader("shaders/basic_normal.vert","shaders/basic_normal.frag");
        // projection_matrix = glm::perspective(glm::radians(FOV), ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);
    }
    // rendering of the whole scene
    void full_render(const std::map<std::string, UniformValue>& uniform_values, RenderMode mode, GLuint buffer, int render_width, int render_height) {
        if (mode == LIGHTING)// render shadowmap first
            full_render({}, SHADOWMAP, light.shadow_map_fb,light.shadow_map_resolution,light.shadow_map_resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glClear(GL_COLOR_BUFFER_BIT);
        // glClear(GL_DEPTH_BUFFER_BIT);
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

