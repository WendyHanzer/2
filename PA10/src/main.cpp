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

Scene modelScene1;

Scene modelSceneText[9];
int scoreP1 = 0, scoreP2 = 0;
int mousex, mousey;

glm::vec3 eyePosition = glm::vec3(0, 30.1, -0.1);
//transform matrices
glm::mat4 model1;

glm::mat4 view;//world->eye 
glm::mat4 projection;//eye->clip 
glm::mat4 mvp;//premultiplied modelviewprojection

Light light; //create light to be used on all our objects
 
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
bool useMouse = true;

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Check if we have a score
bool checkGoal (btTransform trans);
void moveView(float dir, bool xy, bool z);

//--Main
int main(int argc, char **argv)
{ 

    //setup our light with some default values

    //have w = 0 for position makes a directional source, 1 makes point source
    light.light_position = glm::vec4( 1.0, 5.0, 2.0, 0.0 );
    light.light_ambient = glm::vec4( 2.0, 2.0, 2.0, 1.0 );
    light.light_diffuse = glm::vec4( 0.9, 0.9, 0.9, 1.0 );
    light.light_specular = glm::vec4( 3.0, 3.0, 3.0, 1.0 );

    //give the rest of the light properties some default values
    light.constantAttenuation = 0.0;
    light.linearAttenuation = 1.0;
    light.quadraticAttenuation = 0.0;
    light.spotCutoff = 45;
    light.spotExponent = 0;
    light.spotDirection = glm::vec3(0.0, -1.0, 0.0);

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
    glutIdleFunc(update);// Called if there is nothing else to do
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
		mvp = projection * view * model1;
		modelScene1.draw(mvp, light);
                           
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
    // Update the state of the scene 
    glutPostRedisplay();//call the display callback
}  

bool checkGoal (btTransform trans){
	float goal = float(trans.getOrigin().getX());

	if (goal > 15.5 && scoreP2 < 8){
		std::cout << "Goal for player on right!\n";
        scoreP2++;
		return true;
    }

	else if(goal < -15.5 && scoreP1 < 8){
		std::cout << "Goal for player on left!\n";
        scoreP1++;
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

    //load the model, setup shaders
    modelScene1.initMeshes("assets/models/hockeytable.obj"); 

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
   light.light_position.x += 1;
   break;
  case 'h':
   light.light_position.x -= 1;
   break;
  case 't':
   light.light_position.z += 1;
   break;
  case 'y':
   light.light_position.z -= 1;
   break;
  case 'b':
    light.light_position.y += 1;
    break;
  case 'n':
    light. light_position.y -= 1;
    break;
    //turn off ambient lighting
    case '1':
        light.light_ambient = glm::vec4( 0.0, 0.0, 0.0, 1.0 );
        break;
    //turn on low ambient light
    case '2':
        light.light_ambient = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
        break;
    //turn on directional(distants) lighting
    case '3':
	   light.light_position.w = 0.0;
     break;
    //turn off directional(distant) lighting
     // point light 
	
    case '4':
        //turns off directional lighting first
	   light.light_position.w = 1.0;
	   light.spotCutoff = 180.0;
     break;
    //spot light
    case '5':
        //turns off directional lighting first
	   light.light_position.w = 1.0;
     light.spotCutoff = 45.0;
     break;
    case '6':
      light.light_specular = glm::vec4(0.0, 0.0, 0.0, 1.0);
      light.light_diffuse = glm::vec4(0.0, 0.0, 0.0, 1.0);
      break;
    case '7':
      light.light_diffuse = glm::vec4( 0.8, 0.8, 0.8, 1.0 );
      light.light_specular = glm::vec4( 1.0, 1.0, 1.0, 1.0 );
      break;
    case '8':
		light.spotDirection.x+=1;
  		break;
    case '9':
          light.spotDirection.x-=1;
		break;
    case '0':
		light.spotDirection.y+=1;
		break;
    case '-':
		light.spotDirection.y-=1;
		break;
    case '+':
		light.spotDirection.z+=1;
		break;
    case '=':
		light.spotDirection.z-=1;
		break;
  }
}
void keyup(unsigned char key, int x_pos, int y_pos)
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
		//t t is reset, f f is zoom
		//f t is rotate, t f is switch sort of
		if (xz && xy){
		        angleXY = 90.0;
			   angleXZ = 0.0; 
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
    
    //std::cout << eyePosition.x << " " << eyePosition.y << " " << eyePosition.z << '\n';
}

void passive(int x, int y)
{
	
}
