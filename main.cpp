#include "main.h"

//////// RENDER PARAMETERS ///////////
enum debugMode {
    debug_NONE,
    debug_EDGE_ACCENTUATION,
    debug_EDGE_ACCENTUATION2,
    debug_ONLY_EDGE,
    debug_ONLY_LIGHTING,
    debug_SHADOWMAP,
};
debugMode debug = debug_NONE;
bool vsync = false;
bool rotate_light = true;

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
    GLFWwindow* window = setup_openGL(&screenWidth, &screenHeight, key_callback, mouse_callback, vsync);
    if (!window) return -1;

    // SETUP OF THE FRAMEBUFFERS FOR THE RENDERING PIPELINE
    // lighting fb
    GLuint lighting_fb, lighting_tex, lighting_db, combine_fb, combine_tex, combine_db, edge_fb, edge_tex, edge_db,edge_detect_fb, edge_detect_tex, edge_detect_db;
    create_pipeline_buffers(renderWidth, renderHeight, &lighting_fb, &lighting_tex, &lighting_db, &edge_fb, &edge_tex, &edge_db, &edge_detect_fb, &edge_detect_tex, &edge_detect_db, &combine_fb, &combine_tex, &combine_db);
    GLuint edge2_fb, edge2_tex, edge2_db;
    create_framebuffer(&edge2_fb,&edge2_tex,&edge2_db,renderWidth,renderHeight);
    
    // POST PROCESSING AND OTHER SHADERS
    // Shader edge_detect_shader("shaders/edge_detect.vert", "shaders/edge_detect.frag");
    Shader edge_detect_shader("shaders/edge_detect.vert", "shaders/edge_detect_aa.frag");
    Shader combine_shader("shaders/combine.vert", "shaders/combine.frag");
    Shader upscale_shader("shaders/nearest_upscale.vert", "shaders/nearest_upscale.frag");

    // create the VAO and VBO for the full-screen quad
    create_quad_vao(&quadVAO, &quadVBO, quadVertices,sizeof(float)*24);

    // SCENE LOADING
    Scene scene = load_test_scene();
    // Scene scene = load_cottage2_scene();
    selected_scene = &scene;

    char title[256];
    int n_frame = 0;
    
    // RENDER LOOP
    std::cout << "Test scene loaded. Entering Render Loop..." << std::endl;
    while(!glfwWindowShouldClose(window)){
        n_frame++;
        update_deltatime();
        // update title every 60 frames
        if (n_frame >= 60) {
            n_frame = 0;
            snprintf(title, sizeof(title), "FPS: %.0f, Frame Time: %.2f ms", fps, delta_time * 1000.0f);
            glfwSetWindowTitle(window, title);
        }
        glfwPollEvents();
        apply_camera_movements();

        //////////ON FRAME EVENTS/////////
        if (rotate_light)
            selected_scene->directional_light.rotate(15.0f * delta_time, glm::vec3(0.0,1.0f,0.0f));
        if (rotate_light){
            float old_pos = selected_scene->point_lights[1].position[2];
            float new_pos = fmod(old_pos + (5.0f*delta_time), 10.0f);
            selected_scene->point_lights[1].position[2] = new_pos;
            selected_scene->objects[13].set_position(selected_scene->point_lights[1].position);
        }
        /////////////////////////////////

        /*  Render pipeline:
          1. render the scene at a low resolution with lighting only
          2a. render the scene at a low resolution with accentuated edges
          2b. render the scene at a low resolution with normal maps 
          3. apply edge detection to edge accentuated render and normal maps and combine them
          4. combine the edge detection's output with the lighting render to get the final low-res render
          5. upscale to screen resolution with nearest neighbor to keep the pixelated look*/

        // 1. lighting render pass
        selected_scene->full_render({}, LIGHTING, lighting_fb, renderWidth,renderHeight);

        // 2. edge-accentuating render pass
        selected_scene->full_render({},EDGE_ACCENTUATION, edge_fb, renderWidth,renderHeight);
        selected_scene->full_render({},EDGE_ACCENTUATION2, edge2_fb, renderWidth,renderHeight);

        // 3. edge detection pass
        // post_process(edge_detect_fb, {edge_tex}, {"lowResTexture"}, edge_detect_shader, renderWidth, renderHeight);
        post_process(edge_detect_fb, {edge_tex, edge2_tex}, {"edge1_texture", "edge2_texture"}, edge_detect_shader, renderWidth, renderHeight);
        
        // 4. combination pass (lighting + edge detection's outline)
        post_process(combine_fb, {lighting_tex, edge_detect_tex}, {"lightingTexture", "edgeTexture"}, combine_shader, renderWidth, renderHeight);
        
        // debug mode 
        GLuint* final_texture;
        if (debug == debug_NONE) 
            final_texture = &combine_tex;
        else if (debug == debug_ONLY_EDGE) 
            final_texture = &edge_detect_tex;
        else if (debug == debug_EDGE_ACCENTUATION) 
            final_texture = &edge_tex;
        else if (debug == debug_EDGE_ACCENTUATION2) 
            final_texture = &edge2_tex;
        else if (debug == debug_ONLY_LIGHTING) {
            final_texture = &lighting_tex;}
        else if (debug == debug_SHADOWMAP){
            final_texture = &(selected_scene->directional_light.shadow_map_depth_map);}
        
        // 5. we upscale 
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

void update_deltatime() {
    current_time = glfwGetTime();
    delta_time = current_time - last_time;
    fps = 1.0f / delta_time;
    last_time = current_time;
}


void apply_camera_movements()
{
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
    else if (key == GLFW_KEY_R && action == GLFW_PRESS){
        rotate_light = !rotate_light;
    }else if (key >= GLFW_KEY_1 && key <= (GLFW_KEY_1 + debug_SHADOWMAP) && action == GLFW_PRESS){
        // std::cout << "DEBUG MOVE " << key << std::endl;
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
