#pragma once

// #include <utils/model.h>
// #include <utils/shader.h>
// #include <utils/camera.h>
// #include <utils/light.h>
// #include <utils/sceneobject.h>
// #include <utils/misc.h>
// #include <utils/rendering.h>
#include <utils/scene.h>


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

        // render dependant bullshit
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

    LightingPass(int render_width, int render_height, Shader* lighting_shader, Shader* shadow_map_shader, int shadow_map_resolution): shadow_map(shadow_map_resolution, shadow_map_shader){
        create_framebuffer(&buffer,&texture,&depth_buffer,render_width,render_height);
        this->render_width = render_width;
        this->render_height = render_height;
        this->shader = lighting_shader;
   }
    void render(Scene* scene) override{
        // update shadowmap
        shadow_map.render(scene);
        // update spheres
        scene->update_blue_sphere_cubemap();
        scene->update_bayer_sphere_cubemap();

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
        GLuint dither_map = (scene->use_sphere_dithering?(obj->noise_type==BAYER? scene->bayer_sphere_tex : scene->blue_sphere_tex):(obj->noise_type==BAYER? scene->bayer_noise:scene->blue_noise));
        float dither_width = scene->use_sphere_dithering? 640 :(obj->noise_type==BAYER?16:64);
        float dither_height = scene->use_sphere_dithering? 360 :(dither_width);
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
    // for directional light
    Wireframe(int render_width, int render_height, Shader* shader) {
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

