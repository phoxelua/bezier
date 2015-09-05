
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>

#include "Parser.h"

#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
public:
	int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport viewport;
bool mode=0, shading=0, wireframe=0; // true is adaptive, false is uniform
GLuint object;
vector<BezPatch> patches(1);
double step = 0.0;
double zoomFactor = 1.0;



//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
	viewport.w = w;
	viewport.h = h;

	glViewport (0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30*zoomFactor, (GLfloat) w/(GLfloat) h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 20, 0, 0, 0, 0, 0, 0, 1);
}

//Return point and derivative
pair<vec3, vec3> bezcurveinterpolate(BezCurve curve, double u)
{
	vec3 A = curve.p0 * (1.0-u) + curve.p1 * u;
	vec3 B = curve.p1 * (1.0-u) + curve.p2 * u;
	vec3 C = curve.p2 * (1.0-u) + curve.p3 * u;

	vec3 D = A * (1.0-u) + B * u;
	vec3 E = B * (1.0-u) + C * u;

	return pair<vec3, vec3>(D * (1.0-u) + E * u, 3*(E - D));
}

//Return surface point and normal
pair<vec3,vec3> bezpatchinterpolate(BezPatch patch, double u, double v)
{
	//Control points in v
	BezCurve vcurve;
	vcurve.p0 = bezcurveinterpolate(patch.c0, u).first;
	vcurve.p1 = bezcurveinterpolate(patch.c1, u).first;
	vcurve.p2 = bezcurveinterpolate(patch.c2, u).first;
	vcurve.p3 = bezcurveinterpolate(patch.c3, u).first;

	//Control points in u
	BezCurve ucurve;
	ucurve.p0 = bezcurveinterpolate(BezCurve(patch.c0.p0, patch.c1.p0, patch.c2.p0, patch.c3.p0, u), v).first;
	ucurve.p1 = bezcurveinterpolate(BezCurve(patch.c0.p1, patch.c1.p1, patch.c2.p1, patch.c3.p1, u), v).first;
	ucurve.p2 = bezcurveinterpolate(BezCurve(patch.c0.p2, patch.c1.p2, patch.c2.p2, patch.c3.p2, u), v).first;
	ucurve.p3 = bezcurveinterpolate(BezCurve(patch.c0.p3, patch.c1.p3, patch.c2.p3, patch.c3.p3, u), v).first;

	pair<vec3,vec3> pdV = bezcurveinterpolate(vcurve, v);
	pair<vec3,vec3> pdU = bezcurveinterpolate(ucurve, u);

	//if (pdV.first != pdU.first) cout << "Points don't match! "<< pdV.first << pdU.first << endl;
	//cout <<(pdU.second^pdV.second).length() << endl;
	return pair<vec3,vec3>(pdV.first, (pdU.second^pdV.second).normalize());

}

void uniformsubdivide(BezPatch patch, double step)
{
	int numdiv = 1.04 / step;
	for (int iu=0; iu<numdiv; iu++){
		double u = iu * step;
		for(int iv=0; iv<numdiv; iv++){
			double v = iv * step;
			pair<vec3,vec3> pn = bezpatchinterpolate(patch, u, v); //do x4 ?
			//Don't store. Draw write(pun intended) away!
			glNormal3f(pn.second[VX], pn.second[VY], pn.second[VZ]);
			glVertex3f(pn.first[VX], pn.first[VY], pn.first[VZ]);
			

			pn = bezpatchinterpolate(patch, u+step, v);
			glNormal3f(pn.second[VX], pn.second[VY], pn.second[VZ]);
			glVertex3f(pn.first[VX], pn.first[VY], pn.first[VZ]);
			

			pn = bezpatchinterpolate(patch, u+step, v+step);
			glNormal3f(pn.second[VX], pn.second[VY], pn.second[VZ]);
			glVertex3f(pn.first[VX], pn.first[VY], pn.first[VZ]);
			

			pn = bezpatchinterpolate(patch, u, v+step);
			glNormal3f(pn.second[VX], pn.second[VY], pn.second[VZ]);
			glVertex3f(pn.first[VX], pn.first[VY], pn.first[VZ]);
			
		}
	}
}

