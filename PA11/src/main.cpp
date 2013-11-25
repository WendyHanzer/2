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

#include <time.h>

#include "model.h" 
#include "GameStat.h"

//--Evil Global variables
//Just for this example! 
int w = 640, h = 480;// Window size
std::string rotationDirection;
//added
void drawText(int x, int y, void* font, const char* text); 
Scene modelScene1, modelScene2, modelScene3, modelScene4, modelScene5, modelScene6, modelScene7;
Scene modelSceneText[9];
int scoreP1 = 0, scoreP2 = 0;
int mousex, mousey;

glm::vec3 eyePosition = glm::vec3(0, 30.1, 0.1);
//transform matrices
glm::mat4 model1,model2,model3,model4,model5,model6,model7,extraBallMat;

bool extraBall = false;
//added
bool resetBoard = false;
int lives = 3;
std::string message = "NULL";
int maxScore = 40000;
int score = 0;
//
glm::mat4 view;//world->eye 
glm::mat4 projection;//eye->clip 
glm::mat4 mvp;//premultiplied modelviewprojection

Light light[2]; //create light to be used on all our objects
int level = 1;
clock_t totalTime;
GameStat* game;

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
void menuControls(int); 

// keyboard input
bool* keyspushed = new bool[256];
bool* specialkeypushed = new bool[256];
void keyOperations();
void specialKeyOperations();
bool useMouse = true;

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Check if we have a score
int checkBall (btVector3 vec);
void moveView(float dir, bool xy, bool z);
bool swapLevels(int l);
void resetBall();

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
  btRigidBody* extraBallRB;
 

//--Main
int main(int argc, char **argv)
{ 
	game = new GameStat(argv[1]);
      //setup our light with some default values
     game->read();
    //have w = 0 for position makes a directional source, 1 makes point source
    light[0].light_position = glm::vec4( 1.0, 5.0, 2.0, 0.0 );
    light[0].light_ambient = glm::vec4( 2.0, 2.0, 2.0, 1.0 );
    light[0].light_diffuse = glm::vec4( 0.9, 0.9, 0.9, 1.0 );
    light[0].light_specular = glm::vec4( 3.0, 3.0, 3.0, 1.0 );
    light[1].light_position = glm::vec4( 0.0, 5.0, 0.0, 0.0 );
    light[1].light_ambient = glm::vec4( 2.0, 2.0, 2.0, 1.0 );
    light[1].light_diffuse = glm::vec4( 0.9, 0.9, 0.9, 1.0 );
    light[1].light_specular = glm::vec4( 3.0, 3.0, 3.0, 1.0 );

    //give the rest of the light properties some default values
    light[0].constantAttenuation = 0.0;
    light[0].linearAttenuation = 1.0;
    light[0].quadraticAttenuation = 0.0;
    light[0].spotCutoff = 45;
    light[0].spotExponent = 0;
    light[0].spotDirection = glm::vec3(0.0, -1.0, 0.0);
    light[1].constantAttenuation = 0.0;
    light[1].linearAttenuation = 1.0;
    light[1].quadraticAttenuation = 0.0;
    light[1].spotCutoff = 45;
    light[1].spotExponent = 0;
    light[1].spotDirection = glm::vec3(0.0, -1.0, 0.0);
 
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Labyrinth");

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
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keydown);// Called if there is keyboard input
    glutKeyboardUpFunc(keyup);
    glutMouseFunc(mouse);//called when the mouse is depushed or released
    glutPassiveMotionFunc(passive);

    glutSpecialFunc(specialInput); //called for special input, ie arrow keys for this project
    glutSpecialUpFunc(specialUpInput);

    //Create our simple menu
    glutCreateMenu(menuControls);
    glutAddMenuEntry("Quit", 1);
    glutAddMenuEntry("Resume Game", 2);
    glutAddMenuEntry("Pause Game", 3);
    //glutAddMenuEntry("restart", 4);
    glutAddMenuEntry("Add Ball", 5);
    glutAddMenuEntry("Swap Levels", 6);
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

    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.6, 0.6, 0.9, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//added
	
	drawText(20, (h - 20), GLUT_BITMAP_HELVETICA_18, message.c_str());
	const char* top10 = "Top 10";
	drawText(20, (h - 40), GLUT_BITMAP_HELVETICA_10, top10);
	std::string* temp = game->draw();
	for (int i = 0; i < 10; i++){
		drawText(20, ((h - 30) - 20 * (i+1)) , GLUT_BITMAP_HELVETICA_10, temp[i].c_str());
	}
    
    mvp = projection * view * model1;
	modelScene1.draw(mvp, light);

    mvp = projection * view * model2;
    modelScene2.draw(mvp, light);

    mvp = projection * view * model3;
    modelScene3.draw(mvp, light);

    mvp = projection * view * model4;
    modelScene4.draw(mvp, light);

    mvp = projection * view * model5;
    modelScene5.draw(mvp, light);

    mvp = projection * view * model6;
    modelScene6.draw(mvp, light);

    mvp = projection * view * model7;
    modelScene7.draw(mvp, light);


    if (extraBall){
      mvp = projection * view * extraBallMat;
      modelScene1.draw(mvp, light);
    }
     
    //swap the buffers
    glutSwapBuffers();
}
 
