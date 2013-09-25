#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier


//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

float dir = 1; //direction of spin
float speed = 1; //speed of spin
bool canSpin = false; //able to spin flag

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);

//PA02 keyboard and mouse callbacks
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);

//PA04
char* fname;
Vertex* loadOBJ(char* obj);
float colorConv(float i);
unsigned int numOfVert = 0;
float amount = 0;
//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//PA01 loader function
char* shaderLoader(const char*);
//PA02 menu function
void menu(int id);
//--Main
int main(int argc, char ** argv){
     // Initialize glut
     glutInit(&argc, argv);
     glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
     glutInitWindowSize(w, h);
     // Name and create the Window
     glutCreateWindow("PA04");

	//command line arg
	int size = std::strlen(argv[1]);
	fname = new char[size];
	std::strcpy(fname, argv[1]);

     // Now that the window is created the GL context is fully set up
     // Because of that we can now initialize GLEW to prepare work with shaders
     GLenum status = glewInit();
     if( status != GLEW_OK){
          std::cerr << "[F] GLEW NOT INITIALIZED: ";
          std::cerr << glewGetErrorString(status) << std::endl;
          return -1;
     }

     // Set all of the callbacks to GLUT that we need
     glutDisplayFunc(render);// Called when its time to display
     glutReshapeFunc(reshape);// Called if the window is resized
     glutIdleFunc(update);// Called if there is nothing else to do
	
     //PA02 Call back and menu functions
     glutKeyboardFunc(keyboard);// Called if there is keyboard input
     glutMouseFunc(mouse);// Called if there is mouse input
     glutCreateMenu(menu);
	glutAddMenuEntry("quit", 1);
	glutAddMenuEntry("start rotation", 2);
	glutAddMenuEntry("stop rotation", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

     // Initialize all of our resources(shaders, geometry)
     bool init = initialize();
     if(init){
          t1 = std::chrono::high_resolution_clock::now();
          glutMainLoop();
     }

     // Clean up after ourselves
     cleanUp();
     return 0;
}

//--Implementations
void render(){
     //--Render the scene

    	//clear the screen
    	glClearColor(0.0, 0.0, 0.2, 1.0);
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//premultiply the matrix for this example
	mvp = projection * view * model;

     //enable the shader program
     glUseProgram(program);

     //upload the matrix to the shader
     glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

     //set up the Vertex Buffer Object so it can be drawn
     glEnableVertexAttribArray(loc_position);
     glEnableVertexAttribArray(loc_color);
     glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
     //set pointers into the vbo for each of the attributes(position and color)
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

     glDrawArrays(GL_TRIANGLES, 0, numOfVert);//mode, starting index, count

     //clean up
     glDisableVertexAttribArray(loc_position);
     glDisableVertexAttribArray(loc_color);
                           
     //swap the buffers
     glutSwapBuffers();
}

void update(){
     //total time
    	static float angle = 0.0;
	static float spin = 0.0;
    	float dt = getDT();// if you have anything moving, use dt.

     angle += dt * M_PI/2; //move through 90 degrees a second which is in radians
   
     model = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angle), 0.0, 4.0 * cos(angle)) );
	//model = glm::scale( model, glm::vec3(50.0f, 50.0f, 50.0f));
     // model is our cube
     // rotate is a function that will do matrix math with rotation matrix
     // mat4(1.0f) is our that converts result to 4x4 matrix
     // angle*100 is our alpha angle this is in degrees 
     // 3d vector to change matrix so it rotates along y
     // spins clockwise
	if (canSpin){    
     	//converts rad to angles then multiply by direction and speed of spin
          spin += speed * dir * dt * 90;   
	}
	model = model * glm::rotate(glm::mat4(1.0f), spin, glm::vec3(0.0f,1.0f, 1.0f));  
 
     // Update the state of the scene
     glutPostRedisplay();//call the display callback
}