void adaptivesubdivide(BezPatch patch, vec3 p0, vec3 p1, vec3 p2, vec2 u0, vec2 u1, vec2 u2, double step)
{
	vec2 mu01 = (u0+u1)/2, mu12 = (u1+u2)/2, mu02 = (u0+u2)/2;
	vec3 mp01 = (p0+p1)/2, mp12 = (p1+p2)/2, mp02 = (p0+p2)/2;
	
	vec3 mid01 = bezpatchinterpolate(patch, mu01[VX], mu01[VY]).first;
	vec3 mid12 = bezpatchinterpolate(patch, mu12[VX], mu12[VY]).first;
	vec3 mid02 = bezpatchinterpolate(patch, mu02[VX], mu02[VY]).first;

	bool e1 = (mid01 - mp01).length() > step;
	bool e2 = (mid12- mp12).length() > step;
	bool e3 = (mid02 - mp02).length() > step;
	
	if (!e3 && !e2 && !e1) {
		//cout << "yah" << endl;
		
		pair<vec3,vec3> temp = bezpatchinterpolate(patch, u0[VX], u0[VY]);
		glNormal3f(temp.second[VX], temp.second[VY], temp.second[VZ]);
		glVertex3f(p0[VX], p0[VY], p0[VZ]);

		temp = bezpatchinterpolate(patch, u1[VX], u1[VY]);
		glNormal3f(temp.second[VX], temp.second[VY], temp.second[VZ]);
		glVertex3f(p1[VX], p1[VY], p1[VZ]);

		temp = bezpatchinterpolate(patch, u2[VX], u2[VY]);
		glNormal3f(temp.second[VX], temp.second[VY], temp.second[VZ]);
		glVertex3f(p2[VX], p2[VY], p2[VZ]);

		return;
	} else if (!e3 && !e2 && e1) {
		//cout << "eyah1" << endl;
		adaptivesubdivide(patch, mid01, p1, p2, mu01, u1, u2, step);
		adaptivesubdivide(patch, p0, mid01, p2, u0, mu01, u2, step);
	} else if (!e3 &&  e2 && !e1) {
		//cout << "eyah2" << endl;
		//cout << p0 << p1 << p2 <<   endl;
		
		adaptivesubdivide(patch, p0, p1, mid12, u0, u1, mu12, step);
		//cout << "yo" << endl;
		adaptivesubdivide(patch, p0, mid12, p2, u0, mu12, u2, step);
		//cout << "ho" << endl;
	} else if ( e3 && !e2 && !e1) {
		//cout << "eyah3" << endl;
		//cout << p0 << p1 << p2 <<   endl;
		adaptivesubdivide(patch, p0, p1, mid02, u0, u1, mu02, step);
		adaptivesubdivide(patch, mid02, p1, p2, mu02, u1, u2, step);
	} else if (!e3 &&  e2 &&  e1) {
		//cout << "eyah4" << endl;
		adaptivesubdivide(patch, mid01, p1, mid12, mu01, u1, mu12, step);
		adaptivesubdivide(patch, mid01, mid12, p2, mu01, mu12, u2, step);
		adaptivesubdivide(patch, p0, mid01, p2, u0, mu01, u2, step);
	} else if ( e3 &&  e2 && !e1) {
		//cout << "eyah5" << endl;
		adaptivesubdivide(patch, p0, p1, mid12, u0, u1, mu12, step);
		adaptivesubdivide(patch, p0, mid12, mid02, u0, mu12, mu02, step);
		adaptivesubdivide(patch, mid02, mid12, p2, mu02, mu12, u2, step);
	} else if ( e3 && !e2 &&  e1) {
		//cout << "eyah6" << endl;
		adaptivesubdivide(patch, mid02, p1, p2, mu02, u1, u2, step);
		adaptivesubdivide(patch, mid01, p1, mid02, mu01, u1, mu02, step);
		adaptivesubdivide(patch, p0, mid01, mid02, u0, mu01, mu02, step);
	} 
	 else if ( e3 &&  e2 &&  e1) {
		//cout << "eyah7" << endl;
		adaptivesubdivide(patch, mid01, p1, mid12, mu01, u1, mu12, step);
		adaptivesubdivide(patch, mid01, mid12, mid02, mu01, mu12, mu02, step);
		adaptivesubdivide(patch, p0, mid01, mid02, u0, mu01, mu02, step);
		adaptivesubdivide(patch, mid02, mid12, p2, mu02, mu12, u2, step);
	}
}