void update()
{ 
  //handle keyboard input
  keyOperations();
  specialKeyOperations();

  static float dt = 0;
  dt += getDT();

  //to prevent from running at machine speed, check dt and don't update models unless .01 seconds have passed
  if (dt < 0.01)
    return;
  
  dt = 0; //reset dt
  // Update the state of the scene, step by 1/30 to get gameplay faster
  dynamicsWorld->stepSimulation(1.f/30.f,10);

    message = "Lives: ";
    message += std::to_string(lives);
    message += " \r\nTime: ";
    clock_t newTime = clock();
    float time = float(newTime-totalTime) / CLOCKS_PER_SEC;
    message += std::to_string(time);
    message += " \r\nScore: ";
    score = glm::max(0, int((maxScore - time * 100) * (lives + 1)));
    message += std::to_string(score);
   
	// print
	for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
	{
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        if(body && body->getMotionState())
        {
            //ball
            if( j == 0 ) {
              btTransform trans;
              btVector3 vec;
              vec = body -> getCenterOfMassPosition();
              trans = body->getCenterOfMassTransform();
              btScalar m[16];
              trans.getOpenGLMatrix(m);
              model1 = glm::make_mat4(m);
              //std::cout << "Updating ball: " << float(vec.getX()) << " " << float(vec.getY()) << " " << float(vec.getZ()) << std::endl;
              checkBall(vec);
            }
            //big labyrinth board
            else if (j == 1){
              btTransform trans;
              trans = body->getCenterOfMassTransform();
              btScalar m[16];
              trans.getOpenGLMatrix(m);
              model3 = glm::make_mat4(m);
            }
            //small labyrinth board
            else if (j == 2){
              btTransform trans;
              trans = body->getCenterOfMassTransform();
              btScalar m[16];
              trans.getOpenGLMatrix(m);
              model2 = glm::make_mat4(m);
            }
            //our added extra ball (if its in play)
            else if (j == 3){
              btTransform trans;
              btVector3 vec;
              vec = body -> getCenterOfMassPosition();
              trans = body->getCenterOfMassTransform();
              btScalar m[16];
              trans.getOpenGLMatrix(m);
              extraBallMat = glm::make_mat4(m);
              checkBall(vec);
            }
        }
    }

    glutPostRedisplay();//call the display callback
    glutPostOverlayRedisplay();

}  

