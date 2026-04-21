#pragma once
// personally developed classes
// #include <utils/shader.h>
// #include <utils/model.h>
// #include <utils/camera.h>
// #include <utils/scene.h>
// #include "main.h"

#include <string>
#include <iostream>
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif
#include <glad/glad.h>
#include <glfw/glfw3.h>
#ifdef _WINDOWS_
    #error windows.h was included!
#endif
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/matrix_inverse.hpp>
// #include <glm/gtc/type_ptr.hpp>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>


GLFWwindow* setup_openGL(int* width, int* height, GLFWkeyfun key_callback, GLFWcursorposfun mouse_callback, bool vsync);

// TODO move to scene.h
GLint LoadTexture(const char* path);