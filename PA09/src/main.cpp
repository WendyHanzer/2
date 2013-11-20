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
#include "GameStat.h"

//--Evil Global variables
//Just for this example! 
int w = 640, h = 480;// Window size
std::string rotationDirection;

Scene modelScene0, modelScene1, modelScene2, modelScene3, modelScene4, modelScene5; //where the model action goes down
Scene modelSceneText[9];
int scoreP1 = 0, scoreP2 = 0;
int mousex, mousey;

glm::vec3 eyePosition = glm::vec3(0, 30.1, -0.1);
//transform matrices
glm::mat4 model0, model1, model2, model3, modelTextP1, modelTextP2;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye 
glm::mat4 projection;//eye->clip 
glm::mat4 mvp;//premultiplied modelviewprojection
 
//--GLUT Callbacks
void render(); 
void update();
void reshape(int n_w, int n_h);
void keydown(unsigned char key, int x_pos, int y_pos);
void keyup(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);
void specialInput(int key, int x, int y);
void specialUpInput(int key, int x, int y);
void passive(int x, int y);

//--Resource management
bool initialize();
//void cleanUp(); 

//--Model Manipulation
void cubeControls(int); 

// keyboard input
bool* keyspushed = new bool[256];
bool* specialkeypushed = new bool[256];
void keyOperations();
void specialKeyOperations();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Check if we have a score
bool checkGoal (btTransform trans);
void moveView(float dir, bool xy, bool z);

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

	GameStat* v1;
 