int checkBall (btVector3 vec){
 //check if the ball has fallen through a hole
  if (vec.getY() < -10){
    std::cout << "Lost a life!\n\n";
    resetBall();
    //added
    //glutIdleFunc(NULL);
    resetBoard = true;
    
    if (lives == 0){
      std::cout << "All lives lost.  Resetting lives to 3\n";
      lives = 3;
      message = "Sorry, you lost all lives.  Press F to restart";
	 game->save();
      glutIdleFunc(NULL);
    }
    else{
      lives--;
      std::cout << "Lives remaining: " << lives << std::endl;
    }
    return -1;
  }
  else if ((vec.getZ() < -8 && level == 1) || (vec.getX() > 5 && level == 2)){
    //added
    float t;
    std::cout << "Winner!\n";
    message = "Winner! Total time: ";
    resetBall();
    clock_t newTime = clock();
    t = float(newTime - totalTime) / CLOCKS_PER_SEC;
    std::cout << "\r\nTotal time: " << t << std::endl << std::endl;
    message += std::to_string(t);
    message += " \r\nScore: ";
    message += std::to_string(score);
    game->scored(maxScore, t*100, score);
    //reset the clock
    totalTime = clock();
    resetBoard = true;
    swapLevels(1);
    game->save();
    glutIdleFunc(NULL);
    return 1;
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


  //model3 = glm::translate( glm::mat4(1.0f), glm::vec3(-2.5, 0, 6.6));

    //load the model, setup shaders
    modelScene1.initMeshes("assets/models/ball.obj");
    modelScene2.initMeshes("assets/models/smalllabyrinth.obj");
    modelScene3.initMeshes("assets/models/btlabyrinth.obj");
    modelScene4.initMeshes("assets/models/largeboundingboxwithnobs.obj");
    modelScene5.initMeshes("assets/models/largexbar.obj");
    modelScene6.initMeshes("assets/models/largezbar.obj");
    modelScene7.initMeshes("assets/models/smallboundingboxwithnobs.obj");

        model5 = glm::translate(glm::mat4(1.0f), glm::vec3(0,-2,0));
        model6 = glm::translate(glm::mat4(1.0f), glm::vec3(0,-2,0));
	
    // set gravity
    dynamicsWorld->setGravity(btVector3(0,-10,0));

    // create rigid bodies
    {
        // the ball
        btCollisionShape* colShape = new btSphereShape(btScalar(0.4));
        collisionShapes.push_back(colShape);

        // make it dynamic
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(100.f);

        bool isDynamic = (mass!=0.f);
        
        btVector3 localInertia(0,1.,0);
        if(isDynamic)
            colShape->calculateLocalInertia(mass,localInertia);

        startTransform.setOrigin(btVector3(-2.5,5,6.6));
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);

		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRestitution(btScalar(.5));
		body->setFriction(btScalar(100));
		body->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addRigidBody(body);
    }

    {
        // the second ball (to be added to the world later)
        btCollisionShape* colShape = new btSphereShape(btScalar(0.4));
        collisionShapes.push_back(colShape);

        // make it dynamic
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(100.f);

        bool isDynamic = (mass!=0.f);
        
        btVector3 localInertia(0,1.,0);
        if(isDynamic)
            colShape->calculateLocalInertia(mass,localInertia);

        startTransform.setOrigin(btVector3(-2.0,5,6.6));
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);

		extraBallRB = new btRigidBody(rbInfo);
		extraBallRB->setRestitution(btScalar(.5));
		extraBallRB->setFriction(btScalar(100));
		extraBallRB->setActivationState(DISABLE_DEACTIVATION);
    } 
	// create rigid bodies
    {

		btTriangleIndexVertexArray *triMesh = new btTriangleIndexVertexArray();
		btIndexedMesh iMesh;
		for (unsigned int i = 0; i < modelScene3.meshes.size(); i++){   
		
			iMesh.m_numTriangles        = modelScene3.meshes[i].Indices.size() / 3;
			iMesh.m_triangleIndexBase   = (const unsigned char *) (&(modelScene3.meshes[i].Indices[0]));
			iMesh.m_triangleIndexStride = 3 * sizeof(unsigned int);
			iMesh.m_numVertices         = modelScene3.meshes[i].vertices.size();
			iMesh.m_vertexBase          = (const unsigned char * )(&(modelScene3.meshes[i].vertices[0]));
			iMesh.m_vertexStride        = sizeof(Vertex);
			iMesh.m_indexType           = PHY_INTEGER;
			iMesh.m_vertexType          = PHY_FLOAT;
			triMesh->addIndexedMesh( iMesh, PHY_INTEGER );	
		}	
  	

        //large board
        btCollisionShape* boardShape= new btBvhTriangleMeshShape(triMesh,true);
        collisionShapes.push_back(boardShape);

        // make it dynamic
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(0.0f);

        bool isDynamic = (mass!=0.f);
        
        btVector3 localInertia(0,1.,0);
        if(isDynamic)
            boardShape->calculateLocalInertia(mass,localInertia);
        btQuaternion quat;
        quat.setEuler(0, 0, 0);
        startTransform.setRotation(quat);
																					//2.5, 0.0, -6.75
        startTransform.setOrigin(btVector3(10.0, 0.0, 0.0));
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,boardShape,localInertia);

		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRestitution(btScalar(.5));
		body->setFriction(btScalar(100));
		body->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addRigidBody(body);
	}

    // create small board collision object
    {

		btTriangleIndexVertexArray *triMesh = new btTriangleIndexVertexArray();
		btIndexedMesh iMesh;
		for (unsigned int i = 0; i < modelScene2.meshes.size(); i++){   
		
			iMesh.m_numTriangles        = modelScene2.meshes[i].Indices.size() / 3;
			iMesh.m_triangleIndexBase   = (const unsigned char *) (&(modelScene2.meshes[i].Indices[0]));
			iMesh.m_triangleIndexStride = 3 * sizeof(unsigned int);
			iMesh.m_numVertices         = modelScene2.meshes[i].vertices.size();
			iMesh.m_vertexBase          = (const unsigned char * )(&(modelScene2.meshes[i].vertices[0]));
			iMesh.m_vertexStride        = sizeof(Vertex);
			iMesh.m_indexType           = PHY_INTEGER;
			iMesh.m_vertexType          = PHY_FLOAT;
			triMesh->addIndexedMesh( iMesh, PHY_INTEGER );	
		}	
	
        //small board
        btCollisionShape* boardShape = new btBvhTriangleMeshShape(triMesh,true);
        collisionShapes.push_back(boardShape);

        // make it dynamic
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(0.0f);

        bool isDynamic = (mass!=0.f);
        
        btVector3 localInertia(0,1.,0);
        if(isDynamic)
            boardShape->calculateLocalInertia(mass,localInertia);

        startTransform.setOrigin(btVector3(0.0,0.0,-20.0));
        model7 = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,-20));
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,boardShape,localInertia);

		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRestitution(btScalar(.5));
		body->setFriction(btScalar(100));
		body->setActivationState(DISABLE_DEACTIVATION);
                body->setCenterOfMassTransform(startTransform);

		dynamicsWorld->addRigidBody(body);
    }

    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    //view = glm::lookAt( eyePosition, //Eye Position
    //                    glm::vec3(0.0, 0.0, 0.0), //Focus point
    //                    glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
    moveView (30.0, true, true);
    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 


    totalTime = clock();

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glutSetCursor(GLUT_CURSOR_NONE);

	
    message = "Lives: ";
    message += std::to_string(lives);
    message += " Time: ";

    //and its done
    return true;
}

