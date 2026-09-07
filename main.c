#include <stdio.h>
#include <stdlib.h>


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

float px,py; 

void drawPlayer() 
{
    glColor3f(1,1,0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px,py);
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    drawPlayer();
    
    glutSwapBuffers();
}

void buttons(unsigned char key, int x. int y)
{
    
}
void init()
{
    glClearColor(0.3,0.3,0.3,0);
    gluOrtho2D(0,1024,512,0);

    px=300;
    py=300;
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024,512);
    glutCreateWindow("Atlanta3D");
    init();
    glutDisplayFunc(display); 
    glutMainLoop();
}
