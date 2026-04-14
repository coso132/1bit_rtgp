#pragma once

// using namespace std;

// Std. Includes
// #include <vector>
#include <utils/model.h>
#include <utils/shader.h>
#include <utils/camera.h>

enum Material {
    WOOD,
    something_else,
};


// data structure for scene objects
class Object {
public:
    // world coordinates
    glm::vec3 pos;
    // model of the object
    Model model;
    // shader program to render this object with
    // Shader shader;//not needed?
    Material material;
    // ... existing members ...

    // Object(const Object& other) = default;
};


/////////////////// SCENE class ///////////////////////
class Scene {
public:
    // we can have a shader for each material, and we can switch between them when rendering the objects
    static Shader woodShader;
    static Shader something_else_shader;
    static Shader materialShaders[];

    vector<Object*> objects;
    Camera camera;
    // vector<Light> lights;
    // Light light;

   // constructor
   Scene(vector<Object*> objects, Camera camera)
    :objects(objects), camera(camera) {
        //sort objects by material
        // std::sort(this->objects.begin(), this->objects.end(), [](const Object& a, const Object& b) {
        //     return a.material < b.material;
        // });
    }

    // rendering of the whole scene
    void Draw() {
        // we render the objects in the order of their material, so that we minimize the number of shader switches
        // (we can do this because we have sorted the vector of objects by material in the constructor)
        Material current_material = objects[0]->material;
        for (Object* object : objects) {
            if (object->material != current_material) {
                current_material = object->material;
                materialShaders[current_material].Use();
            }
            object->model.Draw();
        }
        
    }
    /*
    //TODO this bs is to be respect unfortunately </3
    // // We want Mesh to be a move-only class. We delete copy constructor and copy assignment
    // // see:
    // // https://docs.microsoft.com/en-us/cpp/cpp/constructors-cpp?view=vs-2019
    // // https://learn.microsoft.com/en-us/cpp/cpp/copy-constructors-and-copy-assignment-operators-cpp?view=msvc-170
    // // https://en.cppreference.com/w/cpp/language/copy_constructor
    // // https://en.cppreference.com/w/cpp/language/as_operator.html
    // // https://www.geeksforgeeks.org/preventing-object-copy-in-cpp-3-different-ways/
    // Mesh(const Mesh& copy) = delete; //disallow copy
    // Mesh& operator=(const Mesh&) = delete;

    // Constructor
    // We use initializer list and std::move in order to avoid a copy of the arguments
    // This constructor empties the source vectors (vertices and indices)
 

    //TODO this bs is to be respect unfortunately </3
    // // We implement a user-defined move constructor and move assignment
    // // see:
    // // https://docs.microsoft.com/en-us/cpp/cpp/move-constructors-and-move-assignment-operators-cpp?view=vs-2019
    // // https://en.cppreference.com/w/cpp/language/move_constructor
    // // https://en.cppreference.com/w/cpp/language/move_assignment
    // // https://www.learncpp.com/cpp-tutorial/15-3-move-constructors-and-move-assignment/

    // // Move constructor
    // // The source object of a move constructor is not expected to be valid after the move.
    // // In our case it will no longer imply ownership of the GPU resources and its vectors will be empty.
    // Mesh(Mesh&& move) noexcept
    //     // Calls move for both vectors, which internally consists of a simple pointer swap between the new instance and the source one.
    //     : vertices(std::move(move.vertices)), indices(std::move(move.indices)), VAO(move.VAO), VBO(move.VBO), EBO(move.EBO)
    // {
    //     move.VAO = 0; // We *could* set VBO and EBO to 0 too,
    //     // but since we bring all the 3 values around we can use just one of them to check ownership of the 3 resources.
    // }

    // // Move assignment
    // Mesh& operator=(Mesh&& move) noexcept
    // {
    //     // calls the function which will delete (if needed) the GPU resources for this instance
    //     freeGPUresources();

    //     if (move.VAO) // source instance has GPU resources
    //     {
    //         this->vertices = std::move(move.vertices);
    //         this->indices = std::move(move.indices);
    //         this->VAO = move.VAO;
    //         this->VBO = move.VBO;
    //         this->EBO = move.EBO;

    //         move.VAO = 0;
    //     }
    //     else // source instance was already invalid
    //     {
    //         this->VAO = 0;
    //     }
    //     return *this;
    // }

    // // destructor
    // ~Mesh() noexcept
    // {
    //     // calls the function which will delete (if needed) the GPU resources
    //     freeGPUresources();
    // }

    //////////////////////////////////////////
    */
private:

    // void freeGPUresources(){}
};

// Define static members outside the class
Shader Scene::woodShader = Shader("shaders/wood.vs", "shaders/wood.fs");
Shader Scene::something_else_shader = Shader("shaders/basic_normal.vert", "shaders/basic_normal.frag");
Shader Scene::materialShaders[] = {Scene::woodShader, Scene::something_else_shader};
