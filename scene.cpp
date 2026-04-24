#include "utils/scene.h"
#include <vector>

glm::mat4 Scene::projection_matrix = glm::perspective(Scene::FOV, Scene::ASPECT_RATIO, Scene::NEAR_PLANE, Scene::FAR_PLANE);

// test scene
Scene load_test_scene(){
    // we load the model(s) 
    std::cout << "Loading test scene..." << std::endl;
    vector<Object> objects;
    objects.push_back(Object(
        glm::vec3( -15.0f, 0.0f,-15.0f), 
        "models/cossack/CossackFull.obj", 
        COMPLEX,
        "models/cossack/Material_Base_Color.png",
        BLUE_NOISE,
        1.0f));
    objects[0].LoadNormalTexture("models/cossack/Material_Normal.png");
    objects.push_back(Object(
        glm::vec3( 15.0f, 0.0f,-15.0f), 
        "models/cottage2/Cottage_FREE.obj", 
        SIMPLE,
        "models/cottage2/Cottage_Clean_Base_Color.png",
        BLUE_NOISE,
        1.0f));
    objects[1].LoadNormalTexture("models/cottage2/Cottage_Clean_Normal.png");
    objects.push_back(Object(
        glm::vec3( 2.0f, 1.0f,-5.0f),   // position
        "models/table_obj/table.obj",   // model location
        SIMPLE,                         // material
        "models/table_obj/table_specular_map.png",  // texture 
        BAYER,                          // noise type
        2.0f));                         // scale
    objects.push_back(Object(
        glm::vec3( 5.0f, 1.0f,-1.0f), 
        "models/cube.obj", 
        SIMPLE, 
        "textures/UV_Grid_Sm.png",  // texture 
        BAYER,                          // noise type
        1.0f));
    objects.push_back(Object(
        glm::vec3( 0.0f, 1.0f,-1.0f), 
        "models/sphere.obj", 
        COMPLEX, 
        "textures/white.png",  // texture 
        BLUE_NOISE,                          // noise type
        1.0f));
    objects.push_back(Object(
        glm::vec3( -4.0f, 1.0f,-6.0f), 
        "models/table_obj/table.obj", 
        SIMPLE,
        "models/table_obj/table_specular_map.png",  // texture 
        BLUE_NOISE,                          // noise type
        2.0f));
    objects.push_back(Object(
        glm::vec3( 6.0f, 1.0f,-6.0f), 
        "models/table_obj/table.obj", 
        SIMPLE,
        "textures/white.png",  // texture 
        BAYER,                          // noise type
        2.0f));
    objects.push_back(Object(
        glm::vec3( 4.0f, 1.0f,4.0f), 
        "models/table_obj/table.obj", 
        SIMPLE,
        "textures/white.png",  // texture 
        BAYER,                          // noise type
        2.0f));
    objects.push_back(Object(
        glm::vec3(-5.0f, 1.0f,-1.0f), 
        "models/bunny_lp.obj", 
        COMPLEX, 
        "textures/white.png",  // texture 
        BLUE_NOISE,                          // noise type
        0.5f));
    objects.push_back(Object(
        glm::vec3( -10.0f, 5.0f,0.0f), 
        "models/plane.obj", 
        SIMPLE,
        "textures/SoilCracked.png",  // texture 
        BAYER,                          // noise type
        2.0f,
        glm::vec3( 0.0f, 0.0f, 1.0f),
        -1.5708f));
    objects.push_back(Object(
        glm::vec3( 5.0f, -0.5f,0.0f), 
        "models/plane.obj", 
        SIMPLE,
        "textures/SoilCracked.png",  // texture 
        BAYER,                          // noise type
        5.0f
        // glm::vec3( 0.0f, 0.0f, 1.0f),
        // 1.5708f
    ));
    // objects.push_back(Object(glm::vec3( 0.0f, -100.0f,-100.0f), "models/city/OBJ/Amaryllis City.obj", SIMPLE, 0.01f));
    Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), false);
    DirectionalLight directional_light(glm::vec3(-1.0, -1.0,-1.0), 30.0f, Shader("shaders/19_shadowmap.vert","shaders/20_shadowmap.frag"));
    return Scene(std::move(objects), camera, directional_light);
}

// test scene
Scene load_cottage1_scene(){
    // we load the model(s) 
    std::cout << "Loading cottage2 scene..." << std::endl;
    vector<Object> objects;
    objects.push_back(Object(
        glm::vec3( 10.0f, 0.0f,-10.0f), 
        "models/cottage1/cottage_obj.obj", 
        SIMPLE,
        "models/cottage1/cottage_diffuse.png",
        BLUE_NOISE,
        1.0f));
    objects[0].LoadNormalTexture("models/cottage1/cottage_normal.png");
    // objects.push_back(Object(glm::vec3( 0.0f, 0.0f,0.0f), "models/plane.obj", SIMPLE,"textures/SoilCracked.png", 5.0f));
    // objects.push_back(Object(glm::vec3( 0.0f, -100.0f,-100.0f), "models/city/OBJ/Amaryllis City.obj", SIMPLE, 0.01f));
    Camera camera(glm::vec3(0.0f, 2.0f, 7.0f), false);
    DirectionalLight directional_light(glm::vec3(-1.0, -0.8,-1.0), 20.0f, Shader("shaders/19_shadowmap.vert","shaders/20_shadowmap.frag"));
    return Scene(std::move(objects), camera, directional_light);
}
// test scene
Scene load_cottage2_scene(){
    // we load the model(s) 
    std::cout << "Loading cottage2 scene..." << std::endl;
    vector<Object> objects;
    objects.push_back(Object(
        glm::vec3( 10.0f, 0.0f,-10.0f), 
        "models/cottage2/Cottage_FREE.obj", 
        SIMPLE,
        "models/cottage2/Cottage_Clean_Base_Color.png",
        BLUE_NOISE,
        1.0f));
    objects[0].LoadNormalTexture("models/cottage2/Cottage_Clean_Base_Color.png");
    objects.push_back(Object(
        glm::vec3( 0.0f, 0.0f,0.0f), 
        "models/plane.obj", 
        SIMPLE,
        "textures/SoilCracked.png", 
        BAYER,
        5.0f));
    Camera camera(glm::vec3(0.0f, 2.0f, 7.0f), false);
    DirectionalLight directional_light(glm::vec3(-1.0, -0.8,-1.0), 30.0f, Shader("shaders/19_shadowmap.vert","shaders/20_shadowmap.frag"));
    return Scene(std::move(objects), camera, directional_light);
}