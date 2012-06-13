// Super-Sampling Anti-Aliasing

#include <stdlib.h>
#include <GLUT/glut.h>
#include <iostream>
using namespace std;

// Global variables
GLfloat alpha = 0;

// Texture Ids and Framebuffer Object Ids
GLuint sceneTextureId = 0;
GLuint depthTextureId = 0;
GLuint sceneFB = 0;
GLuint checkBoardTextureId = 0;

// Window size
int width = 512;       
int height = 512;

bool useSSAA = true;
int samples = 2;	// Die Auflösung wird um das 2^'samples'-fache vergrößert.

// Das Schachbrett kann über kleine schwarze und weiße Quads oder über ein großes texturiertes Quad gerendert werden.
bool useTexturedQuad = true;

// Flag das entscheidet, ob MSAA auf sample- oder pixel-frequency läuft.
bool usePerSampleShading = false;

void initGL()
{
	// Initialize camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);

	// Initialize light source
	GLfloat light_pos[] = {10, 10, 10, 1};
	GLfloat light_col[] = { 1,  1,  1, 1};

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_col);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_col);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);

	// Per-Sample shading aktivieren.
	usePerSampleShading = true;
}

int initFBOTextures()
{
	// Textur (fuer Teapot Bild) anlegen
	glGenTextures (1, &sceneTextureId);
	glBindTexture (GL_TEXTURE_2D, sceneTextureId);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, width*(1<<samples), height*(1<<samples), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	// Der Min-Filter führt derzeit noch kein MipMapping durch. Nutzen Sie auch den Nearest-Filter für das MipMapping!
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	// <- hier den letzten Parameter ändern
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Depth Buffer Textur anlegen 
	glGenTextures (1, &depthTextureId);
	glBindTexture (GL_TEXTURE_2D, depthTextureId);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width*(1<<samples), height*(1<<samples), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// FBO (fuer Teapot Textur) anlegen und Texturen zuweisen
	glGenFramebuffers (1, &sceneFB);
	glBindFramebuffer (GL_FRAMEBUFFER, sceneFB);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTextureId, 0);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);

	// Binden der Szenen Textur
	glBindTexture(GL_TEXTURE_2D, sceneTextureId);
	// Setzen Sie die mindest LOD Stufe auf 'samples'. Nutzen Sie dafpr die Texture Filter Control von glTexEnvf
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, samples);	

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
	glBindFramebufferEXT (GL_FRAMEBUFFER, 0);
	return 0;
}

#define _X_ 0,0,0,255
#define _O_ 255,255,255,255

void initTexture()
{
	unsigned char data[] = {
		_O_, _X_,
		_X_, _O_,
	};

	GLuint texWidth = 2;
	GLuint texHeight = 2;

	// Erzeugen eines Texturnames (handle).
	glGenTextures(1, &checkBoardTextureId);

	// Binden der Textur. (Hinweis: Das target heißt GL_TEXTURE_2D)
	glBindTexture(GL_TEXTURE_2D, checkBoardTextureId);

	// Füllen der Textur.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	// Min und Mag Filter auf Nearest setzen (keine Interpolation).
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
}

void keyboard(unsigned char key, int x, int y)
{
	// set parameters
	switch (key) 
	{       
		case '1':
			useSSAA = !useSSAA;
			printf(useSSAA ? "SSAA/" : (usePerSampleShading ? "MSAA(sample)/" : "MSAA(pixel)/") );
			printf(useTexturedQuad ? "Texture        \r" : "Geometry        \r");
			break;
		case '2':
			useTexturedQuad = !useTexturedQuad;
			printf(useSSAA ? "SSAA/" : (usePerSampleShading ? "MSAA(sample)/" : "MSAA(pixel)/") );
			printf(useTexturedQuad ? "Texture        \r" : "Geometry        \r");
			break;
		case '3':
			usePerSampleShading = !usePerSampleShading;
			
			// Abhängig vom usePerSampleShading-Flag einstellen, für wieviele Samples der Fragment Shader ausgeführt werden soll.
			// Per-Sample Shading = für alle Samples
			if(usePerSampleShading)
				samples = 2;
				//glMinSampleShadingARB(1);
			// Per-Pixel Shading = für ein einziges Sample
			if(!usePerSampleShading)
				samples = 1;
				//glMinSampleShadingARB(0);

			printf(useSSAA ? "SSAA/" : (usePerSampleShading ? "MSAA(sample)/" : "MSAA(pixel)/") );
			printf(useTexturedQuad ? "Texture        \r" : "Geometry        \r");
			break;
	}
}

