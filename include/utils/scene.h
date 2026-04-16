#pragma once

// using namespace std;

// Std. Includes
// #include <vector>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
// #include <glm/glm.hpp>
// #include <matrix_inverse.hpp>

enum Material {
    TEST,
    WOOD,
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

    Shader current_shader = Shader("shaders/basic_normal.vert", "shaders/basic_normal.frag");

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

    // rendering of the whole scene
    void full_render() {
        // we render the objects in the order of their material, so that we minimize the number of shader switches
        // (we can do this because we have sorted the vector of objects by material in the constructor)
        // Shader current_shader = Shader("shaders/basic_normal.vert", "shaders/basic_normal.frag");
        current_shader.Use();
        // Material current_material = objects[0].material;
        // cycle object reference in objects vector
        //TODO optimize by switching shader only when material changes, but for now we just use the same shader for all objects
        // cycle through references by index of objects in vector, necessaary because Object is move-only
        for (GLuint i = 0; i < objects.size(); i++) {
            // is this correct? we want to avoid copying the Object, but we need to access its members, so we take a pointer to it
            Object* object = &objects[i];
            // if (object->material != current_material) {
            //     current_material = object->material;
            //     // materialShaders[current_material]->Use();
            // }
            glm::mat4 model_matrix = object->model_matrix;
            current_shader.set_uniformMatrix4fv("modelMatrix", model_matrix);
            glm::mat4 view_matrix = camera.GetViewMatrix();
            current_shader.set_uniformMatrix4fv("viewMatrix", view_matrix);
            glm::mat4 projection_matrix = this->projection_matrix;
            current_shader.set_uniformMatrix4fv("projectionMatrix", projection_matrix);
            glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
            current_shader.set_uniformMatrix3fv("normalMatrix", normal_matrix);
            glm::vec3 light_dir = glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f));
            current_shader.set_uniform3fv("lightDir", light_dir);
            object->model.Draw();
        }
    }
private:

};

