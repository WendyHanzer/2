#include "model.h"

Model::Model(){
  material.material_ambient = glm::vec4( 0.1, 0.1, 0.1, 1.0 );
  material.material_diffuse = glm::vec4( 0.9, 0.9, 0.9, 1.0 );
  material.material_specular = glm::vec4( 0.9, 0.9, 0.9, 1.0 );
  material.material_shininess = 10.0;
}
  
Model::~Model(){
  cleanUp();
}

void Model::draw(glm::mat4 mvp){
    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    glDrawArrays(GL_TRIANGLES, 0, getNumVertices());//mode, starting index, count
}

void Model::setupRender(Light light){
    //enable the shader program
    glUseProgram(program);

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glEnableVertexAttribArray(loc_normal);
  
    //for texture loading
    glEnableVertexAttribArray(loc_texture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry); 
    //set pointers into the vbo for each of the attributes(position, color, texture
    //                                                     and normal)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));
    glVertexAttribPointer( loc_normal,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,normal));

    glVertexAttribPointer( loc_texture,
                           2,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,texture));

      //rebind our textures for the current object
      glBindTexture(GL_TEXTURE_2D, texId); 
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
                   height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                   texData);

  // Initialize shader lighting parameters
  glm::vec4 ambient_product = light.light_ambient * material.material_ambient;
  glm::vec4 diffuse_product = light.light_diffuse * material.material_diffuse;
  glm::vec4 specular_product = light.light_specular * material.material_specular;

  //assign the parameters to their variables in the shader program
  glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 
                1, glm::value_ptr(ambient_product) );
  glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 
                1, glm::value_ptr(diffuse_product) );
  glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 
                1, glm::value_ptr(specular_product) );
  glUniform4fv( glGetUniformLocation(program, "LightPosition"), 
                1, glm::value_ptr(light.light_position) );
  glUniform1f( glGetUniformLocation(program, "Shininess"), material.material_shininess );

  //added for spot lights
  glUniform3fv( glGetUniformLocation(program, "spotDirection"), 
			 1, glm::value_ptr(light.spotDirection));
  glUniform1f( glGetUniformLocation(program, "constantAttenuation"), light.constantAttenuation);
  glUniform1f( glGetUniformLocation(program, "linearAttenuation"), light.linearAttenuation);
  glUniform1f( glGetUniformLocation(program, "quadraticAttenuation"), light.quadraticAttenuation);
  glUniform1f( glGetUniformLocation(program, "spotCutoff"), light.spotCutoff);
  glUniform1f( glGetUniformLocation(program, "spotExponent"), light.spotExponent);

}

void Model::cleanupRender(){
    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
    glDisableVertexAttribArray(loc_texture);
    glDisableVertexAttribArray(loc_normal);
}

bool Model::init(const char* fileName, const aiScene* scene){
  //save our number of vertices so we know how many to draw later
  numVertices = vertices.size();

  //set up where geometry lives for GL to draw
  glGenBuffers(1, &vbo_geometry);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
  // --Geometry Done

  //setup textures
  initMaterials(fileName, scene); //mesh if our file name, scene is our aiScene


  //set up our shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    // --Shader loader completed, returns string with the shader file
    // Note the added uniform!
    std::string tempString1 = shaderLoader("assets/shaders/vertex-shader");
    const char *vs = tempString1.c_str();

    std::string tempString2 = shaderLoader("assets/shaders/fragment-shader");
    const char *fs = tempString2.c_str();

    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);

    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status){
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1){
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1){
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_normal = glGetAttribLocation(program, 
                     const_cast<const char*>("v_normal"));
    if (loc_normal == -1){
        std::cerr << "[F] v_normal not found \n";
     }

    //add textures
    loc_texture = glGetAttribLocation(program,
                    const_cast<const char*>("texcoord"));
    if(loc_texture == -1){
        std::cerr << "[F] texcoord NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1){
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }



  return true;
}