void reshape(int n_w, int n_h){
	w = n_w;
     h = n_h;
     //Change the viewport to be correct
     glViewport( 0, 0, w, h);
     //Update the projection matrix as well
     //See the init function for an explaination
     projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

bool initialize(){
     // Initialize basic geometry and shaders for this example
	Vertex* geometry = loadOBJ(fname);
     
	// Create a Vertex Buffer object to store this vertex info on the GPU
    	glGenBuffers(1, &vbo_geometry);
    	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    	glBufferData(GL_ARRAY_BUFFER, numOfVert * sizeof(Vertex), geometry, GL_STATIC_DRAW);

    	//--Geometry done

    	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    	//Shader Sources
    	// Put these into files and write a loader in the future
    	// Note the added uniform!
    	const char *vs;
    	vs = shaderLoader("vertexShader");
    	const char *fs;
     fs = shaderLoader("fragmentShader");


	
    	//compile the shaders
    	GLint shader_status;

    	// Vertex shader first
    	glShaderSource(vertex_shader, 1, &vs, NULL);
    	glCompileShader(vertex_shader);
    	//check the compile status
    	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    	if(!shader_status){
     	std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
    		return false;
    	}

    	// Now the Fragment shader
    	glShaderSource(fragment_shader, 1, &fs, NULL);
    	glCompileShader(fragment_shader);
    	//check the compile status
    	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    	if(!shader_status){
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

    	loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    	if(loc_mvpmat == -1){
     	std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
     	return false;
     }
    
    	//--Init the view and projection matrices
    	//  if you will be having a moving camera the view matrix will need to more dynamic
    	//  ...Like you should update it before you render more dynamic 
    	//  for this project having them static will be fine
    	view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
    	                    glm::vec3(0.0, 0.0, 0.0), //Focus point
                         glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
	
  	projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    	//enable depth testing
    	glEnable(GL_DEPTH_TEST);
    	glDepthFunc(GL_LESS);

    	//and its done
    	return true;
}

void cleanUp(){
	// Clean up, Clean up
	glDeleteProgram(program);
     glDeleteBuffers(1, &vbo_geometry);
}

//returns the time delta
float getDT(){
	float ret;
     t2 = std::chrono::high_resolution_clock::now();
     ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
     t1 = std::chrono::high_resolution_clock::now();
     return ret;
}

// shader loader function
char* shaderLoader(const char* shaderFile){
	//opens files
	std::ifstream file;
	file.open(shaderFile, std::ios::in);
	//our temporary buffer to hold file content
	char* buffer;
	if (!file){
		//minor error check
		std::cerr << "File Fail To Open" << std::endl;
	} else {
		/* use seek to search for end of file 
	           This method is used instead of string length because I wanted to avoid
		   reopening the file and inserting white space.
		 */
		file.seekg(0, std::ios::end);
		long len = file.tellg();
		//moved seek cursor back to beginning of file
		file.seekg(std::ios::beg);
		if (len > 0){
			//read file and store in c-string
			int i = 0; //index
			buffer = new char[len+1];
			while (file.good()){
				buffer[i] = file.get();
				if (!file.eof()){
					i++;
				}
			}
			buffer[i] = '\0';
		} else {
			std::cerr << "Empty File" << std::endl;
		}
	}
	file.close();
	return buffer;
}

// menu function for callback
void menu(int id){
	switch(id){
		case 1:
			exit(0);
			break;
		case 2:
			canSpin = true;
			break;
		case 3:
			canSpin = false;
			break;
	}
	glutPostRedisplay();
}
// keyboard function for callback 
void keyboard(unsigned char key, int x, int y){
	switch (key){
		case 'q':
			exit(0);	
			break;
		case 'Q':
			exit(0);
			break;	
		case 27: //excape key
			exit(0);
			break;
		case 'r':
			dir = dir * -1.0f;
			break;
		//spin speed control
		case '1':
			speed = 1.0;
			break;
		case '2':
			speed = 2.0;
			break;
		case '3':
			speed = 3.0;
			break;
		case '4':
			speed = 4.0;
			break;
		case '5':
			speed = 5.0;
			break;
		case '6':
			speed = 6.0;
			break;
	}
}

//mouse function for call back
void mouse(int button, int state, int x, int y){
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		dir = dir * -1.0f;
	}
}
Vertex* loadOBJ(char* obj){
	//number of Reference Vertices	
	int numRefVert = 0;
	//size of temporary vertice array
	int sizeOfTemp = 2;
	
	bool hasVT = false;
	bool hasNorm = false;
	bool hasFourth = false;	
	int match = 0;
		

	//Temporary Reference Vertices array, will be resized dynamically and then exactly sized
	Vertex* tempV = new Vertex[sizeOfTemp];	
	std::vector< unsigned int > vertexIndices;
	FILE * file = fopen(obj, "r");
	if( file == NULL ){
	    printf("Impossible to open the file !\n");
	    return NULL;
	}

	while( 1 ){
 
	    char lineHeader[128];
	    // read the first word of the line
	    int res = fscanf(file, "%s", lineHeader);
	    if (res == EOF)
	        break; // EOF = End Of File. Quit the loop.
 

		if ( strcmp( lineHeader, "v" ) == 0 ){
	    		GLfloat x, y, z;
	    		fscanf(file, "%f %f %f\n", &x, &y, &z );
			if (numRefVert >= sizeOfTemp){
				//resize the vertex array
				Vertex* temp = new Vertex[2 * sizeOfTemp];
				for (int i = 0; i < sizeOfTemp; i++){
					temp[i].position[0] = tempV[i].position[0];
					temp[i].position[1] = tempV[i].position[1];
					temp[i].position[2] = tempV[i].position[2];
					temp[i].color[0] = tempV[i].color[0];
					temp[i].color[1] = tempV[i].color[1];
					temp[i].color[2] = tempV[i].color[2];
				}
				sizeOfTemp = 2 * sizeOfTemp;
				delete[] tempV;
				tempV = temp;	
			}
	    		tempV[numRefVert].position[0] = x;
			tempV[numRefVert].position[1] = y;
			tempV[numRefVert].position[2] = z;
			//Hard coding color based off position
			tempV[numRefVert].color[0] = colorConv(x);
			tempV[numRefVert].color[1] = colorConv(y);
			tempV[numRefVert].color[2] = colorConv(z);
			numRefVert++;
		
		//texture and normals do nothing except determine how we will parse 
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
	    		glm::vec2 uv;
	    		fscanf(file, "%f %f\n", &uv.x, &uv.y );
			hasVT = true;
	    		

		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
    			glm::vec3 normal;
    			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
    			hasNorm = true;


		}else if ( strcmp( lineHeader, "f" ) == 0 ){
		    unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
			if (hasVT && hasNorm){
				match = fscanf(file, "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3] );
				if (match == 12){
					hasFourth = true;
				}
		
			} else if (!hasVT && hasNorm) {
				match = fscanf(file, "%i//%i %i//%i %i//%i %i//%i\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2], &vertexIndex[3], &normalIndex[3]);
				if (match == 8){
					hasFourth = true;
				}
				
			} else if (hasVT && !hasNorm){
				match = fscanf(file, "%i/%i %i/%i %i/%i %i/%i\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2], &vertexIndex[3], &uvIndex[3]);
				if (match == 8){
					hasFourth = true;
				}
			} else if (!hasVT && !hasNorm){	
				match = fscanf(file, "%i %i %i %i\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2], &vertexIndex[3]);
				if (match == 4){
					hasFourth = true;
				}
			}

			//saving face value for reference processing    
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			//quad conversion
			if (hasFourth){
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[2]);
				vertexIndices.push_back(vertexIndex[3]);
			}   
		}

	}
	//exact sizing
	Vertex* temp = new Vertex[numRefVert];
	for (int i = 0; i < numRefVert; i++){
		temp[i].position[0] = tempV[i].position[0];
		temp[i].position[1] = tempV[i].position[1];
		temp[i].position[2] = tempV[i].position[2];
		temp[i].color[0] = tempV[i].color[0];
		temp[i].color[1] = tempV[i].color[1];
		temp[i].color[2] = tempV[i].color[2];
		}
	delete[] tempV;
	tempV = temp;	
	
	
	numOfVert = vertexIndices.size();
	
	//processing data
	// For each vertex of each triangle
	//our final opengl array
	Vertex* out = new Vertex[numOfVert];
	
	for( unsigned int i=0; i< numOfVert; i++ ){
		unsigned int vertexIndex = vertexIndices[i];
		out[i].position[0] = tempV[vertexIndex-1].position[0];
		out[i].position[1] = tempV[vertexIndex-1].position[1];
		out[i].position[2] = tempV[vertexIndex-1].position[2];
		out[i].color[0] = tempV[vertexIndex-1].color[0];
		out[i].color[1] = tempV[vertexIndex-1].color[1];
		out[i].color[2] = tempV[vertexIndex-1].color[2];
	}

	return out;	
}

//determines color based on whether a position is less than 0
GLfloat colorConv(GLfloat i){
	if (i <= 0.0){
		return 0.0;
	} else {
		return 1.0;
	}
	
}

