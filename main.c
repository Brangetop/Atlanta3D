#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // 1 degree to radians that C uses 

float px,py,pdx,pdy,pa; // pa - player angle, d used for delta

int mapX=8,mapY=8,mapS=64;
int map[]=
{
    1,1,1,1,1,1,1,1,
    1,0,0,1,0,0,0,1,
    1,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,
};

void draw2DMap()
{
    int x,y,xo,yo;
    for(y=0;y<mapY;y++) 
    {
        for(x=0;x<mapX;x++)
        {
            if(map[y * mapX + x] == 1)
            {
                glColor3f(1,1,1);
            } else {
                glColor3f(0,0,0);
            }

            xo=x*mapS;
            yo=y*mapS;

            glBegin(GL_QUADS);
            glVertex2i(xo+1,yo+1);
            glVertex2i(xo+1,yo+mapS-1);
            glVertex2i(xo+mapS-1,yo+mapS-1);
            glVertex2i(xo+mapS-1,yo+1);

            glEnd();
        }
        
    }
}

void drawPlayer() 
{
    glColor3f(2,1,0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px,py);
    glEnd();

    glColor3f(2,1,0);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(px,py);
    glVertex2i(px+pdx*10,py+pdy*10);
    glEnd();
}

float dist(float ax, float ay, float bx, float by, float ang)
{
    return( sqrt((bx-ax)*(bx-ax)+(by-ay)*(by-ay)) );
}

void drawRays2D()
{
    int r,mx,my,mp,dof;
    float rx,ry,ra,xo,yo,disT;
    ra=pa-DR*30; if(ra<0){ ra+=2*PI; } if(ra>2*PI){ ra-=2*PI; }

    //normal angle
    // ra=pa;
    for(r=0;r<60;r++)
    {
        // horizontal check
        dof=0;
        float distH=100000,hx=px,hy=py;
        float aTan=-1/tan(ra);

        if(ra>PI) 
        { 
            ry=(((int)py>>6)<<6)-0.0001;
            rx=(py-ry)*aTan+px;
            yo=-64;
            xo=-yo*aTan;
        }
        if(ra<PI) 
        { 
            ry=(((int)py>>6)<<6)+64; 
            rx=(py-ry)*aTan+px;
            yo=64;
            xo=-yo*aTan;
        }
        if(ra==0 || ra==PI) { rx=px; ry=py; dof=8; }
        while(dof<8)
        {
            mx=(int)(rx)>>6; my=(int)(ry)>>6;
            mp=my*mapX+mx;

            if(mp>0 && mp<mapX*mapY && map[mp]>0) { hx=rx; hy=ry; distH=dist(px,py,hx,hy,ra); dof=8; } // hit the wall
            else{ rx+=xo; ry+=yo; dof+=1; }
        }
        //drawing horizontal check line
        // glColor3f(0,1,0); glLineWidth(1); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();

        // --- vertical check ---
        dof=0;
        float disV=1000000,vx=px,vy=py;
        float nTan=-tan(ra);

        if(ra>P2 && ra<P3) 
        { 
            rx=(((int)px>>6)<<6)-0.0001;
            ry=(px-rx)*nTan+py;
            xo=-64;
            yo=-xo*nTan;
        }
        if(ra<P2 || ra>P3) 
        { 
            rx=(((int)px>>6)<<6)+64; 
            ry=(px-rx)*nTan+py;
            xo=64;
            yo=-xo*nTan;
        }
        if(ra==0 || ra==PI) { rx=px; ry=py; dof=8; }
        while(dof<8)
        {
            mx=(int)(rx)>>6; my=(int)(ry)>>6;
            mp=my*mapX+mx;

            if(mp > 0 && mp<mapX*mapY && map[mp]>0) { vx=rx; vy=ry; disV=dist(px,py,vx,vy,ra); dof=8; } // hit the wall
            else{ rx+=xo; ry+=yo; dof+=1; }
        }

        if(disV<distH)
        {
            rx=vx;
            ry=vy;
            disT=disV;
            glColor3f(0.1,0.7,0);
        } 
        else {
            rx=hx;
            ry=hy;
            disT=distH;
            glColor3f(0.1,1,0);
        }
        glLineWidth(1); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();
        
        // ---- drawing 3d lines
        // fisheye fix
        float ca=pa-ra; if(ca<0){ ca+=2*PI; } if(ca>2*PI){ ca-=2*PI; } disT=disT*cos(ca);

        float lineH=(mapS*320)/disT;
        if(lineH>320){ lineH=320; }
        
        float lineO=160-lineH/2;
        float offsetH=170;
        glLineWidth(8);glBegin(GL_LINES);glVertex2i((r*8+350)+offsetH,lineO);glVertex2i((r*8+350)+offsetH,lineH+lineO);glEnd();
        ra+=DR;  if(ra<0){ ra+=2*PI; } if(ra>2*PI){ ra-=2*PI; }
    }
}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    draw2DMap();
    drawRays2D();
    drawPlayer();
    
    glutSwapBuffers();
}

void buttons(unsigned char key, int x, int y)
{
    if(key=='w')
    {
        px+=pdx;
        py+=pdy;
    }
    if(key=='s')
    {
        px-=pdx;
        py-=pdy;
    }
    if(key=='a')
    {
        pa-=0.1;
        if(pa<0) { pa+=2*PI; }
        pdx=cos(pa)*5;
        pdy=sin(pa)*5;
    }
    if(key=='d')
    {
        pa+=0.1;
        if(pa>2*PI) { pa-=2*PI; }
        pdx=cos(pa)*5;
        pdy=sin(pa)*5;
    }

    glutPostRedisplay();
}
void init()
{
    glClearColor(0.3,0.3,0.3,0);
    gluOrtho2D(0,1024,512,0);

    px=300;
    py=300;
    
    pdx=cos(pa)*5;
    pdy=sin(pa)*5;
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024,512);
    glutCreateWindow("Atlanta3D");

    // wayland window size fix
    //glutReshapeWindow(1024,512);

    init();
    glutDisplayFunc(display); 
    glutKeyboardFunc(buttons);
    glutMainLoop();
}
