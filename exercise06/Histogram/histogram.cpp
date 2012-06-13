// ******* GPU Histogram ********

//#include <windows.h>

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

#define PI 3.141592f

#define ROTATE 1
#define MOVE 2

float center[3] = {0.0f, 0.0f, 0.0f}; 
GLfloat viewPosition[4] = {0.0, 0.0, 3.0, 1.0};  
GLfloat viewDirection[4] = {-0.0, -0.0, -1.0, 0.0};  
GLfloat viewAngle = 45.0f;
GLfloat viewNear = 0.01f;
GLfloat viewFar = 10000.0f;

float thetaStart = PI / 2.0f;
float phiStart = PI / 2.0f;
float rStart = 3.5f;

float theta = thetaStart;
float phi = phiStart;
float r = rStart;

float oldX, oldY;
int motionState;

// Texture Ids
GLuint imageTextureId = 0;
GLuint depthTextureId = 0;
GLuint histogramTextureId = 0;

// Framebuffer Object Ids
GLuint createImageFB = 0;
GLuint createHistogramFB = 0;

// GLSL Variables (Shader Ids, Locations, ...)
GLuint vertexShaderCreateHistogram;	
GLuint fragmentShaderCreateHistogram;
GLuint shaderProgramCreateHistogram;

GLint imageTextureLocation;

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
	glGenTextures (1, &imageTextureId);
	glBindTexture (GL_TEXTURE_2D, imageTextureId);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, PIC_WIDTH, PIC_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Depth Buffer Textur anlegen 
	glGenTextures (1, &depthTextureId);
	glBindTexture (GL_TEXTURE_2D, depthTextureId);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, PIC_WIDTH, PIC_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// FBO anlegen und Texturen zuweisen
	glGenFramebuffers (1, &createImageFB);
	glBindFramebuffer (GL_FRAMEBUFFER, createImageFB);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imageTextureId, 0);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);

	// texture for viewer position
	glGenTextures (1, &histogramTextureId);
	glBindTexture (GL_TEXTURE_2D, histogramTextureId);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB32F_ARB, PIC_WIDTH, PIC_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// MRT FBO for position/normal/color
	glGenFramebuffers (1, &createHistogramFB);
	glBindFramebuffer (GL_FRAMEBUFFER, createHistogramFB);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, histogramTextureId, 0);
	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, imageTextureId);


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
	loadShaderProgram(shaderProgramCreateHistogram, vertexShaderCreateHistogram, "create_histogram.vert", fragmentShaderCreateHistogram, "create_histogram.frag");
	glUseProgram(shaderProgramCreateHistogram);

	imageTextureLocation = glGetUniformLocation( shaderProgramCreateHistogram, "imageTexture" );
	glUniform1i(imageTextureLocation, 0);   
}

void drawScene()
{
	// Tiefentest und Beleuchtung anschalten.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	// Perspektivische Projektionsmatrix verwenden.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	gluPerspective(viewAngle, 1.0f, viewNear, viewFar);	
	
	// View-Matrix setzen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	calcViewerCamera(theta, phi, r);
	gluLookAt(viewPosition[0], viewPosition[1], viewPosition[2],
			  viewPosition[0] + viewDirection[0], viewPosition[1] + viewDirection[1], viewPosition[2] + viewDirection[2], 
			  0, 1, 0);	

	// Teekanne mit Fixed-Function-Pipeline rendern.
	glUseProgram( 0 );
	glutSolidTeapot(1.0);
}

// OpenGL display Funktion
void display()
{
	float hPixels[256*3];

	int timeStart = glutGet(GLUT_ELAPSED_TIME);

	// ********* Teekanne in FBO rendern **********
		
	// FBO binden, in das gerendert werden soll.
	glBindFramebuffer (GL_FRAMEBUFFER, createImageFB);      // activate fbo                 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Teekanne rendern.
	drawScene();
	
	// ********* Histogrammdaten erzeugen, indem für jedes Pixel ein Vertex gerendert wird. Der Vertex Shader liest den Farbwert aus und berechnet seinen Position, abhängig von der Helligkeit des gelesenen Pixels. **********
		
	// Orthografische Projektion nutzen, damit die Vertices auch korrekt auf die Pixel fallen.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, PIC_WIDTH, 0, PIC_HEIGHT, -1, 1);
	
	// Einheitsmatrix als Model-View Matrix verwenden. (Blick in z-Richtung)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// TODO: Additives Blending aktivieren (mit jedem ankommenden Pixel wird der Counter um 1 erhöht)
	
	// TODO: Tiefentest und Beleuchtung abschalten
	
	// TODO: Histogram-Shader für jedes Pixel des Bildes ausführen.	

	// Histogramm-Daten von VRAM zu RAM streamen (in das Array hPixels)
	glReadPixels(0, 0, 256, 1, GL_RGB, GL_FLOAT, hPixels);

	// TODO: Blending abschalten	
	
	// ********* Teekanne in Backbuffer rendern **********

	// Rendern in das FBO beenden. Fortan wird wieder in den Backbuffer gerendert.
	glBindFramebuffer (GL_FRAMEBUFFER, 0);       // deactivate fbo
	// Backbuffer wieder leeren.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Szene in den Backbuffer rendern.
	drawScene();

	// ********* Histogramm zeichnen **********

	// TODO: Render Sie die Liniensegmente. Sie können ModelView- und Projection-Matrix auf 
	// die Einheitsmatrix setzen und die Vertices direkt im Clipping-Space an die GPU schicken
	// oder alternativ eine Projektionsmatrix bauen, die es Ihnen erlaubt, die Positionen in
	// Bildschirmkoordinaten (0..PIC_WIDTH-1, 0..PIC_HEIGHT-1) anzugeben.
	
	// Frame ist beendet, Buffer swappen.
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
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutInitWindowSize(PIC_WIDTH, PIC_HEIGHT);
	glutCreateWindow("GPU Histogram");

	// Init glew so that the GLSL functionality will be available
	if(glewInit() != GLEW_OK)
		cout << "GLEW init failed!" << endl;

	initFBOTextures();
	initGLSL();

	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouse);	
	glutDisplayFunc(display);
	glutIdleFunc(display);

	glViewport(0, 0, PIC_WIDTH, PIC_HEIGHT);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(viewAngle, 1.0, viewNear, viewFar);
	glMatrixMode(GL_MODELVIEW);

    glutMainLoop();
                
    return 0;
}