//Set up the menu options
void menuControls(int id){
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

    case 5:
      //add our ball body to the world
      dynamicsWorld->addRigidBody(extraBallRB);
      extraBall = true;
      break;
    //swap which level you're playing
    case 6:
      swapLevels(1);
      break;
  }

  glutPostRedisplay();
  glutPostOverlayRedisplay();
} 

void resetBall(){
  btCollisionObject* ball = dynamicsWorld->getCollisionObjectArray()[0];
  btCollisionObject* ball1;
  btRigidBody* ballRB = btRigidBody::upcast(ball);
  btRigidBody* ball1RB;
  btTransform transBall;
  transBall.setIdentity();

  if (extraBall){
    ball1 = dynamicsWorld->getCollisionObjectArray()[3];
    ball1RB = btRigidBody::upcast(ball1);
    ball1RB->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
  }

  if (level == 2){
    transBall.setOrigin(btVector3(2, 5, 4.6));
    ballRB->setCenterOfMassTransform(transBall);
    if (extraBall) {
      transBall.setOrigin(btVector3(2.5, 5, 4.6));
      ball1RB->setCenterOfMassTransform(transBall);
    }
  }
  else{
    transBall.setOrigin(btVector3(-2.5, 5, 6.6));
    ballRB->setCenterOfMassTransform(transBall);
    if (extraBall) {
      transBall.setOrigin(btVector3(-1.5, 5, 6.6));
      ball1RB->setCenterOfMassTransform(transBall);
    }
  }
  ballRB->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
}

bool swapLevels(int l){
  //get our objects
  btCollisionObject* bigBoard = dynamicsWorld->getCollisionObjectArray()[1];
  btCollisionObject* littleBoard = dynamicsWorld->getCollisionObjectArray()[2];
  btRigidBody* bigRB = btRigidBody::upcast(bigBoard);
  btRigidBody* littleRB = btRigidBody::upcast(littleBoard);

  btTransform transBig, transLittle;

  transLittle = bigRB->getCenterOfMassTransform();
  transBig = littleRB->getCenterOfMassTransform();

  bigRB->setCenterOfMassTransform(transBig);
  littleRB->setCenterOfMassTransform(transLittle);

  if (level == 1){
    level = 2;
	model4 = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,-20));
        model7 = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
        model5 = glm::translate(glm::mat4(1.0f), glm::vec3(0,-2,-20));
        model6 = glm::translate(glm::mat4(1.0f), glm::vec3(0,-2,-20));
}else{
    level = 1;
	level = 1;
        model4 = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
        model7 = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,-20));
        model5 = glm::translate(glm::mat4(1.0f), glm::vec3(0,-2,0));
        model6 = glm::translate(glm::mat4(1.0f), glm::vec3(0,-2,0));
	}
  resetBall();

  return true;
}

