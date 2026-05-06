#include "utils/misc.h"

GLFWwindow* setup_openGL(int* width, int* height, GLFWkeyfun key_callback, GLFWcursorposfun mouse_callback, bool vsync){
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
    GLFWwindow* window = glfwCreateWindow(*width, *height, "easter egg!", nullptr, nullptr);
    if (!window){
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return window;
    }
    glfwMakeContextCurrent(window);
    if (vsync)
        glfwSwapInterval(1);
    else
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return window;
}
int create_quad_vao(GLuint* vao, GLuint* vbo, float * quadVertices, unsigned long size) {
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, size, quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    return 0;
}

int create_framebuffer(GLuint* framebuffer, GLuint* texture, GLuint* depth_buffer, int width, int height) {
    // generate texture for color attachment (low resolution)
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // set filtering to NEAREST to avoid blurring the image when we will render it on the screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        //print error, incomplete framebuffer
        std::cout << "Error: Incomplete framebuffer!" << std::endl;
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}
int create_framebuffer2(GLuint* framebuffer, GLuint* color_texture, GLuint* depth_texture, int width, int height) {
    glGenTextures(1, color_texture);
    glBindTexture(GL_TEXTURE_2D, *color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // depth texture (instead of renderbuffer)
    glGenTextures(1, depth_texture);
    glBindTexture(GL_TEXTURE_2D, *depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    glGenFramebuffers(1, framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *color_texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_texture, 0);

    // Explicitly set draw buffer
    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuf);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error: Incomplete framebuffer!" << std::endl;
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

int create_pipeline_buffers(int width, int height, GLuint* lighting_fb, GLuint* lighting_tex, GLuint* lighting_db, GLuint* edge_fb, GLuint* edge_tex, GLuint* edge_db,  GLuint* edge_detect_fb, GLuint* edge_detect_tex, GLuint* edge_detect_db, GLuint* combine_fb, GLuint* combine_tex, GLuint* combine_db){
    create_framebuffer(lighting_fb, lighting_tex, lighting_db, width, height);
    // edge accentuation fb
    create_framebuffer2(edge_fb, edge_tex, edge_db, width, height);
    // setup edge detection with shader 
    create_framebuffer(edge_detect_fb, edge_detect_tex, edge_detect_db, width, height);
    // setup combination fb and shader 
    create_framebuffer(combine_fb, combine_tex, combine_db, width, height);
    return 1;
}
GLuint load_image(const char* path){
        GLuint texture;
        int w, h, channels;
        unsigned char* image;
        image = stbi_load(path, &w, &h, &channels, STBI_rgb);

        if (image == nullptr)
            std::cout << "Failed to load texture!" << std::endl;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // 3 channels = RGB ; 4 channel = RGBA
        if (channels==3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels==4)
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        // glGenerateMipmap(GL_TEXTURE_2D);
        // we set how to consider UVs outside [0,1] range
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // we set the filtering for minification and magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // we free the memory once we have created an OpenGL texture
        stbi_image_free(image);

        // we set the binding to 0 once we have finished
        glBindTexture(GL_TEXTURE_2D, 0);
        return texture;
    }
GLuint create_cubemap_from_file(const std::string& filepath, int tiling = 1)
{
    if (tiling < 1) tiling = 1;

    // Load the original image
    int src_w, src_h, channels;
    unsigned char* src_data = stbi_load(filepath.c_str(), &src_w, &src_h, &channels, 0);
    if (!src_data)
    {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0;
    }

    // Determine OpenGL format
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    int bytes_per_pixel = channels;

    // Tiled image dimensions
    int tile_w = src_w * tiling;
    int tile_h = src_h * tiling;
    size_t tile_size = tile_w * tile_h * bytes_per_pixel;
    unsigned char* tile_data = new unsigned char[tile_size];

    // Fill tiled image by repeating the source
    for (int y = 0; y < tile_h; ++y)
    {
        int src_y = y % src_h;
        for (int x = 0; x < tile_w; ++x)
        {
            int src_x = x % src_w;
            // Copy pixel
            for (int c = 0; c < bytes_per_pixel; ++c)
            {
                tile_data[(y * tile_w + x) * bytes_per_pixel + c] =
                    src_data[(src_y * src_w + src_x) * bytes_per_pixel + c];
            }
        }
    }

    // Create cubemap texture
    GLuint cubemapID;
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

    // Upload the same tiled image to all six faces
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
                     tile_w, tile_h, 0, format, GL_UNSIGNED_BYTE, tile_data);
    }

    delete[] tile_data;
    stbi_image_free(src_data);

    // Set cubemap parameters
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);  // not strictly needed for cubemap
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return cubemapID;
}

void create_shadow_map(GLuint* shadow_map_fb, GLuint* shadow_map_depth_map, int shadow_map_resolution){
        // create shadowmap
        glGenFramebuffers(1, shadow_map_fb);
        glGenTextures(1, shadow_map_depth_map);
        glBindTexture(GL_TEXTURE_2D, *shadow_map_depth_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_map_resolution, shadow_map_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindFramebuffer(GL_FRAMEBUFFER, *shadow_map_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *shadow_map_depth_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
 