bool Model::initMaterials(const char* fileName, const aiScene* scene){

  ILboolean success;

  /* initialization of DevIL */
  ilInit(); 

  //debug line to confirm material number
  //std::cout << "material number: " << materialNumber << "\n";

  //debug line to check number of materials
  //std::cout << "Materials found " << materialNumber << "\n";

    int texIndex = 0;
    aiString path;	// filename

    aiReturn texFound = scene->mMaterials[materialNumber]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);

    //very strange, but texFound seems to contain the opposite of what we'd expect
    if (texFound) {
      //std::cout << "No texture image found \n  Using default texture";
      textureIdMap["assets/models/textures/default.jpg"] = 0;
      //std::cout <<  path.data << "\n\n";
    }
    else {
      //fill map with textures, OpenGL image ids set to 0

      // Extract the file name out of the file path
      std::string file = path.data;
      std::string::size_type SlashIndex = file.find_last_of("/");
      file = file.substr(SlashIndex+1);
      //and prepend the directory we want to find texture images
      file.insert(0, "assets/models/textures/");

      textureIdMap[file.c_str()] = 0; 
    }

  //because we only have 1 texture per mesh, probably don't need this
  //given more time can strip this down to load our 1 texture for our object
  int numTextures = textureIdMap.size();

  /* create and fill array with DevIL texture ids */
  ILuint* imageIds = new ILuint[numTextures];
  ilGenImages(numTextures, imageIds); 

  /* create and fill array with GL texture ids */
  GLuint* textureIds = new GLuint[numTextures];
  glGenTextures(numTextures, textureIds); /* Texture name generation */

  /* get iterator */
  std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
  int i=0;
  for (; itr != textureIdMap.end(); ++i, ++itr)	{
    //save IL image ID
    std::string filename = (*itr).first;  // get filename
    (*itr).second = textureIds[i];	  // save texture id for filename in map

    ilBindImage(imageIds[i]); /* Binding of DevIL image name */
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 
    success = ilLoadImage((ILstring)filename.c_str());

    if (success) {
      /* Convert image to RGBA */
      ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 

      //save our texture data to be bound later
      width = ilGetInteger(IL_IMAGE_WIDTH);
      height = ilGetInteger(IL_IMAGE_HEIGHT); 
      texId = textureIds[i];
      ILubyte* texDataBytes = ilGetData();
      texData = new ILubyte [ilGetInteger(IL_IMAGE_SIZE_OF_DATA)];
      for (int i=0; i < ilGetInteger(IL_IMAGE_SIZE_OF_DATA); i++){
        texData[i] = texDataBytes[i];
      }
    }
    else 
    printf("Couldn't load Image: %s\n  Please make sure all texture images are located in assets/models/textures/ folder!", filename.c_str());
  }
  /* Because we have already copied image data into texture data
	we can release memory used by image. */
  ilDeleteImages(numTextures, imageIds); 

  //Cleanup
  delete [] imageIds;
  delete [] textureIds;

  return true;
}

void Model::cleanUp(){
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);

}

std::string Model::shaderLoader(std::string file){
  std::ifstream fin (file.c_str());
  std::string fileContent = "";

  if ( fin ) {
    std::stringstream buffer;
    buffer << fin.rdbuf();

    fin.close();

    fileContent = buffer.str();
    return fileContent;
  }
  else {
    std::cout << "Error: File not found";
    return fileContent;
  }
}

unsigned int Model::getNumVertices(){

  return numVertices;
}

bool Scene::initMeshes(const char* fileName) {

   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile(fileName, 0x8); 

   if (!scene){
     std::cout << "Error!  Failed to load scene object. \n  Either unsuspected format or file not found.\n\n";
     return false;
   }

   //get the first model in the scene
   for (unsigned int i=0; i < scene->mNumMeshes; i++) {
     //create a new instance of the model mesh, that will be pushed into the meshes vector
     Model modelMesh;

     //debug line to see how many meshes encountered
     //std::cout << "Loading mesh #: " << i << '\n';

     const aiMesh* model = scene->mMeshes[i];
     if (!model){
       std::cout << "Error!  Failed to load model\n  Unreadable format\n\n";
       return false;
     }
	
	//Loads the triangluated faces
	for (unsigned int i = 0 ; i < model->mNumFaces ; i++) {
		const aiFace& Face = model->mFaces[i];
	     modelMesh.Indices.push_back(Face.mIndices[0]);
		modelMesh.Indices.push_back(Face.mIndices[1]);
		modelMesh.Indices.push_back(Face.mIndices[2]);
	}


     //get the models vertices
     for (unsigned int i = 0; i < modelMesh.Indices.size(); i++){
	  int idx = modelMesh.Indices[i];
       const aiVector3D* pos = &(model->mVertices[idx]);

       Vertex t = { {float(pos->x), float(pos->y), float(pos->z)}, {0.6, 0.6, 0.6}, {0.0, 0.0}, {0.0, 0.0, 0.0} };

       const aiVector3D* texCoord;
       if (model->HasTextureCoords(0)){
         texCoord = &(model->mTextureCoords[0][idx]);
         //debug line to confirm texture coords
         //std::cout << "X: " << texCoord->x << ", Y: " << texCoord->y << ", Z: " << texCoord->z << "\n"; 
         t.texture[0] = texCoord->x;
         t.texture[1] = texCoord->y;
       }

       //if the model has normals, load them
       if (model->HasNormals()){
         const aiVector3D* norm   = &(model->mNormals[idx]);
         t.normal[0] = float(norm->x);
         t.normal[1] = float(norm->y);
         t.normal[2] = float(norm->z);
       }

       //put the vertice info into the Model object
       modelMesh.vertices.push_back(t);
       modelMesh.materialNumber = model->mMaterialIndex;
     }

     //put the model object into our Scene object
     meshes.push_back(modelMesh);


   }
    for( unsigned int i=0; i<meshes.size();i++ ){
     //initialize that mesh we just pushed
     meshes[i].init(fileName, scene);
    }

  return true;
}

void Scene::draw(glm::mat4 mvp, Light light){
  for (unsigned int i=0; i < meshes.size(); i++){
    //std::cout << "Drawing mesh: " << i << "\n";
    //setup pointers and shader program 
    meshes[i].setupRender(light);

    //draw all instances of object
    meshes[i].draw (mvp);

    //clean up after ourselves
    meshes[i].cleanupRender();
  }
}
