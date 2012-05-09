#include <GLUT/glut.h>

int width = 600;
int height = 600;

void drawQuad(float x, float y, float z)
{
	glBegin(GL_QUADS);
	glVertex3f(x,y,z);
	glVertex3f(x+1,y,z);
	glVertex3f(x+1,y+1,z);
	glVertex3f(x,y+1,z);
	glEnd();
}

void display(void)	
{
	/* Aufgabe a, b */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	/* Aufgabe c */
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

	// *** Farben mit Alpha Kanal setzen
	
	/* Aufgabe a */
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	drawQuad(1, 1, -2);
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	drawQuad(0.25, 0.75, -1);
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	drawQuad(0.5, 0.25, 0);

	/* Aufgabe b */
	/*glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
	drawQuad(1, 1, -2);
	glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
	drawQuad(0.25, 0.75, -1);
	glColor4f(0.0f, 0.0f, 1.0f, 0.7f);
	drawQuad(0.5, 0.25, 0);*/

	/* Aufgabe c */
	/*glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	drawQuad(1, 1, -2);
	glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
	drawQuad(0.25, 0.75, -1);
	glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
	drawQuad(0.5, 0.25, 0);*/

	glFlush();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Blending");

	glutDisplayFunc(display);

	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 2, 0, 2, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// *** Blending Funktion setzen
	glEnable(GL_BLEND);

	/* Aufgabe a */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	/* Aufgabe b */
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	/* Aufgabe c */
	//glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	glutMainLoop();

	return 0;
}
