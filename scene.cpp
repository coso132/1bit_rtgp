#include "utils/scene.h"
#include <vector>

// Define static members
// Shader Scene::test_shader;
// Shader Scene::wood_shader;
// Shader Scene::something_else_shader;
// std::vector<Shader> Scene::materialShaders;
glm::mat4 Scene::projection_matrix = glm::perspective(Scene::FOV, Scene::ASPECT_RATIO, Scene::NEAR_PLANE, Scene::FAR_PLANE);

void setup_material_shaders(){
    // Scene::test_shader = Shader("shaders/basic_normal.vert", "shaders/basic_normal.frag");
    // Scene::wood_shader = Shader("shaders/wood.vs", "shaders/wood.fs");
    // Scene::something_else_shader = Shader("shaders/basic_normal.vert", "shaders/basic_normal.frag");
    // Scene::materialShaders[SIMPLE] = &Scene::test_shader;
    // Scene::materialShaders[WOOD] = &Scene::wood_shader;
    // Scene::materialShaders[SOMETHING_ELSE] = &Scene::something_else_shader;
}
// test scene
Scene load_test_scene(){
    // we load the model(s) 
    std::cout << "Loading test scene..." << std::endl;
    vector<Object> objects;
    objects.push_back(Object(glm::vec3( 5.0f, 1.0f,-1.0f), "models/cube.obj", SIMPLE));
    objects.push_back(Object(glm::vec3( 0.0f, 1.0f,-1.0f), "models/sphere.obj", COMPLEX));
    objects.push_back(Object(glm::vec3( 2.0f, 1.0f,-5.0f), "models/table_obj/table.obj", SIMPLE,2.0f));
    objects.push_back(Object(glm::vec3( -2.0f, 1.0f,-5.0f), "models/table_obj/table.obj", SIMPLE,2.0f));
    objects.push_back(Object(glm::vec3( 5.0f, 1.0f,-2.0f), "models/table_obj/table.obj", SIMPLE,2.0f));
    objects.push_back(Object(glm::vec3( 2.0f, 1.0f,2.0f), "models/table_obj/table.obj", SIMPLE,2.0f));
    objects.push_back(Object(glm::vec3(-5.0f, 1.0f,-1.0f), "models/bunny_lp.obj", COMPLEX, 0.5f));
    objects.push_back(Object(glm::vec3( 0.0f, -1.0f,0.0f), "models/plane.obj", SIMPLE, 5.0f));
    // objects.push_back(Object(glm::vec3( 0.0f, -100.0f,-100.0f), "models/city/OBJ/Amaryllis City.obj", SIMPLE, 0.01f));
    Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), false);
    return Scene(std::move(objects), camera);
}