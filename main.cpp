#include "main.h"

//////// RENDER PARAMETERS ///////////
enum debugMode {
    debug_NONE,
    debug_EDGE_ACCENTUATION,
    debug_EDGE_ACCENTUATION2,
    debug_ONLY_EDGE,
    debug_ONLY_LIGHTING,
    debug_BLUE_NOISE_SPHERE,
    debug_BAYER_DITHER_SPHERE,
    debug_SHADOWMAP,
};
debugMode debug = debug_NONE;
bool vsync = false;
bool rotate_light = true;
bool thin_outline = false;
bool black_background = false;
bool sphere_dither = false;

int shadow_map_resolution = 2048;

float pattern_value_blue = 10.0;
float pattern_value_bayer = 30.0;

int screenWidth = 1920, screenHeight = 1080;
GLuint factor = 40;
// GLuint renderWidth = 16*factor, renderHeight = 9*factor;
// GLuint renderWidth = screenWidth, renderHeight = screenHeight;
GLuint renderWidth = 640, renderHeight = 360;
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

    std::cout << "Loading scene..." << std::endl;
    // SCENE LOADING
    Scene scene = load_test_scene();
    selected_scene = &scene;
    // create the VAO and VBO for the full-screen quad
    create_quad_vao(&quadVAO, &quadVBO, quadVertices,sizeof(float)*24);

    char title[256];
    int n_frame = 0;
    
    create_framebuffer(&scene.blue_sphere_fb,&scene.blue_sphere_tex,&scene.blue_sphere_db,1280,720);
    create_framebuffer(&scene.bayer_sphere_fb,&scene.bayer_sphere_tex,&scene.bayer_sphere_db,1280,720);

    std::cout << "Setting up rendering elements..." << std::endl;
    //SETUP OF RENDER OBJECTS FOR PIPELINA
    Shader lighting_shader = Shader("shaders/lighting_dither.vert","shaders/lighting_dither.frag");
    Shader lighting_shader_sphere = Shader("shaders/lighting_dither2.vert","shaders/lighting_dither2.frag");
    Shader shadow_map_shader = Shader("shaders/19_shadowmap.vert","shaders/20_shadowmap.frag");
    LightingPass lighting_pass = LightingPass(renderWidth,renderHeight,&lighting_shader, &shadow_map_shader,shadow_map_resolution);

    Shader edge_accentuation_shader = Shader("shaders/edge_accentuate.vert","shaders/edge_accentuate.frag");
    EdgeAccentuation edge1_pass = EdgeAccentuation(renderWidth,renderHeight,&edge_accentuation_shader);

    Shader edge_accentuation2_shader = Shader("shaders/edge_accentuate2.vert","shaders/edge_accentuate2.frag");
    EdgeAccentuation2 edge2_pass = EdgeAccentuation2(renderWidth,renderHeight,&edge_accentuation2_shader);

    Shader wireframe_shader = Shader("shaders/wf.vert","shaders/wf.frag","shaders/wf.geom");
    Wireframe wireframe = Wireframe(renderWidth,renderHeight, &wireframe_shader, &edge2_pass);

    // SETUP OF THE FRAMEBUFFERS FOR THE POST PROCESSING PIPELINE

    std::cout << "Setting up post processing elements..." << std::endl;
    Shader edge_detect_shader("shaders/edge_detect.vert", "shaders/edge_detect_aa.frag");
    Shader edge_detect2_shader("shaders/edge_detect2.vert", "shaders/edge_detect2.frag");
    GLuint edge_detect_fb, edge_detect_tex, edge_detect_db;
    create_framebuffer(&edge_detect_fb,&edge_detect_tex,&edge_detect_db,renderWidth,renderHeight);

    Shader combine_shader("shaders/combine.vert", "shaders/combine.frag");
    GLuint combine_fb, combine_tex, combine_db;
    create_framebuffer(&combine_fb,&combine_tex,&combine_db,renderWidth,renderHeight);

    Shader upscale_shader("shaders/nearest_upscale.vert", "shaders/nearest_upscale.frag");

    std::cout << "Everything loaded. Entering Render Loop..." << std::endl;
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
        selected_scene->use_sphere_dithering = sphere_dither;

        selected_scene->pvbl = pattern_value_blue;
        selected_scene->pvby = pattern_value_bayer;

        if (black_background)
            glClearColor(0.0,0.0,0.0,1.0);
        else
            glClearColor(1.0,1.0,1.0,1.0);

        if (rotate_light)
            selected_scene->directional_light.rotate(15.0f * delta_time, glm::vec3(0.0,1.0f,0.0f));
        if (rotate_light){
            float old_pos = selected_scene->point_lights[1].position[2];
            float new_pos = fmod(old_pos + (5.0f*delta_time), 10.0f);
            selected_scene->point_lights[1].position[2] = new_pos;
            selected_scene->objects[13].set_position(selected_scene->point_lights[1].position);
        }

        // TODO OUTDATED!!
        /*  Render pipeline:
          1. render the scene at a low resolution with lighting only
          2a. render the scene at a low resolution with accentuated edges
          2b. render the scene at a low resolution with normal maps 
          3. apply edge detection to edge accentuated render and normal maps and combine them
          4. combine the edge detection's output with the lighting render to get the final low-res render
          5. upscale to screen resolution with nearest neighbor to keep the pixelated look*/

        // 1. lighting render pass
        lighting_pass.render(selected_scene);

        // 2. edge-accentuating render pass
        edge1_pass.render(selected_scene);
        if(!thin_outline){
            edge2_pass.render(selected_scene);
        }
        if(thin_outline){
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            wireframe.render(selected_scene);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        // 3. edge detection pass
        if (!thin_outline)
            post_process(edge_detect_fb, {*edge1_pass.get_texture(), *edge2_pass.get_texture(), selected_scene->blue_noise}, {"edge1_texture", "edge2_texture", "blue_noise"}, edge_detect_shader, renderWidth, renderHeight);
        else 
            post_process(edge_detect_fb, {*edge1_pass.get_texture(), *edge1_pass.get_depth_buffer(), *edge2_pass.get_texture()}, {"colorTexture", "depthTexture", "wireframeTexture"}, edge_detect2_shader, renderWidth, renderHeight);

        // 4. combination pass (lighting + edge detection's outline)
        post_process(combine_fb, {*lighting_pass.get_texture(), edge_detect_tex}, {"lightingTexture", "edgeTexture"}, combine_shader, renderWidth, renderHeight);
        
        // debug mode 
        GLuint* final_texture;
        if (debug == debug_NONE) {
            final_texture = &combine_tex;
        }else if (debug == debug_ONLY_EDGE) {
            // final_texture = &edge2_db;
            final_texture = &edge_detect_tex;
        }else if (debug == debug_EDGE_ACCENTUATION) {
            final_texture = edge1_pass.get_texture();
        }else if (debug == debug_EDGE_ACCENTUATION2) {
            final_texture = edge2_pass.get_texture();
        }else if (debug == debug_ONLY_LIGHTING) {
            final_texture = lighting_pass.get_texture();
        }else if (debug == debug_SHADOWMAP){
            final_texture = lighting_pass.shadow_map.get_depth_buffer();
        }else if (debug == debug_BAYER_DITHER_SPHERE){
            final_texture = &(selected_scene->bayer_sphere_tex);
        }else if (debug == debug_BLUE_NOISE_SPHERE){
            final_texture = &(selected_scene->blue_sphere_tex);
        }
        
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
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.Use();
    // shader.set_uniform1i("lowResTexture", 0);
    for (size_t i = 0; i < textures.size(); i++)
    {
        shader.set_uniform1i(texture_names[i], i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
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
    else if (key == GLFW_KEY_I && action == GLFW_PRESS){
        pattern_value_blue+= 0.5;
        std::cout << "Blue pattern scale = " << pattern_value_blue << std::endl;}
    else if (key == GLFW_KEY_K && action == GLFW_PRESS){
        pattern_value_blue-= 0.5;
        std::cout << "Blue pattern scale = " << pattern_value_blue << std::endl;}
    else if (key == GLFW_KEY_L && action == GLFW_PRESS){
        pattern_value_bayer-= 0.5;
        std::cout << "Bayer pattern scale = " << pattern_value_bayer << std::endl;}
    else if (key == GLFW_KEY_O && action == GLFW_PRESS){
        pattern_value_bayer+= 0.5;
        std::cout << "Bayer pattern scale = " << pattern_value_bayer << std::endl;}
    else if (key == GLFW_KEY_E && action == GLFW_PRESS){
        sphere_dither= !sphere_dither;}
    else if (key == GLFW_KEY_R && action == GLFW_PRESS){
        rotate_light = !rotate_light;}
    else if (key == GLFW_KEY_B && action == GLFW_PRESS){
        black_background = !black_background;}
    else if (key == GLFW_KEY_T && action == GLFW_PRESS){
        thin_outline = !thin_outline;}
    else if (key >= GLFW_KEY_1 && key <= (GLFW_KEY_1 + debug_SHADOWMAP) && action == GLFW_PRESS){
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
