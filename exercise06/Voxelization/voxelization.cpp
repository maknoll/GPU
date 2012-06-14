// ******* GPU Voxelization ********

#include <GL/glew.h>
#include <GLUT/glut.h>

#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;


// Bildaufloesung
#define PIC_WIDTH 512
#define PIC_HEIGHT 512

#define VOXEL_WIDTH 128
#define VOXEL_HEIGHT 128

#define PI 3.141592f

#define ROTATE 1
#define MOVE 2

float center[3] = {0.0f, 0.0f, 0.0f}; 
GLfloat viewPosition[4] = {0.0, 0.0, 3.0, 1.0};  
GLfloat viewDirection[4] = {-0.0, -0.0, -1.0, 0.0};  
GLfloat viewAngle = 45.0f;
GLfloat viewNear = 0.01f;
GLfloat viewFar = 1000.0f;

float thetaStart = PI / 2.0f;
float phiStart = PI / 2.0f;
float rStart = 1.5f;

float theta = thetaStart;
float phi = phiStart;
float r = rStart;

float oldX, oldY;
int motionState;

float angle = 0.0f;

// Texture Ids
GLuint voxelizationTextureId = 0;

// Framebuffer Object Ids
GLuint voxelizationFB = 0;

// GLSL Variables (Shader Ids, Locations, ...)
GLuint vertexShaderVoxelization;	
GLuint fragmentShaderVoxelization;
GLuint shaderProgramVoxelization;

// pixel data for visualization of the voxel model
GLuint pixels[PIC_HEIGHT*PIC_WIDTH*4];


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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


// Hilfsfunktion um Vertex & Fragment Shader einzuladen 
void loadShaderProgram(GLuint &shaderProgram, GLuint &vertexShader, char* vertexShaderName, GLuint &fragmentShader, char* fragmentShaderName)
{
	// Create empty shader object (vertex shader)
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
  
	// Read vertex shader source 
	string shaderSource = readFile(vertexShaderName);
	const char* sourcePtr = shaderSource.c_str();
  
	// Attach shader code
	glShaderSource(vertexShader, 1, &sourcePtr, NULL);	
  
	// Compile
	glCompileShader(vertexShader);
	printShaderInfoLog(vertexShader);
  
	// Create empty shader object (fragment shader)
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  
	// Read vertex shader source 
	shaderSource = readFile(fragmentShaderName);
	sourcePtr = shaderSource.c_str();
  
	// Attach shader code
	glShaderSource(fragmentShader, 1, &sourcePtr, NULL);	
  
	// Compile
	glCompileShader(fragmentShader);
	printShaderInfoLog(fragmentShader);
  
	// Create shader program
	shaderProgram = glCreateProgram();	
  
	// Attach shader
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
  
	// Link program
	glLinkProgram(shaderProgram);
	printProgramInfoLog(shaderProgram);
}


// alle Texturen und FBOs anlegen
int initFBOTextures()
{
	// Textur anlegen
	glGenTextures (1, &voxelizationTextureId);
	glBindTexture (GL_TEXTURE_2D, voxelizationTextureId);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32UI, VOXEL_WIDTH, VOXEL_HEIGHT, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
	// FBO anlegen und Texturen zuweisen
	glGenFramebuffers (1, &voxelizationFB);
	glBindFramebuffer (GL_FRAMEBUFFER, voxelizationFB);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, voxelizationTextureId, 0);
  
	// FBO Textur als aktive Textur binden.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, voxelizationTextureId);
  
	// check framebuffer status
	GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	switch (status)
	{
    case GL_FRAMEBUFFER_COMPLETE:
      cout << "FBO complete" << endl;
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      cout << "FBO configuration unsupported" << endl;
      return 1;
    default:
      cout << "FBO programmer error" << endl;
      return 1;
	}
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	return 0;
}


// calc the view position and direction from theta/phi coordinates
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
}

