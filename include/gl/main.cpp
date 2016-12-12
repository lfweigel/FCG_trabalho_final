/*
 * GLUT Shapes Demo
 *
 * Written by Nigel Stewart November 2003
 *
 * This program is test harness for the sphere, cone
 * and torus shapes in GLUT.
 *
 * Spinning wireframe and smooth shaded shapes are
 * displayed until the ESC or q key is pressed.  The
 * number of geometry stacks and slices can be adjusted
 * using the + and - keys.
 */

#include <windows.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "glm.h"
#include "vector3f.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PERSPECTIVE 0
#define ORTHO 1

#define PI 3.14159265



GLMmodel* m_poModel;

GLboolean projection = ORTHO; // ORTHO | PERSPECTIVE
GLfloat fAspect;

vector3f eye;

bool upPressed;
bool downPressed;
bool rightPressed;
bool leftPressed;

GLfloat angleUpDown;
GLfloat angleLeftRight;

//-----------------------------------------------------------------------------
// initialize the class
//-----------------------------------------------------------------------------
bool C3DObject_Init()
{
    if (m_poModel) {

    free(m_poModel);
    m_poModel = NULL;
    }

    glmDelete(m_poModel);

    return true;
}

bool C3DObject_Load(const char *pszFilename)
{
    char aszFilename[256];
    strcpy(aszFilename, pszFilename);

    if (m_poModel) {

    free(m_poModel);
    m_poModel = NULL;
    }

    m_poModel = glmReadOBJ(aszFilename);
    if (!m_poModel)
    return false;

    glmUnitize(m_poModel);
    //glmScale(m_poModel,sFactor); // USED TO SCALE THE OBJECT
    glmFacetNormals(m_poModel);
    glmVertexNormals(m_poModel, 90.0);

    return true;
}

//-----------------------------------------------------------------------------
// draw the object
//-----------------------------------------------------------------------------
void C3DObject_Draw()
{
    if (m_poModel)
    {
        glmDraw(m_poModel, GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
    }

}

/* GLUT callback Handlers */

static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
    if(projection == PERSPECTIVE)
        gluPerspective(60,fAspect,0.1,1000);
    else
        glOrtho(-5, 5, -5, 5, -300, 300);
    gluLookAt(eye.x,eye.y,eye.z,0,0,0,0,1,0);
    glMatrixMode(GL_MODELVIEW);

    glColor3f(1.0,1.0,1.0);

    if(upPressed || downPressed)
        glRotatef(angleUpDown,1,0,0);

    if(leftPressed || rightPressed)
        glRotatef(angleLeftRight,0,1,0);

    C3DObject_Draw();

    glutSwapBuffers();
}

/**
Key press event handler
*/
void onKeyDown(unsigned char key, int x, int y) {
	switch (key) {
		case 119: //w
		    upPressed = true;
			angleUpDown=-5;
			break;
		case 115: //s
		    downPressed = true;
			angleUpDown=5;
			break;
		case 97: //a
		    leftPressed = true;
			angleLeftRight=-5;
			break;
		case 100: //d
		    rightPressed = true;
			angleLeftRight=5;
			break;
        case 112: // p
            if(projection == ORTHO)
                projection = PERSPECTIVE;
            else
                projection = ORTHO;
            break;
		default:
			break;
	}

	//glutPostRedisplay();
}

/**
Key release event handler
*/
void onKeyUp(unsigned char key, int x, int y) {
	switch (key) {
		case 119: //w
			upPressed = false;
			angleUpDown = 0;
			break;
		case 115: //s
			downPressed = false;
			angleUpDown = 0;
			break;
		case 97: //a
			leftPressed = false;
			angleLeftRight = 0;
			break;
		case 100: //d
			rightPressed = false;
			angleLeftRight = 0;
			break;
		case 27:
			exit(0);
			break;
		default:
			break;
	}

	//glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

void initialize()
{
    eye.set(1.5,1.5,1.5);

    fAspect = (GLfloat)WINDOW_WIDTH/(GLfloat)WINDOW_HEIGHT;

    GLfloat angleUp = 0;
    GLfloat angleDown = 0;
    GLfloat angleLeft = 0;
    GLfloat angleRight = 0;
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

/* Program entry point */

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_WIDTH,WINDOW_HEIGHT);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("OBJ Loader - GLM + GLMIMG + TGA TEXTURE");

    C3DObject_Load("penguin.obj");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);

	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);
    glutIdleFunc(idle);

    glClearColor(1,1,1,1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    initialize();

    glutMainLoop();

    return EXIT_SUCCESS;
}