//keyboard input stuff - triggers actions for the model such as start rotation, etc.
void keydown(unsigned char key, int x_pos, int y_pos)
{
		static bool playing = false;
    keyspushed[key] = true;

  switch (key) {
  case 'e':
  case 'E':
    if (useMouse)
      useMouse = false;
    else
      useMouse = true;
    break;
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

  //add some very basic light controls
  case 'g':
   light[0].light_position.x += 1;
   break;
  case 'h':
   light[0].light_position.x -= 1;
   break;
  case 't':
   light[0].light_position.z += 1;
   break;
  case 'y':
   light[0].light_position.z -= 1;
   break;
  case 'b':
    light[0].light_position.y += 1;
    break;
  case 'n':
    light[0]. light_position.y -= 1;
    break;
case 'G':
   light[1].light_position.x += 1;
   break;
  case 'H':
   light[1].light_position.x -= 1;
   break;
  case 'T':
   light[1].light_position.z += 1;
   break;
  case 'Y':
   light[1].light_position.z -= 1;
   break;
  case 'B':
    light[1].light_position.y += 1;
    break;
  case 'N':
    light[1]. light_position.y -= 1;
    break;
  
    //turn off ambient lighting
    case '1':
        light[0].light_ambient = glm::vec4( 0.0, 0.0, 0.0, 1.0 );
        break;
    //turn on low ambient light
    case '2':
        light[0].light_ambient = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
        break;
    //turn on directional(distants) lighting
    case '3':
	   light[0].light_position.w = 0.0;
     break;
	case '#':
	   light[1].light_position.w = 0.0;
     break;

    //turn off directional(distant) lighting
     // point light 
	
    case '4':
        //turns off directional lighting first
	   light[0].light_position.w = 1.0;
	   light[0].spotCutoff = 180.0;
	   
     break;
	case '$':
		light[1].light_position.w = 1.0;
	   light[1].spotCutoff = 180.0;
	break;
    //spot light
    case '5':
        //turns off directional lighting first
	   light[0].light_position.w = 1.0;
        light[0].spotCutoff = 15.0;

     	break;
	case '%':
		light[1].light_position.w = 1.0;
        	light[1].spotCutoff = 45.0;
		break;
    case '6':
      light[0].light_specular = glm::vec4(0.0, 0.0, 0.0, 1.0);
      light[0].light_diffuse = glm::vec4(0.0, 0.0, 0.0, 1.0);
      break;
    case '7':
      light[0].light_diffuse = glm::vec4( 10, 10, 10, 1.0 );
      light[0].light_specular = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
      break;
    case '8':
		light[0].spotDirection.x+=1;
  		break;
    case '9':
          light[0].spotDirection.x-=1;
		break;
    case '0':
		light[0].spotDirection.y+=1;
		break;
    case '-':
		light[0].spotDirection.y-=1;
		break;
    case '+':
		light[0].spotDirection.z+=1;
		break;
    case '=':
		light[0].spotDirection.z-=1;
		break;
    glutPostRedisplay();
    glutPostOverlayRedisplay();
  }
}void keyup(unsigned char key, int x_pos, int y_pos)
{
    keyspushed[key] = false;
}

void specialInput (int key, int x, int y) {


    specialkeypushed[key] = true;

}
void specialUpInput(int key, int x, int y)
{
    specialkeypushed[key] = false;
}

