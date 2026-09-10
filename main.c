#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>

#define mapX  8
#define mapY  8
#define mapS 64
int map[]=
{
    1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,1,1,1,
    1,0,1,1,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,1,0,1,0,1,
    1,0,0,1,0,0,0,1,
    1,1,1,1,1,1,1,1,	
};

typedef struct main
{
    /* data */
    int w,a,s,d;
}ButtonKeys; ButtonKeys Keys;

float sensitivityLR = 0.2;
float sensitivityMV = 0.2;

void drawMap2D()
{
    int x,y,xo,yo;
    for(y=0;y<mapY;y++)
    {
        for(x=0;x<mapX;x++)
        {
            if(map[y*mapX+x]==1){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}
            xo=x*mapS; yo=y*mapS;
            glBegin(GL_QUADS); 
            glVertex2i( 0   +xo+1, 0   +yo+1); 
            glVertex2i( 0   +xo+1, mapS+yo-1); 
            glVertex2i( mapS+xo-1, mapS+yo-1);  
            glVertex2i( mapS+xo-1, 0   +yo+1); 
            glEnd();
        } 
    } 
}

float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a)
{
    if(a>359){ a-=360;}
    if(a<0){ a+=360;}
    
    return a;
}

float px,py,pdx,pdy,pa;

void drawPlayer2D()
{
    glColor3f(1,1,0);
    glPointSize(8);
    glLineWidth(4);
    glBegin(GL_POINTS);
    glVertex2i(px,py);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(px,py);
    glVertex2i(px+pdx*20,py+pdy*20);
    glEnd();
}

void Buttons(unsigned char key,int x,int y)
{
    if(key=='a'){ pa+=5; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
    if(key=='d'){ pa-=5; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 
    if(key=='w'){ px+=pdx*5; py+=pdy*5;}
    if(key=='s'){ px-=pdx*5; py-=pdy*5;}

    glutPostRedisplay();
}

void drawRays2D()
{
    // rendering background
    glColor3f(0.4f, 0.18f, 0.05f); glBegin(GL_QUADS); glVertex2i(526,  0); glVertex2i(1006,  0); glVertex2i(1006,160); glVertex2i(526,160); glEnd();	
    glColor3f(0.15f, 0.25f, 0.15f); glBegin(GL_QUADS); glVertex2i(526,160); glVertex2i(1006,160); glVertex2i(1006,320); glVertex2i(526,320); glEnd();	 	
        
    int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
    
    ra=FixAng(pa+30);
    
    for(r=0;r<60;r++)
    {
        dof=0; side=0; disV=100000;
        float Tan=tan(degToRad(ra));

        // horizontal rays
        if(cos(degToRad(ra))> 0.001)
        { 
            rx=(((int)px>>6)<<6)+64;
            ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;
        }
        else if(cos(degToRad(ra))<-0.001)
        { 
            rx=(((int)px>>6)<<6) -0.0001; 
            ry=(px-rx)*Tan+py; xo=-64; 
            yo=-xo*Tan;
        }
        else { rx=px; ry=py; dof=8;} 

        while(dof<8)
        { 
            mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
            if(mp>0 && mp<mapX*mapY && map[mp]==1){ dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit    
            else{ rx+=xo; ry+=yo; dof+=1;}
        } 
        vx=rx; vy=ry;

        //vertical rays
        dof=0; disH=100000;
        Tan=1.0/Tan; 
        if(sin(degToRad(ra))> 0.001){ 
            ry=(((int)py>>6)<<6) -0.0001; 
            rx=(py-ry)*Tan+px; yo=-64; 
            xo=-yo*Tan;
        }
        else if(sin(degToRad(ra))<-0.001){ 
            ry=(((int)py>>6)<<6)+64;
            rx=(py-ry)*Tan+px; 
            yo= 64; 
            xo=-yo*Tan;
        }
        else{ rx=px; ry=py; dof=8;}
        
        while(dof<8) 
        { 
            mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
            if(mp>0 && mp<mapX*mapY && map[mp]==1){ dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit        
            else{ rx+=xo; ry+=yo; dof+=1;}
        } 
        
        //draw the shortest one
        glColor3f(0,0.8,0);
        if(disV<disH){ rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}
        glLineWidth(2); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();
        
        // draw 3D
        int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca)); // fisheye fix
        int lineH = (mapS*320)/(disH); if(lineH>320){ lineH=320;}
        int lineOff = 160 - (lineH>>1);
        
        glLineWidth(8);glBegin(GL_LINES);glVertex2i(r*8+530,lineOff);glVertex2i(r*8+530,lineOff+lineH);glEnd();

        ra=FixAng(ra-1);
    }
}

//initialization
float frame1,frame2,fps;

void init()
{
 glClearColor(0.3,0.3,0.3,0);
 gluOrtho2D(0,1024,510,0);
 px=150; py=400; pa=90;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa)); 
}

void ButtonDown(unsigned char key,int x,int y)
{
    if(key=='w')
    {
        Keys.w=1;
    }
    if(key=='a')
    {
        Keys.a=1;
    }
    if(key=='s')
    {
        Keys.s=1;
    }
    if(key=='d')
    {
        Keys.d=1;
    }

    glutPostRedisplay;
}

void ButtonUp(unsigned char key,int x,int y)
{
    if(key=='w')
    {
        Keys.w=0;
    }
    if(key=='a')
    {
        Keys.a=0;
    }
    if(key=='s')
    {
        Keys.s=0;
    }
    if(key=='d')
    {
        Keys.d=0;
    }

    glutPostRedisplay;
}


void resize(int w, int h)
{
    glutReshapeWindow(1024,512);
}

void display()
{
    frame2=glutGet(GLUT_ELAPSED_TIME);
    fps=(frame2-frame1);
    frame1=glutGet(GLUT_ELAPSED_TIME);
    

    
    int xo=0; if(pdx<0) { xo=-20; } else { xo=20; }
    int yo=0; if(pdy<0) { yo=-20; } else { yo=20; }
    int ipx=px/64.0, ipxa_xo=(px+xo)/64.0, ipxs_xo=(px-xo)/64.0;
    int ipy=py/64.0, ipya_yo=(py+yo)/64.0, ipys_yo=(py-yo)/64.0;
    
    if(Keys.a==1){ pa+=sensitivityLR*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
    if(Keys.d==1){ pa-=sensitivityLR*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 
    
    if(Keys.w==1)
    {  
        if(map[ipy*mapX+ipxa_xo]==0){ px+=pdx*0.2*fps;}
        if(map[ipya_yo*mapX+ipx]==0){ py+=pdy*0.2*fps;}
    }
    if(Keys.s==1)
    {  
        if(map[ipy*mapX+ipxs_xo]==0){ px-=pdx*0.2*fps;}
        if(map[ipys_yo*mapX+ipx]==0){ py-=pdy*0.2*fps;}
    }
    


    glutPostRedisplay();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    drawMap2D();
    drawPlayer2D();
    drawRays2D();
    glutSwapBuffers();  
}

int main(int argc, char* argv[])
{ 
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024,512);
    glutCreateWindow("Atlanta3D");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);

    // older key handling
    // MAKE A BUILD FLAG FOR TS
    //glutKeyboardFunc(Buttons);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
    glutMainLoop();
}
