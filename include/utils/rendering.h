#pragma once

#include <utils/scene.h>


//TODO proper comments everywher
// idea for how to do render passes
class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void render(Scene* scene)=0;

    GLuint* get_depth_buffer() {
        return &depth_buffer;}
    GLuint* get_buffer() {
        return &buffer;}
    GLuint* get_texture() {
        return &texture;}
    int get_width(){
        return render_width;}
    int get_height(){
        return render_height;}
protected:
    int render_width {640};
    int render_height {360};
    GLuint buffer;
    GLuint texture;
    GLuint depth_buffer;
    Shader* shader;
};

class ShadowMap:public RenderPass{
public:
    ShadowMap(int resolution, Shader* shader) {
        create_shadow_map(&this->buffer,&this->depth_buffer, resolution);
        this->render_width = resolution;
        this->render_height = resolution;
        this->shader = shader;
        // this->associated_directional_light = associated_directional_light;
    }
    void render(Scene* scene) override{
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render dependant stuff
        shader->Use();
        associated_directional_light = &scene->directional_light;
        glm::mat4 light_space_matrix = associated_directional_light->get_light_space_matrix();
        shader->set_uniformMatrix4fv("lightSpaceMatrix", light_space_matrix);

        // render objects
        for (GLuint i = 0; i < scene->objects.size(); i++) {
            Object* object = &scene->objects[i];
            if (object->material == DUST) continue;
            shader->set_uniformMatrix4fv("modelMatrix", object->model_matrix);
            object->draw();
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
private:
    DirectionalLight* associated_directional_light;
};

class LightingPass:public RenderPass{
public:
    // for directional light
    ShadowMap shadow_map;
    // pattern repetition factor for blue noise and bayer dither sphere
    float pvby = 24.0;
    float pvbl = 10.0;
    bool use_sphere_dithering = false;
    // dither textures
    GLuint blue_noise;
    GLuint bayer_noise;
    // dither sphere buffers
    GLuint blue_sphere_fb, blue_sphere_tex, blue_sphere_db;
    GLuint bayer_sphere_fb, bayer_sphere_tex, bayer_sphere_db;
 
    LightingPass(int render_width, int render_height, Shader* lighting_shader, Shader* shadow_map_shader, int shadow_map_resolution): shadow_map(shadow_map_resolution, shadow_map_shader){
        create_framebuffer(&buffer,&texture,&depth_buffer,render_width,render_height);
        this->render_width = render_width;
        this->render_height = render_height;
        this->shader = lighting_shader;
        blue_noise = load_image("textures/blue_noise.png");
        bayer_noise = load_image("textures/bayer_noise.png");
        m = dither_sphere.model_matrix;
        create_framebuffer(&blue_sphere_fb,&blue_sphere_tex,&blue_sphere_db,2*render_width,2*render_height); //TODO MAGIC NUMBER
        create_framebuffer(&bayer_sphere_fb,&bayer_sphere_tex,&bayer_sphere_db,2*render_width,2*render_height);
   }
    void render(Scene* scene) override{
        // update shadowmap
        shadow_map.render(scene);

        v = scene->camera.GetViewMatrix();
        p = scene->projection_matrix;
        // update spheres
        if (use_sphere_dithering){
            update_blue_sphere_cubemap(scene);
            update_bayer_sphere_cubemap(scene);
        }

        // reset render parameters
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        shader->Use();

        // set directional light uniforms
        DirectionalLight* dl = &scene->directional_light;
        shader->set_uniform3fv("directionalLightDirection", dl->direction);
        // shader->set_uniform1f("directionalLightIntensity", dl->intensity);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *shadow_map.get_depth_buffer());
        shader->set_uniform1i("shadowMap", 0);
        shader->set_uniformMatrix4fv("lightSpaceMatrix", dl->light_space_matrix);

        // set point lights uniforms
        shader->set_uniform1i("numPointLights",scene->nl);
        shader->set_uniform3fv("cameraWorldPos",scene->camera.Position);
        for (int i = 0; i < scene->nl; i++){
            string si = to_string(i);
            string pli = ("pointLightIntensity[" + si + ']').c_str();
            string plp =("PointLightPos["+si+']').c_str();
            scene->point_lights[i].set_shader_uniforms(shader, pli, plp);
        }

        shader->set_uniform1i("sphere", use_sphere_dithering? 1:0);
        // render objects
        for (GLuint i = 0; i < scene->objects.size(); i++) {
            Object* object = &scene->objects[i];
            if (object->material == DUST) continue;
            set_object_uniforms(scene,object,i);
            object->draw();
            // glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
private:
    

    // dither sphere buffers
    GLuint blue_noise_sphere;
    GLuint bayer_noise_sphere;

    Object dither_sphere = Object(glm::vec3(0.0,0.0,0.0), "models/ICOuv.obj",DITHER_SPHERE);
    Shader blue_sphere_shader = Shader("shaders/sphere2.vert","shaders/sphere2.frag");
    Shader bayer_sphere_shader = Shader("shaders/sphere2.vert","shaders/sphere2.frag");
    glm::mat4 m;
    glm::mat4 v;
    glm::mat4 p;



    void update_blue_sphere_cubemap(Scene* scene){
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        glBindFramebuffer(GL_FRAMEBUFFER, blue_sphere_fb);
        //TODO MAGIC NUMBER
        glViewport(0, 0, 2*render_width, 2*render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blue_sphere_shader.Use();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blue_noise); // bind blue noise texture
        blue_sphere_shader.set_uniform1i("noise",0); 

        blue_sphere_shader.set_uniform1f("tile",pvbl); // set tiling factor for blue noise
        
        // set dither sphere position to camera position, so that it always surrounds the camera
        dither_sphere.pos = scene->camera.Position;
        
        glm::mat4 viewRot = glm::mat4(glm::mat3(scene->camera.GetViewMatrix())); // rotation only
        glm::mat4 model = glm::mat4(1.0f);                   // identity

        glm::mat4 mvp = scene->projection_matrix * viewRot * model;

        blue_sphere_shader.set_uniformMatrix4fv("mvp", mvp);
        dither_sphere.model.Draw();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void update_bayer_sphere_cubemap(Scene* scene){
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        // glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, bayer_sphere_fb);
        glViewport(0, 0, 2*render_width, 2*render_height);
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
        dither_sphere.pos = scene->camera.Position;
        // dither_sphere.set_position(camera.Position);
        glm::mat4 viewRot = glm::mat4(glm::mat3(scene->camera.GetViewMatrix())); // rotation only
        glm::mat4 model = glm::mat4(1.0f);                   // identity

        glm::mat4 mvp = scene->projection_matrix * viewRot * model;

        bayer_sphere_shader.set_uniformMatrix4fv("mvp", mvp);
        dither_sphere.model.Draw();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void set_object_uniforms(Scene* scene, Object* obj, int i) {
        
        // reset texutred or dust status
        // glBindTexture(GL_TEXTURE_2D, 0);
        shader->set_uniform1f("textured",0.0);
        shader->set_uniform1i("dust",0);

        // texture and noise uniforms
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, obj->texture);
        shader->set_uniform1i("tex",1);
        shader->set_uniform1f("repeat",1.0);
        shader->set_uniform1f("textured",1.0);//TODO fix this uniforms type
        shader->set_uniform1i("noise_type",obj->noise_type);

        // dither uniforms
        GLuint dither_map = (use_sphere_dithering?(obj->noise_type==BAYER? bayer_sphere_tex : blue_sphere_tex):(obj->noise_type==BAYER? bayer_noise:blue_noise));
        float dither_width = use_sphere_dithering? render_width :(obj->noise_type==BAYER?16:64);
        float dither_height = use_sphere_dithering? render_height :(dither_width);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dither_map); 
        shader->set_uniform1i("dither_map",2);
        shader->set_uniform1f("width",dither_width);
        shader->set_uniform1f("height",dither_height);
         
        // obvious mvp matrixes
        glm::mat4 model_matrix = obj->model_matrix;
        shader->set_uniformMatrix4fv("modelMatrix", obj->model_matrix);
        glm::mat4 view_matrix = scene->camera.GetViewMatrix();
        shader->set_uniformMatrix4fv("viewMatrix", view_matrix);
        glm::mat4 projection_matrix = scene->projection_matrix;
        shader->set_uniformMatrix4fv("projectionMatrix", projection_matrix);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
        shader->set_uniformMatrix3fv("normalMatrix", normal_matrix);
    }
};


class EdgeAccentuation:public RenderPass{
public:
    // for directional light
    EdgeAccentuation(int render_width, int render_height, Shader* shader) {
        create_framebuffer2(&buffer,&texture,&depth_buffer,render_width,render_height);
        this->render_width = render_width;
        this->render_height = render_height;
        this->shader = shader;
    }
    void render(Scene* scene) override{
        // reset render parameters
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        shader->Use();

        // render objects
        for (GLuint i = 0; i < scene->objects.size(); i++) {
            Object* object = &scene->objects[i];
            if (object->material == DUST) continue;
            set_object_uniforms(scene,object,i);
            object->draw();
        }
    }
private:
    void set_object_uniforms(Scene* scene, Object* obj, int i) {
        
        // reset texutred or dust status
        // glBindTexture(GL_TEXTURE_2D, 0);
        shader->set_uniform1f("textured",0.0);
        shader->set_uniform1i("dust",0);

        if (obj->material == COMPLEX)
            {shader->set_uniform1f("fill_in", 1.0f);}
        else 
            {shader->set_uniform1f("fill_in", 0.0f);}
        shader->set_uniform1f("object_id_in", (float)(i+1));
        // texture and noise uniforms
        
        // obvious mvp matrixes
        glm::mat4 model_matrix = obj->model_matrix;
        shader->set_uniformMatrix4fv("modelMatrix", obj->model_matrix);
        glm::mat4 view_matrix = scene->camera.GetViewMatrix();
        shader->set_uniformMatrix4fv("viewMatrix", view_matrix);
        glm::mat4 projection_matrix = scene->projection_matrix;
        shader->set_uniformMatrix4fv("projectionMatrix", projection_matrix);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
        shader->set_uniformMatrix3fv("normalMatrix", normal_matrix);
    }
};



class EdgeAccentuation2:public RenderPass{
public:
    // for directional light
    EdgeAccentuation2(int render_width, int render_height, Shader* shader) {
        create_framebuffer(&buffer,&texture,&depth_buffer,render_width,render_height);
        this->render_width = render_width;
        this->render_height = render_height;
        this->shader = shader;
   }
    void render(Scene* scene) override{
        // reset render parameters
        glDepthFunc(GL_LESS); 
        glDepthMask(GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        shader->Use();

        // render objects
        for (GLuint i = 0; i < scene->objects.size(); i++) {
            Object* object = &scene->objects[i];
            set_object_uniforms(scene,object,i);
            object->draw();
        }
    }
private:
    void set_object_uniforms(Scene* scene, Object* obj, int i) {
        
        // reset texutred or dust status
        // glBindTexture(GL_TEXTURE_2D, 0);
        shader->set_uniform1f("textured",0.0);
        shader->set_uniform1i("dust",0);

        if (obj->material == COMPLEX)
            {shader->set_uniform1f("fill_in", 1.0f);}
        else 
            {shader->set_uniform1f("fill_in", 0.0f);}
        shader->set_uniform1f("object_id_in", (float)(i+1));

        // normal texture uniforms
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, obj->normal_texture);
        shader->set_uniform1i("normal_tex",1);
        shader->set_uniform1f("repeat",1.0);
        shader->set_uniform1f("textured",1.0);
        shader->set_uniform1i("dust", obj->material == DUST);
       
        // obvious mvp matrixes
        glm::mat4 model_matrix = obj->model_matrix;
        shader->set_uniformMatrix4fv("modelMatrix", obj->model_matrix);
        glm::mat4 view_matrix = scene->camera.GetViewMatrix();
        shader->set_uniformMatrix4fv("viewMatrix", view_matrix);
        glm::mat4 projection_matrix = scene->projection_matrix;
        shader->set_uniformMatrix4fv("projectionMatrix", projection_matrix);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
        shader->set_uniformMatrix3fv("normalMatrix", normal_matrix);
    }
};

class Wireframe:public RenderPass{
public:
    EdgeAccentuation2* edge2;
    // for directional light
    Wireframe(int render_width, int render_height, Shader* shader, EdgeAccentuation2* edge2) {
        create_framebuffer(&buffer,&texture,&depth_buffer,render_width,render_height);
        this->render_width = render_width;
        this->render_height = render_height;
        this->shader = shader;
        this->edge2 = edge2;
   }
    void render(Scene* scene) override{
        edge2->render(scene);
        glDepthMask(GL_FALSE);

        // reset render parameters
        // glDepthFunc(GL_LESS); 
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        glViewport(0, 0, render_width, render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        shader->Use();

        // render objects
        for (GLuint i = 0; i < scene->objects.size(); i++) {
            Object* object = &scene->objects[i];
            if (object->material == DUST) continue;
            set_object_uniforms(scene,object,i);
            object->draw();
        }
    }
private:
    void set_object_uniforms(Scene* scene, Object* obj, int i) {
        
        // reset texutred or dust status
        // glBindTexture(GL_TEXTURE_2D, 0);
        shader->set_uniform1f("textured",0.0);
        shader->set_uniform1i("dust",0);
        // obvious mvp matrixes
        glm::mat4 model_matrix = obj->model_matrix;
        shader->set_uniformMatrix4fv("modelMatrix", obj->model_matrix);
        glm::mat4 view_matrix = scene->camera.GetViewMatrix();
        shader->set_uniformMatrix4fv("viewMatrix", view_matrix);
        glm::mat4 projection_matrix = scene->projection_matrix;
        shader->set_uniformMatrix4fv("projectionMatrix", projection_matrix);
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix * model_matrix)));
        shader->set_uniformMatrix3fv("normalMatrix", normal_matrix);
    }
};

