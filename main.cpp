using namespace std;
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
// #include <iostream>
// #include <ostream>
// #include "scenes.cpp"
// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
// dimensions of application's window
//TODO: figure out how to change screen resolution but keep render resolution
// ideally screen resolution would be 2^n times render resolution 
GLuint screenWidth = 1280, screenHeight = 720;
GLuint renderWidth = 320, renderHeight = 180;
// the rendering steps used in the application
//TODO: learn ts PROPERLY like goddamn, you prolly need more render passes for lighting and edge-detection anyways
enum render_passes{ LIGHTING, SHADOWMAP, EDGE, RENDER};

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();


// we initialize an array of booleans for each keyboard key
bool keys[1024];

// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;
//MY FUNCTIONS
GLFWwindow* setup_openGL(int* width, int* height);
void setup_material_shaders();
Scene load_test_scene();
//fps calculations
GLfloat fps, current_time, last_time,delta_time = 1.0f;
void update_deltatime();

Scene* selected_scene;

// Vertex data for a full-screen quad (two triangles)
float quadVertices[] = {
    // positions     // tex coords
    -1.0f,  1.0f,    0.0f, 1.0f,
    -1.0f, -1.0f,    0.0f, 0.0f,
     1.0f, -1.0f,    1.0f, 0.0f,

    -1.0f,  1.0f,    0.0f, 1.0f,
     1.0f, -1.0f,    1.0f, 0.0f,
     1.0f,  1.0f,    1.0f, 1.0f
};

/////////////////// MAIN function ///////////////////////
int main(){
    std::cout << "Entering main..." << std::endl;
    update_deltatime();

    int width, height;
    std::cout << "Setting up OpenGL..." << std::endl;
    GLFWwindow* window = setup_openGL(&width, &height);
    if (!window) return -1;

    // generate texture for color attachment (low resolution)
    GLuint low_res_texture;
    glGenTextures(1, &low_res_texture);
    glBindTexture(GL_TEXTURE_2D, low_res_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // set filtering to NEAREST to avoid blurring the image when we will render it on the screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // wrap clamp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //create low-resolution framebuffer for offscreen rendering
    GLuint low_res_framebuffer;
    glGenFramebuffers(1, &low_res_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, low_res_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, low_res_texture, 0);

    // depth buffer for the low-res framebuffer
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, renderWidth, renderHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        //print error, incomplete framebuffer
        std::cout << "Error: Incomplete framebuffer!" << std::endl;
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // setup upscaling shader and quad for rendering the low-res texture to the screen
    Shader upscale_shader("shaders/nearest_upscale.vert", "shaders/nearest_upscale.frag");
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    // setup_material_shaders();

    // loading scene
    Scene test_scene = load_test_scene();
    selected_scene = &test_scene;

    char title[256];
    int n_frame = 0;
    std::cout << "Test scene loaded. Entering Render Loop..." << std::endl;
    while(!glfwWindowShouldClose(window)){
        n_frame++;
        // std::cout << "Frame n: " << n_frame << std::endl;
        update_deltatime();
        // update window title to display fps and frametime
        // update title every 60 frames
        if (n_frame >= 30) {
            n_frame = 0;
            snprintf(title, sizeof(title), "FPS: %.0f, Frame Time: %.2f ms", fps, delta_time * 1000.0f);
            glfwSetWindowTitle(window, title);
        }
        // Check is an I/O event is happening
        glfwPollEvents();

        // we apply FPS camera movements
        apply_camera_movements();

        // we render the scene
        // render to low-res framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, low_res_framebuffer);
        glViewport(0, 0, renderWidth, renderHeight);
        // glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // test clear color to red to see if the framebuffer is working correctly
        // glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        test_scene.full_render();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // render full screen quad with upscaled texture
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        upscale_shader.Use();
        glBindTexture(GL_TEXTURE_2D, low_res_texture);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        upscale_shader.set_uniform1f("lowResTexture", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // glBindVertexArray(0);

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // cleanup and exit
    glfwTerminate();
    //scene.delete(); //TODO implement this method to free the memory of the objects in the scene, and the shaders, etc...

    return 0;
}

void update_deltatime() {
    current_time = glfwGetTime();
    delta_time = current_time - last_time;
    fps = 1.0f / delta_time;
    last_time = current_time;
}

GLFWwindow* setup_openGL(int* width, int* height){
    // Initialization of OpenGL context using GLFW
    glfwInit();
    // We set OpenGL specifications required for this application
    // In this case: 4.1 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "easter egg!", nullptr, nullptr);
    if (!window){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    // we disable the mouse cursor
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return window;
    }
    // we define the viewport dimensions
    // int width, height;
    glfwGetFramebufferSize(window, width, height);
    // we enable Z test
    glEnable(GL_DEPTH_TEST);
    //the "clear" color for the frame buffer
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);
    return window;
}

// we load the image from disk and we create an OpenGL texture
// TODO move to scene.h
GLint LoadTexture(const char* path){
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels==4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;
}

// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
    // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A), 
    // then the camera will apply a compensation factor to the velocities applied in the single directions, 
    // in order to have the full v applied in the diagonal direction  
    // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.  
    GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]); 
    selected_scene->camera.SetMovementCompensation(diagonal_movement);
    
    if(keys[GLFW_KEY_SPACE])
        selected_scene->camera.ProcessKeyboard(UP, delta_time);
    if(keys[GLFW_KEY_LEFT_SHIFT])
        selected_scene->camera.ProcessKeyboard(DOWN, delta_time);
    if(keys[GLFW_KEY_W])
        selected_scene->camera.ProcessKeyboard(FORWARD, delta_time);
    if(keys[GLFW_KEY_W])
        selected_scene->camera.ProcessKeyboard(FORWARD, delta_time);
    if(keys[GLFW_KEY_S])
        selected_scene->camera.ProcessKeyboard(BACKWARD, delta_time);
    if(keys[GLFW_KEY_A])
        selected_scene->camera.ProcessKeyboard(LEFT, delta_time);
    if(keys[GLFW_KEY_D])
        selected_scene->camera.ProcessKeyboard(RIGHT, delta_time);
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    // Disable mouse cursor when left muose button is pressed down
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse=true;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if(firstMouse){
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        // offset of mouse cursor position
        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos;

        // the new position will be the previous one for the next frame
        lastX = xpos;
        lastY = ypos;

        // we pass the offset to the Camera class instance in order to update the rendering
        selected_scene->camera.ProcessMouseMovement(xoffset, yoffset);
    }
}
