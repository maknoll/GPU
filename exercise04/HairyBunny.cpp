
// *** Hairy Bunny mit Geometry Shader ***

#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// Window size
int width = 512;       
int height = 512;

// camera movement
float center[3];

#define PI 3.141592f

#define ROTATE 1
#define MOVE 2

float thetaStart = PI / 2.0f - 0.5f;
float phiStart = PI / 2.0f;
float rStart = 3.0f;

float theta = thetaStart;
float phi = phiStart;
float r = rStart;

float oldX, oldY;
int motionState;

float viewPosition[3];
float viewDirection[3];

GLuint vaoBunny;
GLuint iboBunny;
GLuint progSimple;
GLuint progHair;
GLuint uboCamera;

extern float bunnyData[];
extern unsigned int bunnyStride;
extern unsigned int bunnySize;
extern unsigned int bunnyIndices[];
extern unsigned int bunnyIndicesStride;
extern unsigned int bunnyIndicesSize;

// Print information about the compiling step
void printShaderInfoLog(GLuint shader)
{
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);		
	infoLog = (char *)malloc(infologLength);
	glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
	printf("%s\n",infoLog);
	free(infoLog);
}

// Print information about the linking step
void printProgramInfoLog(GLuint program)
{
	GLint infoLogLength = 0;
	GLsizei charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH,&infoLogLength);
	infoLog = (char *)malloc(infoLogLength);
	glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
	printf("%s\n",infoLog);
	free(infoLog);
}

// Reads a file and returns the content as a string
string readFile(string fileName)
{
	string fileContent;
	string line;

	ifstream file(fileName.c_str());
	if (file.is_open()) {
		while (!file.eof()){
			getline (file,line);
			line += "\n";
			fileContent += line;					
		}
		file.close();
	}
	else
		cout << "ERROR: Unable to open file " << fileName << endl;

	return fileContent;
}

void calcViewerCamera(float theta, float phi, float r)
{
    float x = r * sin(theta) * cos(phi);
    float y = r * cos(theta);
    float z = r * sin(theta) * sin(phi);
 
	viewPosition[0] = center[0] + x;
	viewPosition[1] = center[1] + y;
	viewPosition[2] = center[2] + z;
	viewDirection[0] = -x;
	viewDirection[1] = -y;
	viewDirection[2] = -z;

	glLoadIdentity();
	gluLookAt(viewPosition[0], viewPosition[1], viewPosition[2],
				viewPosition[0] + viewDirection[0], viewPosition[1] + viewDirection[1], viewPosition[2] + viewDirection[2], 
				0, 1, 0);

	// TODO: Updaten der View-Matrix. Die View-Matrix beginnt ab dem 17ten float des UBOs.
}

void mouseMotion(int x, int y)
{
	float deltaX = x - oldX;
	float deltaY = y - oldY;
	
	if (motionState == ROTATE) {
		theta -= 0.001f * deltaY;

		if (theta < 0.001f) theta = 0.001f;
		else if (theta > PI - 0.001f) theta = PI - 0.001f;

		phi += 0.001f * deltaX;	
		if (phi < 0) phi += 2*PI;
		else if (phi > 2*PI) phi -= 2*PI;
		calcViewerCamera(theta, phi, r);
	}
	else if (motionState == MOVE) {
		r += 0.03f * deltaY;
		if (r < 0.1f) r = 0.1f;
		calcViewerCamera(theta, phi, r);
	}

	oldX = (float)x;
	oldY = (float)y;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	oldX = (float)x;
	oldY = (float)y;

	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			motionState = ROTATE;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			motionState = MOVE;
		}
	}
}


//------------------------------------------------------------------------
// Rendering loop.
//------------------------------------------------------------------------
void display(void)
{
	// clear frame.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Den Block-Index des Uniform-Blocks suchen, das im Shader 'progSimple' den Namen "GlobalMatrices" trägt.
	
	// TODO: Binden Sie diesen Blockindex an den Binding Point 0.
	
	// TODO: Binden Sie das gesamte UBO an den Binding Point 0. Offset = 0 und Size = Größe der Daten im UBO.
	

	// Bind VAO and IBO
	glBindVertexArrayAPPLE(vaoBunny);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboBunny);

	// Draw solid bunny
	glUseProgram(progSimple);		
	glDrawElements(GL_TRIANGLES, bunnyIndicesSize/bunnyIndicesStride, GL_UNSIGNED_INT, 0);
	glUseProgram(0);

	// Draw hair
	glUseProgram(progHair);
	glDrawElements(GL_TRIANGLES, bunnyIndicesSize/bunnyIndicesStride, GL_UNSIGNED_INT, 0);
	glUseProgram(0);
	
	// Unbind VAO and IBO
	glBindVertexArrayAPPLE(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Flush command buffer and swap buffers.
    glFlush();
    glutSwapBuffers();
}


void initBunny()
{
	// Create vertex buffer object
	GLuint vbo;	
	glGenBuffers(1, &vbo);	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, bunnySize, bunnyData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create index buffer object
	glGenBuffers(1, &iboBunny);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboBunny);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bunnyIndicesSize, bunnyIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create vertex array object
	glGenVertexArraysAPPLE(1, &vaoBunny);
	glBindVertexArrayAPPLE(vaoBunny);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, bunnyStride, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, bunnyStride, (char*)12);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, bunnyStride, (char*)24);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	glBindVertexArrayAPPLE(0);
}