void keyOperations()
{

	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
	btRigidBody* body = btRigidBody::upcast(obj);
  btVector3 vec = body->getLinearVelocity();

    if( vec.y() > 1 ) vec *= btVector3(1, 0, 1);

    if( keyspushed['w'] )
    {
        if( vec.x() < 20 )
        {
            vec += btVector3(1, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( keyspushed['s'] )
    {
        if( vec.x() > -20 )
        {
            vec += btVector3(-1, 0, 0);
            body->setLinearVelocity(vec);
        }
    }
    if( keyspushed['d'] )
    {
        if( vec.z() < 20 )
        {
            vec += btVector3(0, 0, 1);
            body->setLinearVelocity(vec);
        }
    }
    if( keyspushed['a'] )
    {
        if( vec.z() > -20 )
        {
            vec += btVector3(0, 0, -1);
            body->setLinearVelocity(vec);
        }
    }
/*
    if( !keyspushed['w'] && !keyspushed['s'] )
    {
            vec *= btVector3(0, 1, 1);
            body->setLinearVelocity(vec);
    }
    if( !keyspushed['a'] && !keyspushed['d'] )
    {
            vec *= btVector3(1, 1, 0);
            body->setLinearVelocity(vec);
    }
*/
    if( keyspushed['q'] || keyspushed['Q'] || keyspushed[27] )
        exit(0);

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


   btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[level];
   btRigidBody* body = btRigidBody::upcast(obj);

   btTransform trans;
   body->getMotionState()->getWorldTransform(trans);

   btCollisionObject* obj1 = dynamicsWorld->getCollisionObjectArray()[0];
   btRigidBody* body1 = btRigidBody::upcast(obj1);

   btTransform trans1;
   body1->getMotionState()->getWorldTransform(trans1);
/* 
   btTransform boardTrans;
   boardTrans.setIdentity();
   btQuaternion quat;
*/
  btQuaternion quat;
  btTransform tr;
  tr.setIdentity();


/*
	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
	btRigidBody* body = btRigidBody::upcast(obj);
  btVector3 vec = body->getLinearVelocity();
*/
    static float uAngle = 0.0;
    static float sAngle = 0.0;
    //float dt = getDT();
    
    if(specialkeypushed[GLUT_KEY_UP])
    {
       uAngle -= 0.01;
       model5 = glm::rotate( model5, uAngle*180, glm::vec3(1.0f,0.0f,0.0f) );
    }
    if(specialkeypushed[GLUT_KEY_DOWN])
    {
       uAngle += 0.01;
       model5 = glm::rotate( model5, uAngle*180, glm::vec3(-1.0f,0.0f,0.0f) );
    }
    if(specialkeypushed[GLUT_KEY_LEFT])
    {
       sAngle += 0.01;
       model6 = glm::rotate( model6, uAngle*180, glm::vec3(0.0f,0.0f,1.0f) );
    }
    if(specialkeypushed[GLUT_KEY_RIGHT])
    {
       sAngle -= 0.01;
       model6 = glm::rotate( model6, uAngle*180, glm::vec3(0.0f,0.0f,-1.0f) );
    }

  if (resetBoard){
    uAngle = 0;
    sAngle = 0;
    resetBoard = false;
  }
  quat.setEuler(0,uAngle,sAngle);

  tr.setRotation(quat);
  quat.setEuler(0, uAngle * 40, sAngle * 40);
  trans.setRotation(quat);
  trans1.setRotation(quat);

  body->getMotionState()->setWorldTransform(trans);
  body->setCenterOfMassTransform(tr);
}

void mouse(int button, int state, int x, int y) {

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
    //updateModel(2);
  }

  
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

void moveView(float dir, bool xy, bool xz){
		float dt = 0;
		dt = getDT();
		static float angleXY = 90.0;
		static float angleXZ = -90.0;
		//t t is reset, f f is zoom
		//f t is rotate, t f is switch sort of
		if (xz && xy){
		     angleXY = 90.0;
                     angleXZ = -90.0; 
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
		//default camera distance
		static float dis = 30.0;
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

		    		
		view = glm::lookAt( eyePosition, //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
    glutPostRedisplay();
    glutPostOverlayRedisplay();
    //std::cout << eyePosition.x << " " << eyePosition.y << " " << eyePosition.z << '\n';
}

void drawText(int x, int y, void* font, const char* text){
	const char* c = text;
	glLoadIdentity();
	glOrtho(0, w, 0,h, 0, 1);
	glColor3f(0.0, 0.0, 0.0);	
	glRasterPos2i(x,y);
	
	
	while (*c != '\0'){
		glutBitmapCharacter(font, *c);
		c++;
	}
	glutPostRedisplay();
	glutPostOverlayRedisplay();
	
}

void passive(int x, int y){
}
