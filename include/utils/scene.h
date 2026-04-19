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
    WOOD,
    COMPLEX,
    SOMETHING_ELSE,
};


// data structure for scene objects
class Object {
public:
    // world coordinates
    glm::vec3 pos;
    glm::mat4 model_matrix;
    // model of the object
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
    // we can have a shader for each material, and we can switch between them when rendering the objects
    // static Shader test_shader;
    // static Shader wood_shader; 
    // static Shader something_else_shader;
    // static Shader* materialShaders[SOMETHING_ELSE + 1];
    static Scene load_test_scene();

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    static constexpr float FOV = 45.0f;
    static constexpr float ASPECT_RATIO = 16.0f/9.0f;
    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float FAR_PLANE = 10000.0f;
    static glm::mat4 projection_matrix;

    vector<Object> objects;
    Camera camera;
    // vector<Light> lights;
    // Light light;

   // constructor (takes ownership of objects via move semantics)
   Scene(vector<Object>&& objects, Camera camera)
    :objects(std::move(objects)), camera(camera) {
        //sort objects by material
        // std::sort(this->objects.begin(), this->objects.end(), [](const Object& a, const Object& b) {
        //     return a.material < b.material;
        // });
    }

    using UniformValue = std::variant<float, glm::vec2, glm::vec3, glm::mat3, glm::mat4>;
    // rendering of the whole scene
    // take in the shader to render the scene with, and a map to assign every uniform in the shader to the corresponding value in the scene (e.g. light direction, camera position, etc...)
    void full_render(Shader* shader, const std::map<std::string, UniformValue>& uniform_values, bool obj_based_uniform_assignment = false) {
        shader->Use();
        // set all the custom uniform values in the shader
        for (const auto& [name, value] : uniform_values) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, float>)
                    shader->set_uniform1f(name, arg);
                else if constexpr (std::is_same_v<T, glm::vec2>)
                    shader->set_uniform2fv(name, arg);
                else if constexpr (std::is_same_v<T, glm::vec3>)
                    shader->set_uniform3fv(name, arg);
                else if constexpr (std::is_same_v<T, glm::mat3>)
                    shader->set_uniformMatrix3fv(name, arg);
                else if constexpr (std::is_same_v<T, glm::mat4>)
                    shader->set_uniformMatrix4fv(name, arg);
            }, value);
        }
        // render objects
        for (GLuint i = 0; i < objects.size(); i++) {
            if (obj_based_uniform_assignment) {
                if (objects[i].material == COMPLEX)
                    {shader->set_uniform1f("fill_in", 1.0f);}
                else 
                    {shader->set_uniform1f("fill_in", 0.0f);}
                shader->set_uniform1f("object_id_in", (float)(i+1));
                shader->set_uniform1f("object_id_in", (float)(i+1));
                shader->set_uniform3fv("object_pos_in", objects[i].pos);
            }
            Object* object = &objects[i];
            glm::mat4 model_matrix = object->model_matrix;
            shader->set_uniformMatrix4fv("modelMatrix", model_matrix);
            glm::mat4 view_matrix = camera.GetViewMatrix();
            shader->set_uniformMatrix4fv("viewMatrix", view_matrix);
            glm::mat4 projection_matrix = this->projection_matrix;
            shader->set_uniformMatrix4fv("projectionMatrix", projection_matrix);
            glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
            shader->set_uniformMatrix3fv("normalMatrix", normal_matrix);
            object->model.Draw();
        }
    }
private:

};