void drawGround()
{
	float size = 7;
	int resolution = 6;
	glPushMatrix();
	glTranslatef(0, -2, 0);	
	if (useTexturedQuad)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, checkBoardTextureId);
		glBegin(GL_QUADS);
			glNormal3f(0,1,0);
			glColor3f(1,1,1);
			glTexCoord2f((float)resolution, (float)resolution);
			glVertex3f( size, 0,  size);

			glTexCoord2f((float)resolution,0);
			glVertex3f( size, 0, -size);

			glTexCoord2f(0,0);
			glVertex3f(-size, 0, -size);

			glTexCoord2f(0,(float)resolution);
			glVertex3f(-size, 0,  size);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{	
		glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		for (int x=-resolution; x<resolution; ++x)
		{
			for (int y=-resolution; y<resolution; ++y)
			{	
				if ( (x+y)%2 == 0 )
					glColor3f(1,1,1);
				else glColor3f(0,0,0);
				glVertex3f((x+1) / (float)resolution * size, 0, (y+1) / (float)resolution * size);
				glVertex3f((x+1) / (float)resolution * size, 0,  y    / (float)resolution * size);
				glVertex3f( x    / (float)resolution * size, 0,  y    / (float)resolution * size);
				glVertex3f( x    / (float)resolution * size, 0, (y+1) / (float)resolution * size);				
			}
		}
		glEnd();
	}	
	glPopMatrix();
}

// Bildschirmfuellendes Rechteck zeichnen -> Fragment Program wird fuer jedes Pixel aufgerufen
void drawScreenFillingQuad() 
{
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0,0);
		glVertex2f(-1,-1);
		glTexCoord2f(1,0);
		glVertex2f( 1,-1);
		glTexCoord2f(1,1);
		glVertex2f(1,1);
		glTexCoord2f(0,1);
		glVertex2f( -1,1);
	}       
	glEnd();

	glPopMatrix();	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
} 

void drawScene()
{
	glLoadIdentity();
	gluLookAt(10, 7, 10, 0, 0, 0, 0, 1, 0);

	glRotatef(alpha, 0, 1, 0);
	drawGround();
	
	glutSolidTeapot(3);
}

void display()
{	
	// Soll SSAA verwendet werden?
	if (useSSAA)
	{
		// Größe des Viewports auf das 2^(samples) fache setzen
		glViewport(0, 0, width * (1<<samples), height * (1<<samples));

		// Binden des FBOs, in das gerendert werden soll.
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, sceneFB);

		// Clear Color- und Depth-Buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Rendern der Szene
		drawScene();

		// Viewport auf die Auflösung des Backbuffers setzen.
		glViewport(0, 0, width, height);
		
		// FBO abschalten: jetzt wird wieder in den Backbuffer gerendert
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		// Binden der Szenen-Textur.
		glBindTexture(GL_TEXTURE_2D, sceneTextureId);

		// Da die Textur nun aktiv ist, müssen die MipMap Stufen neu generiert werden.
		glGenerateMipmapEXT(GL_TEXTURE_2D);

		// Color- und Depth-Buffer clearen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Das Fullscreen Quad rendern, das mit der FBO Textur texturiert ist.		
		drawScreenFillingQuad();

		// Textur nicht mehr binden.
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		// Clear window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Dis Szene rendern.
		drawScene();
	}

	// Increment rotation angle
	alpha += 1;

	// Swap display buffers
	glutSwapBuffers();
}

void timer(int value)
{
   // Call timer() again in 25 milliseconds
   glutTimerFunc(25, timer, 0);

   // Redisplay frame
   glutPostRedisplay();
}

int main(int argc, char** argv)
{
   // Initialize GLUT
   glutInit(&argc, argv);

   // Enable Multi-Sampling
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
   glutInitWindowSize(width, height);
   glutCreateWindow("Super-Sampling Anti-Aliasing");

	// OpenGL/GLSL initializations
	initGL();
	initFBOTextures();
	initTexture();

	// Register callback functions   
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutTimerFunc(25, timer, 0);     // Call timer() in 25 milliseconds

    // Mipmapping Fix
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0);

	// Enter main loop
	glutMainLoop();

	return 0;
}
