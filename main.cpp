#include "main.h"
#include "misc.h"

//////// RENDER PARAMETERS ///////////
enum debugMode {
    NONE,
    EDGE_ACCENTUATION,
    ONLY_EDGE,
    ONLY_LIGHTING,
    SHADOWMAP,
};
debugMode debug = NONE;
bool vsync = true;

int screenWidth = 1920, screenHeight = 1080;
GLuint factor = 50;
GLuint renderWidth = 16*factor, renderHeight = 9*factor;
// GLuint renderWidth = screenWidth, renderHeight = screenHeight;
// GLuint renderWidth = 640, renderHeight = 360;
//////////////////////////////////////

GLfloat fps, current_time, last_time,delta_time = 1.0f;

bool keys[1024];
GLfloat lastX, lastY;
bool firstMouse = true;

// fullscren quad for post processing 
float quadVertices[] = {
    // positions     // tex coords
    -1.0f,  1.0f,    0.0f, 1.0f,
    -1.0f, -1.0f,    0.0f, 0.0f,
     1.0f, -1.0f,    1.0f, 0.0f,

    -1.0f,  1.0f,    0.0f, 1.0f,
     1.0f, -1.0f,    1.0f, 0.0f,
     1.0f,  1.0f,    1.0f, 1.0f
};
GLuint quadVAO, quadVBO;
Scene* selected_scene;
int main(){
    std::cout << "Entering main..." << std::endl;
    update_deltatime();

    std::cout << "Setting up OpenGL..." << std::endl;
    GLFWwindow* window = setup_openGL(&screenWidth, &screenHeight, key_callback, mouse_callback, true);
    if (!window) return -1;

    // SETUP OF THE FRAMEBUFFERS FOR THE RENDERING PIPELINE
    // lighting fb
    GLuint lighting_fb, lighting_tex, lighting_db;
    create_framebuffer(&lighting_fb, &lighting_tex, &lighting_db);
    // edge accentuation fb
    GLuint edge_fb, edge_tex, edge_db;
    create_framebuffer(&edge_fb, &edge_tex, &edge_db);
    // setup edge detection with shader 
    Shader edge_detect_shader("shaders/edge_detect.vert", "shaders/edge_detect.frag");
    GLuint edge_detect_fb, edge_detect_tex, edge_detect_db;
    create_framebuffer(&edge_detect_fb, &edge_detect_tex, &edge_detect_db);
    // setup combination fb and shader 
    Shader combine_shader("shaders/combine.vert", "shaders/combine.frag");
    GLuint combine_fb, combine_tex, combine_db;
    create_framebuffer(&combine_fb, &combine_tex, &combine_db);
    // upscale shader
    Shader upscale_shader("shaders/nearest_upscale.vert", "shaders/nearest_upscale.frag");
    // create the VAO and VBO for the full-screen quad
    create_quad_vao(&quadVAO, &quadVBO);

    // SCENE LOADING
    Scene test_scene = load_test_scene();
    selected_scene = &test_scene;

    char title[256];
    int n_frame = 0;
    std::cout << "Test scene loaded. Entering Render Loop..." << std::endl;
    while(!glfwWindowShouldClose(window)){
        n_frame++;
        update_deltatime();
        // update window title to display fps and frametime
        // update title every 60 frames
        if (n_frame >= 60) {
            n_frame = 0;
            snprintf(title, sizeof(title), "FPS: %.0f, Frame Time: %.2f ms", fps, delta_time * 1000.0f);
            glfwSetWindowTitle(window, title);
        }
        glfwPollEvents();
        apply_camera_movements();

        //////////ON FRAME EVENTS///////////////////////
        selected_scene->light.rotate(30.0f * delta_time, glm::vec3(0.0,1.0f,0.0f));
        /////////////////////////////////

        /*  Render pipeline:
          1. render the scene at a low resolution with accentuated edges
          2. render the scene at a low resolution with lighting only
          3. apply edge detection to edge accentuated render
          4. combine the edge detection's output with the lighting render to get the final low-res render
          5. upscale to screen resolution with nearest neighbor to keep the pixelated look*/

        // 2. lighting render pass
        test_scene.full_render({}, Scene::LIGHTING, lighting_fb, renderWidth,renderHeight);

        // 1. edge-accentuating render pass
        test_scene.full_render({}, Scene::EDGE_ACCENTUATION, edge_fb, renderWidth,renderHeight);

        // 3. edge detection pass
        post_process(edge_detect_fb, {edge_tex}, {"lowResTexture"}, edge_detect_shader, renderWidth, renderHeight);
        
        // 4. combination pass (lighting + edge detection's outline)
        post_process(combine_fb, {lighting_tex, edge_detect_tex}, {"lightingTexture", "edgeTexture"}, combine_shader, renderWidth, renderHeight);
        
        // 5. we upscale 
        GLuint* final_texture;
        if (debug == NONE) 
            final_texture = &combine_tex;
        else if (debug == ONLY_EDGE) 
            final_texture = &edge_detect_tex;
        else if (debug == EDGE_ACCENTUATION) 
            final_texture = &edge_tex;
        else if (debug == ONLY_LIGHTING) 
            final_texture = &lighting_tex;
        else if (debug == SHADOWMAP)
            final_texture = &test_scene.light.shadow_map_depth_map;
        
        post_process(0, {*final_texture}, {"lowResTexture"}, upscale_shader, screenWidth, screenHeight);

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // cleanup and exit
    glfwTerminate();
    //scene.delete(); //TODO implement this method to free the memory of the objects in the scene, and the shaders, etc...

    return 0;
}

void post_process(GLuint buffer, vector<GLuint> textures, vector<string> texture_names, Shader shader, int width, int height){
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.Use();
    shader.set_uniform1i("lowResTexture", 0);
    for (size_t i = 0; i < textures.size(); i++)
    {
        shader.set_uniform1i(texture_names[i], i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int create_quad_vao(GLuint* vao, GLuint* vbo) {
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    return 0;
}

int create_framebuffer(GLuint* framebuffer, GLuint* texture, GLuint* depth_buffer) {
    // generate texture for color attachment (low resolution)
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // set filtering to NEAREST to avoid blurring the image when we will render it on the screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // wrap clamp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //create low-resolution framebuffer for offscreen rendering
    glGenFramebuffers(1, framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);

    // depth buffer for the low-res framebuffer
    glGenRenderbuffers(1, depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, *depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, renderWidth, renderHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        //print error, incomplete framebuffer
        std::cout << "Error: Incomplete framebuffer!" << std::endl;
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}
void update_deltatime() {
    current_time = glfwGetTime();
    delta_time = current_time - last_time;
    fps = 1.0f / delta_time;
    last_time = current_time;
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
    else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_1 + SHADOWMAP && action == GLFW_PRESS){
        debug = static_cast<debugMode>(key - GLFW_KEY_1);
    }
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