//--Main
int main(int argc, char **argv)
{
	
	/**
		ADD THE FOLLOWING TO MASTER
	*/
	char* n1;
	char* n2;
	n1 = new char[std::strlen(argv[1])+1];
	n2 = new char[std::strlen(argv[2])+1];
	std::strcpy(n1, argv[1]);
	std::strcpy(n2, argv[2]);
	v1 = new GameStat(n1, n2);
	v1->read();
	v1->draw();
    modelTextP1 = glm::translate(glm::mat4(1.0), glm::vec3(15., 0, 12)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));
    modelTextP2 = glm::translate(glm::mat4(1.0), glm::vec3(13., 0, 12)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Air Hockey");

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
    glutKeyboardFunc(keydown);// Called if there is keyboard input
    glutKeyboardUpFunc(keyup);
    glutMouseFunc(mouse);//called when the mouse is depushed or released
    glutPassiveMotionFunc(passive);

    glutSpecialFunc(specialInput); //called for special input, ie arrow keys for this project
    glutSpecialUpFunc(specialUpInput);

    //Create our simple menu
    glutCreateMenu(cubeControls);
    glutAddMenuEntry("quit", 1);
    glutAddMenuEntry("resume game", 2);
    glutAddMenuEntry("pause game", 3);
    glutAddMenuEntry("restart", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    //modelMesh.cleanUp();

	
		/**
			ADD THE FOLLOWING TO MASTER
		*/
	v1->save();
	v1->draw();
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

    mvp = projection * view * modelTextP1;
    modelSceneText[scoreP1].draw(mvp);

    mvp = projection * view * modelTextP2;
    modelSceneText[scoreP2].draw(mvp);
                           
    //swap the buffers
    glutSwapBuffers();
}
 
void update()
{ 
    //handle keyboard input
    keyOperations();
    specialKeyOperations();
    //model = updateModel();
    //model1 = glm::translate( glm::mat4(1.0f), glm::vec3(0, 5, 1));
  static float dt = 0;
  dt += getDT();
  //std::cout << "\n" << dt;

  //to prevent from running at machine speed, check dt and don't update models unless .01 seconds have passed
  if (dt < 0.01)
    return;
  
  dt = 0; //reset dt

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
				//printf("%i: world pos = %f,%f,%f\n", j, float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
				if (j==1){
					//transform paddle left
					model0 = glm::translate( glm::mat4(1.0f), glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
          modelTextP1 = model0;
					modelTextP1 = glm::translate(modelTextP1, glm::vec3(0, 3.1, 0));

				}
        else if (j==2){
					//transform paddle right
					model3 = glm::translate( glm::mat4(1.0f), glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
          modelTextP2 = model3;
					modelTextP2 = glm::translate(modelTextP2, glm::vec3(0, 3.1, 0));
        }
        else if (j==3){
          //transform puck
					if (checkGoal(trans)){
	  				btTransform puckTrans;
						puckTrans.setIdentity();
	  				puckTrans.setOrigin(btVector3(0,-1,0));
						btDefaultMotionState* puckMotionState = new btDefaultMotionState(puckTrans);
						body->setMotionState(puckMotionState);//->setWorldTransform(puckTrans);
						body->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
					}
					model2 = glm::translate( glm::mat4(1.0f), glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
					//check goal
        }
				else{
					//transform ground (can probably get rid of this)
					model1 = glm::translate( glm::mat4(1.0f), glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));//don't scale the board * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
				}
			}
		}

    // Update the state of the scene 
    glutPostRedisplay();//call the display callback
}  

bool checkGoal (btTransform trans){
	float goal = float(trans.getOrigin().getX());

	if (goal > 15.5 && scoreP2 < 8){
		std::cout << "Goal for player on right!\n";
        scoreP2++;
		/**
			ADD THE FOLLOWING TO MASTER
		*/
		v1->scored(false);
		return true;
    }

	else if(goal < -15.5 && scoreP1 < 8){
		std::cout << "Goal for player on left!\n";
        scoreP1++;
		
		/**
			ADD THE FOLLOWING TO MASTER
		*/
		v1->scored(true);
		return true;
	}

	return false;
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


    //try to setup our physics stuff
    #define BIT(x) (1<<(x))
    enum collisiontypes {
      COLLIDE_NOTHING = 0,
      COLLIDE_MIDDLE = BIT(0), //<collide with wall in middle
      COLLIDE_GOAL = BIT(1), //collide with entire wall of goal
      COLLIDE_GOAL_WALL = BIT(2), //collide with goal walls but not goal itself
      COLLIDE_EVERYTHING_ELSE = BIT(3)
    };

    int paddleCollideWith = COLLIDE_MIDDLE | COLLIDE_GOAL | COLLIDE_EVERYTHING_ELSE;
    int puckCollideWith = COLLIDE_GOAL_WALL | COLLIDE_EVERYTHING_ELSE; 

    //set gravity into our world
	  dynamicsWorld->setGravity(btVector3(0,-10,0));

	///create a few basic rigid bodies

	{
  		//create our ground
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(15.5),btScalar(0.01),btScalar(8.5)));
		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0,-2,0));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));
		body->setFriction(btScalar(10));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_EVERYTHING_ELSE, puckCollideWith);
	}

	{
		//create paddle on right

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(100.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,1.,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

		startTransform.setOrigin(btVector3(5,1,0));
		
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);

		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRestitution(btScalar(.5));
		body->setFriction(btScalar(100));
		body->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addRigidBody(body, COLLIDE_EVERYTHING_ELSE, paddleCollideWith);
	}

	{
		//create paddle on left

		//btCollisionShape* colShape = new btBoxShape(btVector3(.5,1,.5));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(100.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,1.,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

			startTransform.setOrigin(btVector3(-5,1,0));
		
			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			//body->setLinearVelocity(localInertia);
			body->setRestitution(btScalar(1.));
			body->setFriction(btScalar(100));
			body->setActivationState(DISABLE_DEACTIVATION);

			dynamicsWorld->addRigidBody(body, COLLIDE_EVERYTHING_ELSE, paddleCollideWith);
	}

	{
	  //create puck
	  btCollisionShape* groundShape = new btSphereShape(btScalar(1.));

	  collisionShapes.push_back(groundShape);

    //add our static box
	  btTransform groundTransform;
	  groundTransform.setIdentity();
	  groundTransform.setOrigin(btVector3(0,-1,0));

		btScalar mass(1.);

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
		body->setFriction(btScalar(0));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_EVERYTHING_ELSE, puckCollideWith);
	}
    // bottom wall
	{

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(15.5),btScalar(10.01),btScalar(0.1)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0,0,-9.5));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_EVERYTHING_ELSE, puckCollideWith);
	}
    // top wall
	{

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(15.5),btScalar(10.01),btScalar(0.1)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0,0,9.5));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_EVERYTHING_ELSE, puckCollideWith);
	}
    // side wall
	{ //goal right side?

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(10.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(-15.,0,0));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_GOAL, paddleCollideWith);
	}
    // side wall
	{ //goal left side

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(10.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(15.,0,0));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_GOAL, paddleCollideWith);
	}

