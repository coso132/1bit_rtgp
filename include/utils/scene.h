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

/////////////////// SCENE class ///////////////////////
//TODO EXPLAIN
class Scene {
public:
    static constexpr float FOV = 45.0f;
    static constexpr float ASPECT_RATIO = 16.0f/9.0f;
    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float FAR_PLANE = 10000.0f;
    static glm::mat4 projection_matrix;

    vector<Object> objects;
    Camera camera;
    DirectionalLight directional_light; // singular directional light for now
    vector<PointLight> point_lights; // multiple point lights
    int nl;

    // constructor
    Scene(vector<Object>&& objects, Camera camera, DirectionalLight light, vector<PointLight> point_lights)
    :objects(std::move(objects)), camera(camera), directional_light(light), point_lights(point_lights){
        nl = point_lights.size();
    }
   
private:

};

Scene load_test_scene();
Scene load_cottage1_scene();
// Scene load_cottage2_scene();

