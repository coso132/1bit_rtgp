#pragma once

#include <variant>
#include <map>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/light.h>
#include <utils/sceneobject.h>
#include <utils/misc.h>
// #include <utils/rendering.h>

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

    vector<Object> objects;
    // Shader* current_shader;
    // RenderMode current_rendermode;
    Camera camera;
    DirectionalLight directional_light; // singular directional light for now
    vector<PointLight> point_lights; // multiple point lights
    int nl;
    
    // blue noise texture
    GLuint blue_noise;
    GLuint bayer_noise;

    GLuint blue_noise_sphere;
    GLuint bayer_noise_sphere;

    float pvby = 24.0;
    float pvbl = 10.0;
    bool use_sphere_dithering = false;

    // TODO MOVE EVERYTHING REGARDING DITHER SPHERE ON LIGHTINGPASS CLASS 
    Object dither_sphere = Object(camera.Position, "models/ICOuv.obj",DITHER_SPHERE);
    GLuint blue_sphere_fb, blue_sphere_tex, blue_sphere_db;
    Shader blue_sphere_shader = Shader("shaders/sphere2.vert","shaders/sphere2.frag");
    GLuint bayer_sphere_fb, bayer_sphere_tex, bayer_sphere_db;
    Shader bayer_sphere_shader = Shader("shaders/sphere2.vert","shaders/sphere2.frag");
    glm::mat4 m;
    glm::mat4 v;
    glm::mat4 p;

    // LightingPass lighting_pass;

    // constructor
    Scene(vector<Object>&& objects, Camera camera, DirectionalLight light, vector<PointLight> point_lights)
    :objects(std::move(objects)), camera(camera), directional_light(light), point_lights(point_lights){
        blue_noise = load_image("textures/blue_noise.png");
        bayer_noise = load_image("textures/bayer_noise.png");
        m = dither_sphere.model_matrix;
        v = camera.GetViewMatrix();
        p = this->projection_matrix;
        nl = point_lights.size();
        // lighting_pass= LightingPass();
    }

    GLuint create_cubemap_checkerboard(int size = 256)
    {
        // Create a simple checkerboard pattern in memory
        unsigned char* data = new unsigned char[size * size * 3];
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                int c = ((x / 32) + (y / 32)) % 2 * 255;
                data[(y * size + x) * 3 + 0] = c;
                data[(y * size + x) * 3 + 1] = c;
                data[(y * size + x) * 3 + 2] = c;
            }
        }
        
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        for (int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                        size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        delete[] data;
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return id;
    }

    void update_blue_sphere_cubemap(){
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        // glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, blue_sphere_fb);
        glViewport(0, 0, 1280, 720);
        // glViewport(0, 0, 640, 360);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blue_sphere_shader.Use();
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, blue_noise_sphere);
        // sphere_shader.set_uniform1i("uCubemap",0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blue_noise);
        blue_sphere_shader.set_uniform1i("noise",0);
        blue_sphere_shader.set_uniform1f("tile",pvbl);
        dither_sphere.pos = camera.Position;
        // dither_sphere.set_position(camera.Position);
        glm::mat4 viewRot = glm::mat4(glm::mat3(camera.GetViewMatrix())); // rotation only
        glm::mat4 model = glm::mat4(1.0f);                   // identity

        glm::mat4 mvp = this->projection_matrix * viewRot * model;

        blue_sphere_shader.set_uniformMatrix4fv("mvp", mvp);
        dither_sphere.model.Draw();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void update_bayer_sphere_cubemap(){
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        // glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, bayer_sphere_fb);
        glViewport(0, 0, 1280, 720);
        // glViewport(0, 0, 640, 360);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bayer_sphere_shader.Use();
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, bayer_noise_sphere);
        // sphere_shader.set_uniform1i("uCubemap",0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bayer_noise);
        bayer_sphere_shader.set_uniform1i("noise",0);
        bayer_sphere_shader.set_uniform1f("tile",pvby);
        dither_sphere.pos = camera.Position;
        // dither_sphere.set_position(camera.Position);
        glm::mat4 viewRot = glm::mat4(glm::mat3(camera.GetViewMatrix())); // rotation only
        glm::mat4 model = glm::mat4(1.0f);                   // identity

        glm::mat4 mvp = this->projection_matrix * viewRot * model;

        bayer_sphere_shader.set_uniformMatrix4fv("mvp", mvp);
        dither_sphere.model.Draw();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:

};

Scene load_test_scene();
Scene load_cottage1_scene();
// Scene load_cottage2_scene();