// try to create middle wall with collision filtering
	{

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(10.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0.,0,0));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_MIDDLE, paddleCollideWith);
	}

	{ //set up our goals

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(3.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(15.,0,6.));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_GOAL_WALL, puckCollideWith);
	}

	{ 

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(3.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(15.,0,-6.));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_GOAL_WALL, puckCollideWith);
	}

	{

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(3.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(-15.,0,6.));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_GOAL_WALL, puckCollideWith);
	}

	{ 

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.01),btScalar(13.5),btScalar(3.5)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(-15.,0,-6.));
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

    //set the "springiness of our object
		body->setRestitution(btScalar(0.5));

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body, COLLIDE_GOAL_WALL, puckCollideWith);
	}

    //load the model, setup shaders
    modelScene1.initMeshes("assets/models/hockeytable.obj"); 
	modelScene0.initMeshes("assets/models/hockeycirclepaddle.obj");
	modelScene3.initMeshes("assets/models/hockeycirclepaddle.obj");
	modelScene2.initMeshes("assets/models/hockeypuck.obj");
	modelScene4.initMeshes("assets/models/hockeyrectanglepaddle.obj");
	modelScene5.initMeshes("assets/models/hockeytpuck.obj");
  modelSceneText[0].initMeshes("assets/models/zero.obj");
  modelSceneText[1].initMeshes("assets/models/one.obj");
  modelSceneText[2].initMeshes("assets/models/two.obj");
  modelSceneText[3].initMeshes("assets/models/three.obj");
  modelSceneText[4].initMeshes("assets/models/four.obj");
  modelSceneText[5].initMeshes("assets/models/five.obj");
  modelSceneText[6].initMeshes("assets/models/six.obj");
  modelSceneText[7].initMeshes("assets/models/seven.obj");
  modelSceneText[8].initMeshes("assets/models/winner.obj");

    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( eyePosition, //Eye Position
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
     // updateModel(3);
      glutIdleFunc(update);
      break;
    case 3:
      glutIdleFunc(NULL);
      break;
  }

  glutPostRedisplay();
} 

//keyboard input stuff - triggers actions for the model such as start rotation, etc.
void keydown(unsigned char key, int x_pos, int y_pos)
{
		static bool playing = false;
    keyspushed[key] = true;
/*
	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[2];
	btRigidBody* body = btRigidBody::upcast(obj);
	btVector3 vec = body->getLinearVelocity();

  // Handle keyboard input
*/
  switch (key) {
/*
  case 27: 
  case 'q':
  case 'Q': //exit the program

    exit (0);
    break;

  case 'd':
  case 'D': //play animation
    vec += btVector3(-5, 0, 0);
    body->setLinearVelocity(vec);
    break;

  case 'a':
  case 'A': //pause cube's rotation
    vec += btVector3(5, 0, 0);
    body->setLinearVelocity(vec);
    break;

  case 's':
  case 'S': //restart cube rotation
    vec += btVector3(0, 0, -5);
    body->setLinearVelocity(vec);
    break;

  case 'w':
  case 'W':
    vec += btVector3(0, 0, 5);
    body->setLinearVelocity(vec);
    break;
*/
  case 'f':
  case 'F': //pause animation	
		if (playing){
			glutIdleFunc(NULL);
			playing = false;
		}
		else{
    	glutIdleFunc(update);
			playing = true;
		}
    break;
  }

}
void keyup(unsigned char key, int x_pos, int y_pos)
{
    keyspushed[key] = false;
}

void specialInput (int key, int x, int y) {


    specialkeypushed[key] = true;
/*
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
*/
}
void specialUpInput(int key, int x, int y)
{
    specialkeypushed[key] = false;
}