// Alle Shader einladen & uniform Variablen setzen
void initGLSL()
{
	loadShaderProgram(shaderProgramVoxelization, vertexShaderVoxelization, "voxelization.vert", fragmentShaderVoxelization, "voxelization.frag");
	glUseProgram(shaderProgramVoxelization);
}


void drawVoxel(float x, float y, float z)
{
	float sizeX = 1.0f/(float)(VOXEL_WIDTH);
	float sizeY = 1.0f/(float)(VOXEL_HEIGHT);
	float sizeZ = 1.0f/128.0f;
  
	// TODO: Rendern Sie einen Cube mit dem Zentrum an der Position (x,y,z) und der Ausdehnung sizeX x sizeY x sizeZ.
  glPushMatrix();
  glTranslatef(x, y, z);
  
  float a = sizeX / 2;
  float b = sizeY / 2;
  float c = sizeZ / 2;
  
  GLfloat triangles[] = {
    a, -b, -c,
    a, b, -c,
    a, -b, c,
    a, -b, c,
    a, b, -c,
    a, b, c,
    
    a, b, -c,
    -a, b, -b,
    a, b, c,
    a, b, c,
    -a, b, -c,
    -a, b, c,
    
    -a, b, -c,
    -a, -b, -c,
    -a, b, c,
    -a, b, c,
    -a, -b, -c,
    -a, -b, c,
    
    -a, -b, -c,
    a, -b, -c,
    -a, -b, c,
    -a, -b, b,
    a, -b, -c,
    a, -b, c,
    
    a, b, c,
    -a, b, c,
    a, -b, c,
    a, -b, c,
    -a, b, c,
    -a, -b, c,
    
    a, -b, -c,
    -a, -b, -c,
    a, b, -c,
    a, b, -c,
    -a, -b, -c,
    -a, b, -c
  };
  
  GLubyte colors[] = {
    0, 0, 255, 255,
    0, 0, 255, 255,
    0, 0, 255, 255,
    0, 0, 255, 255,
    0, 0, 255, 255,
    0, 0, 255, 255,
    
    0, 255, 0, 255,
    0, 255, 0, 255,
    0, 255, 0, 255,
    0, 255, 0, 255,
    0, 255, 0, 255,
    0, 255, 0, 255,
    
    255, 0, 0, 255,
    255, 0, 0, 255,
    255, 0, 0, 255,
    255, 0, 0, 255,
    255, 0, 0, 255,
    255, 0, 0, 255,
    
    0, 255, 255, 255,
    0, 255, 255, 255,
    0, 255, 255, 255,
    0, 255, 255, 255,
    0, 255, 255, 255,
    0, 255, 255, 255,
    
    255, 0, 255, 255,
    255, 0, 255, 255,
    255, 0, 255, 255,
    255, 0, 255, 255,
    255, 0, 255, 255,
    255, 0, 255, 255,
    
    255, 255, 0, 255,
    255, 255, 0, 255,
    255, 255, 0, 255,
    255, 255, 0, 255,
    255, 255, 0, 255,
    255, 255, 0, 255
  };
  
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangles);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, colors);
  glDrawArrays (GL_TRIANGLES, 0, 36);
  
  glPopMatrix();
}

void drawVoxelModel()
{

	int i = 0;
	for (int y = 0 ; y < VOXEL_HEIGHT ; y++) {
    
		for (int x = 0 ; x < VOXEL_WIDTH ; x++) {
      
			for (int k = 3 ; k >= 0 ; k--) {	// a,b,g,r
        
				unsigned int bitCode = pixels[i];	// 32-Bit Komponente lesen
				i++;
        
				for (int z = 0 ; z < 32 ; z++) {	// jedes Bit einzeln durchgehen
					
					if (bitCode & 1) {	// wenn Bit gesetzt, zeichne ein Voxel
            
						drawVoxel(x/(float)(VOXEL_WIDTH) - 0.5f, y/(float)(VOXEL_HEIGHT) - 0.5f, 0.25f*(k-2 + z/32.0f));
					}
          
					bitCode = bitCode >> 1;	// das nächste Bit betrachten
				}
			}
		}
	}

}