//****************************************************
// Simple init function
//****************************************************
void initScene(){
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_FLAT);

	object = glGenLists(1);
	glNewList(object, GL_COMPILE);
	if (mode){
		glBegin(GL_TRIANGLES);
		//for each patch i in all patches
		vec3 p0, p1, p2, p3;

		for (int i = 0; i<patches.size(); i++) {
			cout << "%";
			p0 = bezpatchinterpolate(patches[i], 0, 0).first;
			p1 = bezpatchinterpolate(patches[i], 0, 1).first;
			p2 = bezpatchinterpolate(patches[i], 1, 1).first;
			p3 = bezpatchinterpolate(patches[i], 1, 0).first;
			adaptivesubdivide(patches[i], p0, p1, p2, vec2(0,0), vec2(0,1), vec2(1,1), step);
			adaptivesubdivide(patches[i], p0, p2, p3, vec2(0,0), vec2(1,1), vec2(1,0), step);
		}
		glEnd();
	}
	else{
		//for each patch i in all patches
		glBegin(GL_QUADS);
		for (int i = 0; i<patches.size(); i++) {
			cout << "%";
			uniformsubdivide(patches[i], step);
		}
		glEnd();
	}
	cout << endl;
	glEndList();

	glClearColor(0.0, 0.0, 0.0, 0.0);
}


//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f (1.0, 0.0, 0.0);
	glCallList(object);
	glFlush();
	glutSwapBuffers();  
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 's':
		if (shading) { 
			glShadeModel(GL_FLAT);
		} else {
			glShadeModel(GL_SMOOTH);
		}
		glutPostRedisplay();
		shading = !shading;
		break;
	case 'w':
		if (wireframe) { 
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		} else {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		glutPostRedisplay();
		wireframe = !wireframe;
		break;
	case '+':
		zoomFactor -= 0.1;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(30*zoomFactor, (GLfloat) viewport.w/(GLfloat) viewport.h, 1.0, 100.0);
		glutPostRedisplay();
		break;
	case '-':
		zoomFactor += 0.1;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(30*zoomFactor, (GLfloat) viewport.w/(GLfloat) viewport.h, 1.0, 100.0);
		glutPostRedisplay();
		break;
	case 27:
		exit(0);
		break;
	}
}

void arrows(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		glRotatef(15.,1.0,0.0,0.0);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		glRotatef(-15.,1.0,0.0,0.0);
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		glRotatef(-15.,0.0,1.0,0.0);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		glRotatef(15.,0.0,1.0,0.0);
		glutPostRedisplay();
		break;
	}
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
	
	if (argc > 1){
		mode = (argc == 4);
		step = atof(argv[2]);
		loadScene(argv[1], patches, step, step);
		//cout << patches[0].c1.p0  << " " << patches[0].c2.p1  << " " << patches[0].c3.p2  << " " << patches[0].u << endl;
		cout << "Step Size: " << step << endl;
		cout << "Done parsing." << endl;
	}
	else {
		cout << "Unable to find file. No arguments given." << endl;
	}

	//This initializes glut
	glutInit(&argc, argv);

	//This tells glut to use a double-buffered window with red, green, and blue channels 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Initalize theviewport size
	viewport.w = 800;
	viewport.h = 800;

	//The size and position of the window
	glutInitWindowSize(viewport.w, viewport.h);
	glutInitWindowPosition(0,0);
	glutCreateWindow(argv[0]);

	initScene();							// quick function to set up scene

	glutDisplayFunc(myDisplay);				// function to run when its time to draw something
	glutReshapeFunc(myReshape);				// function to run when the window gets resized
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrows);
	glutMainLoop();							// infinite loop that will keep drawing and resizing
	// and whatever else

	return 0;
}








