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

#include <bullet/btBulletDynamicsCommon.h>

#include "model.h" 


//--Evil Global variables
//Just for this example! 
int w = 640, h = 480;// Window size
std::string rotationDirection;
char *modelString0, *modelString1, *modelString2, *modelString3; 

Scene modelScene0, modelScene1, modelScene2, modelScene3; //where the model action goes down

//transform matrices
glm::mat4 model0, model1, model2, model3;//obj->world each object should have its own model matrix
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
bool initialize();
//void cleanUp(); 

//--Model Manipulation
glm::mat4 updateModel(int=0);
void cubeControls(int); 

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//-- Setup physics stuff --//
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);

	//keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

 
//--Main
int main(int argc, char **argv)
{
    //check if command line arguments provided
    if (!argv[4]){
      std::cout << "[F] Please provide 4 model file names!\n\n";
      return -1;
    }
  
    int size = std::strlen(argv[1]);
    modelString0 = new char [size+1];
    std::strcpy(modelString0, argv[1]);

    size = std::strlen(argv[2]);
    modelString1 = new char [size+1];
    std::strcpy(modelString1, argv[2]);  

    size = std::strlen(argv[3]);
    modelString2 = new char [size+1];
    std::strcpy(modelString2, argv[3]);

    size = std::strlen(argv[4]);
    modelString3 = new char [size+1];
    std::strcpy(modelString3, argv[4]);

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

    //try to setup our physics stuff
		
    //set gravity into our world
	  dynamicsWorld->setGravity(btVector3(0,-10,0));


	///create a few basic rigid bodies
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(5.),btScalar(0.01),btScalar(3.)));

	collisionShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0,-2,0));

	{
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRestitution(btScalar(1.0));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}


	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(3.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,1.,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

			startTransform.setOrigin(btVector3(0,10,0));
		
			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			//body->setLinearVelocity(localInertia);
			body->setRestitution(btScalar(1.));

			dynamicsWorld->addRigidBody(body);
	}

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
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
    mvp = projection * view * model0;

    modelScene0.draw(mvp);

		mvp = projection * view * model1;

		modelScene1.draw(mvp);

    mvp = projection * view * model2;
    modelScene2.draw(mvp);

    mvp = projection * view * model3;
    modelScene3.draw(mvp);
                           
    //swap the buffers
    glutSwapBuffers();
}
 
void update()
{ 
    //model = updateModel();
    //model1 = glm::translate( glm::mat4(1.0f), glm::vec3(0, 5, 1));

  //physic stuff
  dynamicsWorld->stepSimulation(1.f/60.f,10);
		
		//print positions of all objects
		for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				btTransform trans;
				body->getMotionState()->getWorldTransform(trans);
				printf("world pos = %f,%f,%f\n",float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
				if (j==1){
					//transform ball
					model0 = glm::translate( glm::mat4(1.0f), glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));

				}
				else{
					//transform ground
					model1 = glm::translate( glm::mat4(1.0f), glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()))) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
				}
			}
		}

    //temporary for models
		model2 = glm::mat4(1.0f);
    model3 = glm::mat4(1.0f);

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

bool initialize() 
{
    //load the model, setup shaders
    modelScene0.initMeshes(modelString0); 
		modelScene1.initMeshes(modelString1);
		modelScene2.initMeshes(modelString2);
		modelScene3.initMeshes(modelString3);

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

	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1];
	btRigidBody* body = btRigidBody::upcast(obj);
	btVector3 vec = body->getLinearVelocity();

  switch(key) {
  case GLUT_KEY_LEFT:
    //updateModel(2);
    vec += btVector3(5, 0, 0);
    body->setLinearVelocity(vec);
    break;

  case GLUT_KEY_RIGHT:
    //updateModel(2);
    vec += btVector3(-5, 0, 0);
    body->setLinearVelocity(vec);
    break;
  case GLUT_KEY_UP:
    vec += btVector3(0, 0, 5);
    body->setLinearVelocity(vec);
    break;
  case GLUT_KEY_DOWN:
    vec += btVector3(0, 0, -5);
    body->setLinearVelocity(vec);
    break;
}
}

void mouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    updateModel(2);
  }
}
