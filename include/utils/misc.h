#pragma once
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
#include <stb_image/stb_image.h>


GLFWwindow* setup_openGL(int* width, int* height, GLFWkeyfun key_callback, GLFWcursorposfun mouse_callback, bool vsync);

// TODO move 
GLint LoadTexture(const char* path);

int create_quad_vao(GLuint* vao, GLuint* vbo, float*, unsigned long size) ;
int create_framebuffer(GLuint* framebuffer, GLuint* texture, GLuint* depth_buffer, int width, int height);
int create_pipeline_buffers(int width, int height, GLuint* lighting_fb, GLuint* lighting_tex, GLuint* lighting_db, GLuint* edge_fb, GLuint* edge_tex, GLuint* edge_db,  GLuint* edge_detect_fb, GLuint* edge_detect_tex, GLuint* edge_detect_db, GLuint* combine_fb, GLuint* combine_tex, GLuint* combine_db);
GLuint load_image(const char* path);