// tri1.cpp : Defines the entry point for the console application.
//
 
 
#include <stdlib.h>
#include "glut.h"
#include "glaux.h"
 
#include <math.h> 
#pragma comment(lib,"glaux")
#pragma comment(lib,"legacy_stdio_definitions")
 
unsigned int texture;
AUX_RGBImageRec* image;               


int i, j, alpha=0, beta=0, down=0, oldx, oldy;
int GLx=0, GLy=0, GLz=0;


void   reshape (int width,int height)
{

    glViewport(0,0,width,height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity(); 
    gluPerspective(45, 1, 1 , 40000);
    glMatrixMode( GL_MODELVIEW );
}
 
void display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glPushMatrix();
    glTranslated(GLx, GLy, GLz);
    glRotated(alpha, 0, 1, 0);
    glRotated(beta, -1, 0, 0);

   
    glEnable(GL_TEXTURE_2D);    // включает текстуру
    glColor3ub(190, 194, 12);
    glNormal3f(0, 0, 1);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLfloat BlueCol[3] = {0,0,1};

    glMatrixMode(GL_MODELVIEW);

     
    glBegin(GL_QUADS);
    glTexCoord2d(0 - GLx, 0 - GLy); glVertex3f(0.0 - GLx, 0.0 - GLy, 0.0);
    glTexCoord2d(1 - GLx, 0 - GLy); glVertex3f(1.0 - GLx, 0.0 - GLy, 0.0);
    glTexCoord2d(1 - GLx, 1 - GLy); glVertex3f(1.0 - GLx, 1.0 - GLy, 0.0);
    glTexCoord2d(0 - GLx, 1 - GLy); glVertex3f(0.0 - GLx, 1.0 - GLy, 0.0);

    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
    
}

void Keyboard( unsigned char key, int x, int y )
{
    switch (key)
    {
    case VK_ESCAPE:
    {
        exit(0); 
    }
    case '0': 
    {  
        GLx = 0; GLy = 0; GLz = 0;
        break; 
    }
    case '1': 
    {
        GLx = 1; GLy = 1; GLz = 0;
        break; 
    }
    case '2': 
    {
        GLx = 1; GLy = 0; GLz = 0; 
        break;
    }
    case '3': 
    {
        GLx = 0; GLy = 1; GLz = 0;
        break;
    }
    }
    glutPostRedisplay();
 }

void moving(int x, int y) { // поиск движения мыши
    if (down) {
        alpha = (x - oldx);
        beta = (y - oldy);
        glutPostRedisplay(); // перерисовка окна
    }
}


void mouse(int b, int state, int x, int y) {
    if (b == GLUT_LEFT_BUTTON) { // нажимается левая кнопка
        if (state == GLUT_UP) {
            down = 0;
        }
        else if (state == GLUT_DOWN) {
            down = 1;
            oldx = x;
            oldy = y;
        }
    }
}



int main(int argc, char* argv[])
{

    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize( 512, 512);
    glutInitWindowPosition(50, 10 );
    glutCreateWindow("Test");	
 
    glClearColor(0,0,1,1.0);
    glShadeModel(GL_FLAT); 
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    // light 
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	float pos[4] = {0,0,100,1};
    float dir[3] = {-1,-1,-1};
    GLfloat mat_specular[] = {1,1,1,1};
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
 
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0);
  
	image = auxDIBImageLoad("t1.bmp");             
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);           
	glPixelZoom(1,1);
	glGenTextures(1, &texture); 

	glBindTexture(GL_TEXTURE_2D, texture ); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, image->sizeX, image->sizeY, GL_RGB, GL_UNSIGNED_BYTE, image->data);
 
    glutReshapeFunc( reshape );
    glutKeyboardFunc(Keyboard);
    glTranslated(0, 0, -3.5);
    glutDisplayFunc(display);
    glutMotionFunc(moving);
    glutMouseFunc(mouse);
  
    glutMainLoop();
    glFinish();
    glutSwapBuffers();
	return 0;
}