// OpenGL display Funktion
void display()
{
  
	int timeStart = glutGet(GLUT_ELAPSED_TIME);
  
	// ********** voxelize teapot into integer texture ************
	
	// TODO: FBO binden, in das gerendert werden soll. Clearen Sie das FBO und binden Sie den Voxelisierungs-Shader.
  glBindFramebuffer (GL_FRAMEBUFFER, voxelizationFB);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shaderProgramVoxelization);
	
	// TODO: Viewport auf Auflösung der Voxel-Textur setzen.	
  
  glViewport(0, 0, VOXEL_WIDTH, VOXEL_HEIGHT);
  
	// TODO: Tiefentest deaktivieren.
  glDisable(GL_DEPTH_TEST);
	
	// TODO: Logik-Operation aktivieren. Anstatt den Farbwert in das Target zu schreiben, 
	// werden die Komponenten des Pixels als UINTs aufgefasst und mit dem Pixel im FBO mit OR verknüpft ("reingeodert"...)	
  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_OR);
  
	// Projektionsmatrix setzen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	
	// Viewmatrix auf Einheitsmatrix setzen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
  
	// Kanne rotieren
	glRotatef(angle, 0.0f, 1.0f, 0.0f);
	angle += 0.1f;
  
	// Kanne zeichnen
	glutSolidTeapot(0.7);
	
	// TODO: Rendern in FBO beenden (Backbuffer wieder aktiv) und Fixed-Function Pipeline aktivieren.
  
	// *************** read voxel texture for visualization *************
	// TODO: Texturdaten der Voxelisierungs-Textur auslesen.
  
  glReadPixels(0, 0, VOXEL_WIDTH, VOXEL_HEIGHT, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixels);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(0);
  
	// **************** draw the voxel model ****************
	// TODO: Viewport auf Bildschirmauflösung setzen und Backbuffer clearen.
  
  glViewport(0, 0, PIC_WIDTH, PIC_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// TODO: Tiefentest an, Beleuchtung und Logik-Operationen aus.
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_LOGIC_OP);
	
	// Perspektivische Projektionsmatrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(viewAngle, 1.0f, viewNear, viewFar);	
	
	// Viewmatrix setzen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	calcViewerCamera(theta, phi, r);
	gluLookAt(viewPosition[0], viewPosition[1], viewPosition[2],
            viewPosition[0] + viewDirection[0], viewPosition[1] + viewDirection[1], viewPosition[2] + viewDirection[2], 
            0, 1, 0);
  
	// Voxel Model zeichnen
	drawVoxelModel();
  
	// Szene ist fertig, Buffer swappen.
	glutSwapBuffers();
  
	// Verstrichene Zeit ausgeben.
	int timeEnd = glutGet(GLUT_ELAPSED_TIME);
	printf("Delay %4d\r",timeEnd - timeStart);
}


// use a virtual trackball as mouse control
void mouseMotion(int x, int y)
{
	float deltaX = x - oldX;
	float deltaY = y - oldY;
	
  
  if (motionState == ROTATE) {
    theta -= 0.002f * deltaY;
    
    if (theta < 0.002f) theta = 0.002f;
    else if (theta > PI - 0.002f) theta = PI - 0.002f;
    
    phi += 0.002f * deltaX;	
    if (phi < 0) phi += 2*PI;
    else if (phi > 2*PI) phi -= 2*PI;
  }
  else if (motionState == MOVE) {
    r += 0.01f * deltaY;
    if (r < 0.01f) r = 0.01f;
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



int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutInitWindowSize(PIC_WIDTH, PIC_HEIGHT);
	glutCreateWindow("GPU Voxelization");
  
	// Init glew so that the GLSL functionality will be available
	if(glewInit() != GLEW_OK)
		cout << "GLEW init failed!" << endl;
  
	initFBOTextures();
	initGLSL();
  
	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouse);
	glutDisplayFunc(display);
	glutIdleFunc(display);
  
  glutMainLoop();
  
  return 0;
}



