#pragma once
// personally developed classes
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>
#include <utils/scene.h>

#include <string>
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif
#include <glad/glad.h>
#include <glfw/glfw3.h>
#ifdef _WINDOWS_
    #error windows.h was included!
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// FUNCTIONS
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();
void update_deltatime();
void post_process(GLuint buffer, vector<GLuint> textures, vector<string> texture_names, Shader shader, int width, int height);