void keyOperations()
{
	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[2];
	btRigidBody* body = btRigidBody::upcast(obj);
	btVector3 vec = body->getLinearVelocity();

    if( vec.y() > 1 ) vec *= btVector3(1, 0, 1);

    if( keyspushed['q'] || keyspushed['Q'] || keyspushed[27] )
        exit(0);
/*
    if( keyspushed['d'] || keyspushed['D'] )
    {
        if( vec.x() > -30 )
        {
            vec += btVector3(-5, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( !keyspushed['d'] && !keyspushed['D'] && !keyspushed['a'] && !keyspushed['A'] )
    {
       
            vec *= btVector3(0, 1, 1);
            body->setLinearVelocity(vec);
       
    }
    if( keyspushed['a'] || keyspushed['A'] )
    {
        if( vec.x() < 30 )
        {
            vec += btVector3(5, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( keyspushed['s'] || keyspushed['S'] )
    {
        if( vec.z() > -30 )
        {
            vec += btVector3(0, 0, -5);
            body->setLinearVelocity(vec);
        }
    }
    if( !keyspushed['s'] && !keyspushed['S'] && !keyspushed['w'] && !keyspushed['W'] )
    {
            vec *= btVector3(1, 1, 0);
            body->setLinearVelocity(vec);
    }
    if( keyspushed['w'] || keyspushed['W'] )
    {
        if( vec.z() < 30 )
        {
            vec += btVector3(0, 0, 5);
            body->setLinearVelocity(vec);
        }
    }
*/
    if ( keyspushed['u'] || keyspushed['U']) {
	    moveView(1,false, false);
    }
    if (keyspushed['j'] || keyspushed['J']) {
	    moveView(1, false, true);
    }
    if (keyspushed['i'] || keyspushed['I']){
      moveView (1.0, true, false);
    }
    if (keyspushed['k'] || keyspushed['K']){
      moveView (-1.0, true, false);
    }
    if (keyspushed['o'] || keyspushed['O']){
      moveView (-1.0, false, false);
    }
    if (keyspushed['l'] || keyspushed['L']){
      moveView (-1.0, false, true);
    }
    if (keyspushed['p'] || keyspushed['P']){
     	moveView(1, true, true);
    }
}
void specialKeyOperations()
{
	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1];
	btRigidBody* body = btRigidBody::upcast(obj);
  btVector3 vec = body->getLinearVelocity();

    if( vec.y() > 1 ) vec *= btVector3(1, 0, 1);

    if( specialkeypushed[GLUT_KEY_LEFT] )
    {
        if( vec.x() < 30 )
        {
            vec += btVector3(5, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( !specialkeypushed[GLUT_KEY_LEFT] && !specialkeypushed[GLUT_KEY_RIGHT] )
    {
            vec *= btVector3(0, 1, 1);
            body->setLinearVelocity(vec);
    }
    if( specialkeypushed[GLUT_KEY_RIGHT] )
    {
        if( vec.x() > -30 )
        {
            vec += btVector3(-5, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( specialkeypushed[GLUT_KEY_UP] )
    {
        if( vec.z() < 30 )
        {
            vec += btVector3(0, 0, 5);
            body->setLinearVelocity(vec);
        }
    }
    if( !specialkeypushed[GLUT_KEY_UP] && !specialkeypushed[GLUT_KEY_DOWN] )
    {
            vec *= btVector3(1, 1, 0);
            body->setLinearVelocity(vec);
    }
    if( specialkeypushed[GLUT_KEY_DOWN] )
    {
        if( vec.z() > -30 )
        {
            vec += btVector3(0, 0, -5);
            body->setLinearVelocity(vec);
        }
    }
}

void mouse(int button, int state, int x, int y) {

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    //updateModel(2);
  }

  
}

void moveView(float dir, bool xy, bool xz){
		float dt = 0;
		dt = getDT();
		static float angleXY = 90.0;
		static float angleXZ = 0.0;
		//default camera distance
		static float dis = 30.0;
		//t t is reset, f f is zoom
		//f t is rotate, t f is switch sort of
		if (xz && xy){
		        angleXY = 90.0;
			   angleXZ = 0.0;
			   dis = 30.0;
      		   
		} else {
			if (xz){
				angleXZ += dir * dt * M_PI/2;
				if (angleXZ > 360){
					angleXZ = 0;
				}
				
			}
			if (xy){
				angleXY += dir * dt * M_PI/2;
				if (angleXY > 360){
					angleXY = 0;
				}
				
				
			}
	
		}
		
		if (!xy && !xz){
			//dt makes everything smooth like ... (mad lib dirty noun here)
			if (dis < 60 && dir == 1.0){
				dis += dir * dt * M_PI * 2;
			}
			if (dis > 5 && dir == -1.0){
				dis += dir * dt * M_PI * 2;
			} 
		}
		//think unit circle and trig
		float radius = dis * cos(angleXY);
		float height = dis * sin(angleXY);
		eyePosition = glm::vec3(radius * cos(angleXZ),  height, radius * sin(angleXZ));
		if (xz && xy){
			eyePosition = glm::vec3(0.0,  30.0, -0.1);
		}
		    		
		view = glm::lookAt( eyePosition, //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
    
    std::cout << eyePosition.x << " " << eyePosition.y << " " << eyePosition.z << '\n';
}

void passive(int x, int y)
{
	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[2];
	btRigidBody* body = btRigidBody::upcast(obj);
	btVector3 vec = body->getLinearVelocity();

    if( vec.y() > 1 )
    {
        vec *= btVector3(1, 0, 1);
        body->setLinearVelocity(vec);
    }
	vec = body->getLinearVelocity();
    if( mousex - x < 0)
    {
        if( vec.x() > -30 && vec.x() < 30 )
        {
            vec += btVector3(-5, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( mousex - x > 0)
    {
        if( vec.x() > -30 && vec.x() < 30 )
        {
            vec += btVector3(5, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( mousex-x == 0 )
    {
        vec *= btVector3(0,1,1);
        body->setLinearVelocity(vec);
    }
	vec = body->getLinearVelocity();
    if( y - mousey < 0 )
    {
        if( vec.z() > -30 && vec.z() < 30 )
        {
            vec += btVector3(0, 0, 5);
            body->setLinearVelocity(vec);
        }
    }
    if( y - mousey > 0 )
    {
        if( vec.z() > -30 && vec.z() < 30 )
        {
            vec += btVector3(0, 0, -5);
            body->setLinearVelocity(vec);
        }
    }
    if( mousey-y == 0 )
    {
        vec *= btVector3(1,1,0);
        body->setLinearVelocity(vec);
    }
    mousey = y;
    mousex = x;

}
