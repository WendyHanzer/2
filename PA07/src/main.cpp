#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include <fstream>  //for shader loader
#include <sstream> //for string
#include <stdio.h> //for c style i/o
#include <vector> 
#include <GL/freeglut.h> 

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h> 
#include <assimp/color4.h> 

#include "model.h" 

//--Evil Global variables  
//Just for this example! 
int w = 640, h = 480;// Window size
std::string rotationDirection;
char *modelString, *modelString1; 

Scene modelScene, modelMesh1; //where the model action goes down

//transform matrices
glm::mat4 model, model1;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye 
glm::mat4 projection;//eye->clip 
glm::mat4 mvp;//premultiplied modelviewprojection
 
//--GLUT Callbacks
void render(); 
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);
void specialInput(int key, int x, int y);

//--Resource management
bool initialize(char*);
//void cleanUp(); 

//--Model Manipulation
glm::mat4 updateModel(int=0);
void cubeControls(int); 

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

 
//--Main
int main(int argc, char **argv)
{
    //check if command line arguments provided
    if (!argv[1]){
      std::cout << "[F] Please provide model file name!\n\n";
      return -1;
    }
  
    int size = std::strlen(argv[1]);
    modelString = new char [size+1];
    std::strcpy(modelString, argv[1]);

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(NULL);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutMouseFunc(mouse);//called when the mouse is depressed or released

    glutSpecialFunc(specialInput); //called for special input, ie arrow keys for this project

    //Create our simple menu
    glutCreateMenu(cubeControls);
    glutAddMenuEntry("quit", 1);
    glutAddMenuEntry("start rotation", 2);
    glutAddMenuEntry("stop rotation", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(argv[1]);
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    //modelMesh.cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;

    modelScene.draw(mvp);
                           
    //swap the buffers
    glutSwapBuffers();
}
 
void update()
{ 
    model = updateModel();
    model1 = glm::translate( glm::mat4(1.0f), glm::vec3(0, 5, 1));

    // Update the state of the scene 
    glutPostRedisplay();//call the display callback
}  

glm::mat4 updateModel(int val){
    //total time
    static float angleTrans = 0.0, angleRot = 0.0;
    float dt = getDT();// if you have anything moving, use dt.
    static float rotVal = -1.0, lastDT = dt, rotAngMult = 100.0;//need lastDT to restore from pause
    static bool clockwise = true, clockwiseRotate = true, rotate = true;
    float transX, transY, transZ;

    //switch to manipulate rotation and movement 
    switch (val) {
      case 1: //change translation to opposite direction
        if (clockwise){
          clockwise = false;
        }
       else{
          clockwise = true;
        } 
        break;
        
      case 2: //rotate cube in opposite direction
        if (clockwiseRotate)
          clockwiseRotate = false;  
        else 
          clockwiseRotate = true;
        break;

      case 3: dt = lastDT; //don't want the current dt because it is unknown how long program was paused
        break;

      case 4: rotate = false; //stop rotation
        break;

      case 5: rotate = true; //restart rotation
        break;
    }
 

    if(clockwise) 
      angleTrans += dt * M_PI/2;  
    else
      angleTrans -= dt * M_PI/2;
    
    if (rotate && clockwiseRotate)
      angleRot += dt * M_PI/2;//move through 90 degrees a second
    else if (rotate)
      angleRot -= dt * M_PI/2;

    transX = 4.0 * cos(angleTrans);
    transY = 0.0;
    transZ = 4.0 * sin(angleTrans);
    
    //perform translation (move the model)
    glm::mat4 translation = glm::translate( glm::mat4(1.0f), glm::vec3(transX, transY, transZ));

    //perform rotation of the world(rotate the model)
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angleRot * rotAngMult, glm::vec3(0.0f,rotVal,1.0f));

    //combine the two, first performing the rotation, then the translation on the world
   return translation * rotation;
}

void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

bool initialize(char* fname) 
{
    //load the model, setup shaders
    //modelScene.initMeshes(modelString); 
    //load the model, setup shaders
    modelScene.initMeshes(fname); 

    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}

//Set up the menu options
void cubeControls(int id){
  switch(id)
  {
    case 1:
      exit(0);
      break;
    case 2:
      updateModel(3);
      glutIdleFunc(update);
      break;
    case 3:
      glutIdleFunc(NULL);
      break;
  }

  glutPostRedisplay();
} 

//keyboard input stuff - triggers actions for the model such as start rotation, etc.
void keyboard(unsigned char key, int x_pos, int y_pos)
{
  // Handle keyboard input
  switch (key) {
  case 27: 
  case 'q':
  case 'Q': //exit the program
    exit (0);
    break;

  case 'r':
  case 'R': //change rotation
    updateModel(1);
    glutPostRedisplay();
    break;

  case 'e':
  case 'E': //change orbit of cube
    updateModel(2);
    glutPostRedisplay();
    break;

  case 'd':
  case 'D': //play animation
    updateModel(3);
    glutIdleFunc(update);
    break;
 
  case 'f':
  case 'F': //pause animation
    glutIdleFunc(NULL);
    break;

  case 'a':
  case 'A': //pause cube's rotation
    updateModel(4);
    break;

  case 's':
  case 'S': //restart cube rotation
    updateModel(5);
    break;
  }
}

void specialInput (int key, int x, int y) {
  switch(key) {
  case GLUT_KEY_LEFT:
    updateModel(2);
    break;

  case GLUT_KEY_RIGHT:
    updateModel(2);
    break;
}
}

void mouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    updateModel(2);
  }
}
