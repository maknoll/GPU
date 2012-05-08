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
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

	// *** Farben mit Alpha Kanal setzen
	drawQuad(1, 1, -2);
	drawQuad(0.25, 0.75, -1);
	drawQuad(0.5, 0.25, 0);

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

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 2, 0, 2, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// *** Blending Funktion setzen

	glutMainLoop();
	return 0;
}