void initGL()
{
	// Initialize light source
	GLfloat light_pos[] = {10, 10, 10, 1};
	GLfloat light_col[] = { 1,  1,  1, 1};

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_col);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_col);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Initialize material
	GLfloat Bunny_diffuse[]  = {0.75f, 0.375f, 0.075f, 1};
	GLfloat Bunny_specular[] = {0.8f, 0.8f, 0.8f, 1};

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Bunny_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Bunny_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 45.2776f);

	// TODO: Uniform Buffer Object für die Camera Matrizen anlegen.
	
	// TODO: Das UBO binden (target = GL_UNIFORM_BUFFER)
		
	// TODO: Speicherplatz allokieren mit glBufferData. Reservieren Sie Platz für 2 4x4 Matrizen mit float-Einträgen. Data = NULL und Usage = GL_STREAM_DRAW

	// Initialize camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 0.1, 100);
	
	// TODO: query projection matrix and update the vbo.
		// Getten Sie sich die Projektionsmatrix und kopieren Sie sie auf die ersten 16 float Werte des UBOs. Beachten Sie, das das UBO dazu gebunden sein muss!
		// Verwenden Sie dazu die Befehle glGetFloatv und glBufferSubData
	
	// Viewmatrix initialisieren
	glMatrixMode(GL_MODELVIEW);
	calcViewerCamera(theta, phi, r);

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);
}

void initGLSL()
{
	// Create empty shader object (vertex shader)
	GLuint vertexShaderHair = glCreateShader(GL_VERTEX_SHADER);

	// Read vertex shader source 
	string shaderSource = readFile("hair.vert");
	const char* sourcePtr = shaderSource.c_str();

	// Attach shader code
	glShaderSource(vertexShaderHair, 1, &sourcePtr, NULL);	

	// Compile
	glCompileShader(vertexShaderHair);
	printShaderInfoLog(vertexShaderHair);


	// Create empty shader object (fragment shader)
	GLuint fragmentShaderHair = glCreateShader(GL_FRAGMENT_SHADER);

	// Read vertex shader source 
	shaderSource = readFile("hair.frag");
	sourcePtr = shaderSource.c_str();

	// Attach shader code
	glShaderSource(fragmentShaderHair, 1, &sourcePtr, NULL);	

	// Compile
	glCompileShader(fragmentShaderHair);
	printShaderInfoLog(fragmentShaderHair);


	// Create empty shader object (geometry shader)
	GLuint geometryShaderHair = glCreateShader(GL_GEOMETRY_SHADER);

	// Read vertex shader source 
	shaderSource = readFile("hair.geom");
	sourcePtr = shaderSource.c_str();

	// Attach shader code
	glShaderSource(geometryShaderHair, 1, &sourcePtr, NULL);	

	// Compile
	glCompileShader(geometryShaderHair);
	printShaderInfoLog(geometryShaderHair);

	// Create shader program
	progHair = glCreateProgram();	

	// Attach shader
	glAttachShader(progHair, vertexShaderHair);
	glAttachShader(progHair, fragmentShaderHair);
	glAttachShader(progHair, geometryShaderHair);

    // Geometryshader init
    glProgramParameteriEXT(progHair, GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    glProgramParameteriEXT(progHair, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    glProgramParameteriEXT(progHair, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);


	// Link program
	glLinkProgram(progHair);
	printProgramInfoLog(progHair);

	// Create empty shader object (vertex shader)
	GLuint vertexShaderSimple = glCreateShader(GL_VERTEX_SHADER);

	// Read vertex shader source 
	shaderSource = readFile("simple.vert");
	sourcePtr = shaderSource.c_str();

	// Attach shader code
	glShaderSource(vertexShaderSimple, 1, &sourcePtr, NULL);	

	// Compile
	glCompileShader(vertexShaderSimple);
	printShaderInfoLog(vertexShaderSimple);


	// Create empty shader object (fragment shader)
	GLuint fragmentShaderSimple = glCreateShader(GL_FRAGMENT_SHADER);

	// Read vertex shader source 
	shaderSource = readFile("simple.frag");
	sourcePtr = shaderSource.c_str();

	// Attach shader code
	glShaderSource(fragmentShaderSimple, 1, &sourcePtr, NULL);	

	// Compile
	glCompileShader(fragmentShaderSimple);
	printShaderInfoLog(fragmentShaderSimple);

	// Create shader program
	progSimple = glCreateProgram();	

	// Attach shader
	glAttachShader(progSimple, vertexShaderSimple);
	glAttachShader(progSimple, fragmentShaderSimple);
	
	// Link program
	glLinkProgram(progSimple);
	printProgramInfoLog(progSimple);
}

//------------------------------------------------------------------------
//   It's the main application function. Note the clean code you can
//   obtain using he GLUT library. No calls to dark windows API
//   functions with many obscure parameters list. =)
//------------------------------------------------------------------------
int main(int argc, char** argv)
{
     // Initialize GLUT
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
   glutInitWindowSize(width, height);
   glutCreateWindow("Hairy Bunny");

   // Init glew so that the GLSL functionality will be available
   if(glewInit() != GLEW_OK)
	   cout << "GLEW init failed!" << endl;

	// OpenGL/GLSL initializations
	initGL();
	initBunny();
	initGLSL();

	// Register callback functions   
	// glutKeyboardFunc(keyboard);
	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouse);
	glutDisplayFunc(display);
	
	// Enter main loop
	glutMainLoop();

	return 0;
}
