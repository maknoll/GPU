// *** Transformationen

#include <cmath>
#include <unistd.h>
#include <GLUT/glut.h>

#define PI 3.141592f

#define ROTATE 1
#define MOVE 2

#define fnord fmod

int width = 600;
int height = 600;

float theta = PI / 2.0f - 0.4f;
float phi = 0.0f;
float distance = 25.0f;
float oldX, oldY;
int motionState;

// Winkel, der sich kontinuierlich erhöht. (Kann für die Bewegungen auf den Kreisbahnen genutzt werden)
float angle = 0.0f;

float toDeg(float angle) { return angle / PI * 180.0f; }
float toRad(float angle) { return angle * PI / 180.0f; }

// Zeichnet einen Kreis mit einem bestimmten Radius und einer gewissen Anzahl von Liniensegmenten (resolution) in der xz-Ebene.
void drawCircle(float radius, int resolution)
{
	float winkel = 360 / resolution;

	// Abschalten der Beleuchtung.
	glDisable(GL_LIGHTING);

	// Zeichnen eines Kreises. 
	// Nutzen Sie die Methoden glBegin, glEnd, glVertex3f und ggf. glColor3f um einen GL_LINE_STRIP zu rendern.
    glBegin(GL_LINE_STRIP);
		glColor3f(0, 0, 0);

		for(float i = 0; i <= 360; i += winkel)
			glVertex3f(radius * cos(toRad(i)), 0.0, radius * sin(toRad(i)));
    glEnd();

	// Anschalten der Beleuchtung.
	glEnable(GL_LIGHTING);
}

void display(void)	
{
	// Buffer clearen
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// View Matrix erstellen
	glLoadIdentity();
	float x = distance * sin(theta) * cos(phi);
	float y = distance * cos(theta);
	float z = distance * sin(theta) * sin(phi);
	gluLookAt(x, y, z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Teekanne rendern.
	glutSolidTeapot(1);

	// Den Matrix-Stack sichern.
	glPushMatrix();
	
		// Zeichnen der Kugelkreisbahn.
		drawCircle(10, 360);

		// Zeichnen der Kugel.
		// Wenden Sie eine Translation und eine Rotation an, bevor sie die Kugel zeichnen. Sie können die Variable 'angle' für die Rotation verwenden.
		// Bedenken Sie dabei die richtige Reihenfolge der beiden Transformationen.
		glRotatef(angle, 0, 1, 0);
		glTranslatef(10, 0, 0);
		glutSolidSphere(1, 100, 100);

		// Zeichnen der Würfelkreisbahn.
			// Hinweis: Der Ursprung des Koordinatensystems befindet sich nun im Zentrum des Würfels.
			// Drehen Sie das Koordinatensystem um 90° entlang der Achse, die für die Verschiebung des Würfels genutzt wurde.
			// Danach steht die Würfelkreisbahn senkrecht zur Tangentialrichtung der Kugelkreisbahn.
		glRotatef(90, 1, 0, 0);
		drawCircle(5, 360);

		// Zeichnen des Würfels.
			// Wenden Sie die entsprechende Translation und Rotation an, bevor sie den Würfel zeichnen.
		glRotatef(angle, 0, 1, 0);
		glTranslatef(-5, 0, 0);
		glutSolidCube(1);

		// Zeichnen einer Linie von Würfel zu Kegel.
		glBegin(GL_LINE_STRIP);
			glColor3f(0, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(-3, 0, 0);
		glEnd();

		// Drehung anwenden, sodass Koordinatensystem in Richtung Ursprung orientiert ist. (Hinweis: Implementieren Sie dies zuletzt.)		
		glTranslatef(-3, 0, 0);
		glRotatef(90, 0, 1, 0);

		// Zeichnen des Kegels.
		float distance = sqrt(164 - 160 * cos(toRad(angle)));
		float gamma = toDeg(acos(-((36 - (distance * distance))/(16 * distance))));
		glRotatef(gamma, 0, angle < 180 ? 1 : -1, 0);
		glutSolidCone(0.5, 1, 100, 100);

		// Zeichnen der Linie von Kegel zu Urpsrung.		
		glBegin(GL_LINE_STRIP);
			glColor3f(0, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, distance);
		glEnd();
	
	// Den Matrix-Stack wiederherstellen.
	glPopMatrix();
	
	glutSwapBuffers();	

	angle = fnord(angle + 10.0f / 60.0f, 360);
}

void mouseMotion(int x, int y)
{
	float deltaX = x - oldX;
	float deltaY = y - oldY;
	
	if (motionState == ROTATE) {
		theta -= 0.01f * deltaY;

		if (theta < 0.01f) theta = 0.01f;
		else if (theta > PI/2.0f - 0.01f) theta = PI/2.0f - 0.01f;

		phi += 0.01f * deltaX;	
		if (phi < 0) phi += 2*PI;
		else if (phi > 2*PI) phi -= 2*PI;
	}
	else if (motionState == MOVE) {
		distance += 0.01f * deltaY;
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

void idle(void)
{
	usleep(10000);
	glutPostRedisplay();
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Transformationen");

	glutDisplayFunc(display);
	glutMotionFunc(mouseMotion);
	glutMouseFunc(mouse);
	glutIdleFunc(idle);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glEnable(GL_DEPTH_TEST);

	glViewport(0,0,width,height);					
	glMatrixMode(GL_PROJECTION);					
	glLoadIdentity();								

	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutMainLoop();
	return 0;
}
