#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <GL/glew.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <IL/il.h> //devil image loader

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include <fstream>  //for shader loader
#include <sstream>  //for string stream object
#include <stdio.h>  //for c style i/o
#include <iostream>

#include <map> //map for loading/pairing texutres

//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
    GLfloat texture[2];
    GLfloat normal[3];
};

//Create an object for our lighting parameter, will make access easier later
struct Light {
  // Initialize shader lighting parameters
  //have w = 0 makes a directional source, 1 makes point source
  glm::vec4 light_position;
  glm::vec4 light_ambient;
  glm::vec4 light_diffuse;
  glm::vec4 light_specular;

  float constantAttenuation, linearAttenuation, quadraticAttenuation;
  float spotCutoff, spotExponent;
  glm::vec3 spotDirection;
};

//set the material properties for how light will reflect/refract
struct Material {
  glm::vec4 material_ambient;
  glm::vec4 material_diffuse;
  glm::vec4 material_specular;
  float  material_shininess;
};

class Model {
  public:
    Model();
    ~Model();

    //sets up shader program and pointers
    void setupRender(Light);
    void cleanupRender();
    void draw(glm::mat4 mvp);//actually performs the drawing, should be used for instancing
    bool init(const char* mesh, const aiScene* scene);
    unsigned int getNumVertices();
    unsigned int materialNumber; //to pick the correct material
    std::string shaderLoader(std::string file);

    //keep track of our geometry
    std::vector<Vertex> vertices;
    std::vector<unsigned int> Indices;
    //map for pairing textures
    std::map<std::string, GLuint> textureIdMap;

    GLuint program;
    GLuint vbo_geometry;

    //uniform locations
    GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader
    //attribute locations
    GLint loc_position;
    GLint loc_color;
    GLint loc_texture;
    GLint loc_normal;

    //texture vars
    GLsizei width;
    GLsizei height;
    GLuint  texId;
    ILubyte* texData;

    Material material;

  private:
    void cleanUp();
    bool initMaterials(const char* mesh, const aiScene* scene);

    unsigned int numVertices;
    unsigned int materialIndex;
};

class Scene{
  public:
     //keep an array of our meshes, each with their own encapsulated functionality
     std::vector<Model> meshes;
     //load meshes and initialize them
     bool initMeshes(const char* fileName);
     //draw all meshes in the scene
     void draw(glm::mat4 mvp, Light light);

  private:

};

#endif

