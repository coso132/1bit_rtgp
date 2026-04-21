#include "utils/scene.h"
#include <vector>

glm::mat4 Scene::projection_matrix = glm::perspective(Scene::FOV, Scene::ASPECT_RATIO, Scene::NEAR_PLANE, Scene::FAR_PLANE);

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
    DirectionalLight directional_light(glm::vec3(-1.0, -1.0,-1.0), 20.0f, Shader("shaders/19_shadowmap.vert","shaders/20_shadowmap.frag"));
    return Scene(std::move(objects), camera, directional_light);
}