#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>

#include "textures/textures32.ppm"
#include "textures/skybox.ppm"

#define mapX  8
#define mapY  8
#define mapS 64
#define doorValue 3

int mapW[]=
{
    2,2,2,2,2,2,2,2,
    2,0,0,0,0,0,0,1,
    2,0,0,0,0,0,0,doorValue,
    2,0,0,0,0,0,0,1,
    2,5,5,doorValue,5,0,0,1,
    5,0,0,0,5,0,0,1,
    5,0,0,0,5,0,0,1,
    2,5,5,5,2,2,2,2,	
};

int mapF[]=
{
    3,3,3,3,3,1,1,1,
    3,3,3,3,3,3,3,1,
    3,3,3,7,7,7,7,1,
    3,3,3,7,3,3,3,1,
    3,3,3,5,3,3,3,3,
    3,5,5,5,3,3,3,3,
    3,5,5,5,3,3,3,3,
    3,3,3,3,3,3,3,3,	
};

int mapC[]=
{
    -1,1,1,1,1,1,1,-1,
    1,-1,-1,-1,-1,-1,-1,1,
    1,-1,-1,-1,-1,-1,-1,1,
    1,-1,-1,-1,-1,-1,-1,1,
    2,2,2,6,0,-1,-1,-1,
    2,6,6,6,0,-1,-1,-1,
    2,6,6,6,0,-1,-1,-1,
    -1,2,2,2,2,1,1,1,	
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
            if(mapW[y*mapX+x]>0){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}
            if(mapW[y*mapX+x]==4) { glColor3f(1,0.7,0.3); } // different color for doors in debug window
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

// momentum system related variables
// will be replaced with speed when i feel like it
float pmom_f,pmom_b,mom_multiplier;
float pmom_accel = 0.0;


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
    // Main walls/floor/ceiling rendering logic
    int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
    
    ra=FixAng(pa+30);
    
    for(r=0;r<120;r++)
    {
        int vmt=0,hmt=0;
        dof=0; side=0; disV=100000;
        float Tan=tan(degToRad(ra));

        // vertical rays
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
            if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit    
            else{ rx+=xo; ry+=yo; dof+=1;}
        } 
        vx=rx; vy=ry;

        //horizontal rays
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
            if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit        
            else{ rx+=xo; ry+=yo; dof+=1;}
        } 
        
        //draw the shortest one

        float shade=1;
        //glColor3f(0,0.8,0);
        if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV;}
        //glLineWidth(2); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd(); // top down rays
        
        // draw 3D
        int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca)); // fisheye fix
        int lineH=(mapS*640)/(disH); 
        
        float ty_step=32.0/(float)lineH;
        float offset=0;
        if(lineH>640){ offset=(lineH-640)/2.0; lineH=640; }
        int lineOff=320-(lineH>>1);
        
        // --- Draw walls ---
        int y;
        float ty=offset*ty_step;
        float tx;
        if(shade==1)
        {
            tx=(int)floor(rx/2)%32; if (ra>180){tx=31-tx;}   
        } 
        else 
        {
            tx=(int)floor(ry/2)%32; if (ra>90&&ra<270){tx=31-tx;} 
        }

        for(y=0;y<lineH;y++) {
            int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
            int red=All_Textures[pixel+0]*shade;
            int green=All_Textures[pixel+1]*shade;
            int blue=All_Textures[pixel+2]*shade;
            
            glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,y+lineOff);glEnd();
            ty+=ty_step;
        }

        // --- Draw floors and roof ---
        for(y=lineOff+lineH;y<640;y++)
        {
            float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
        
            tx=px/2 + cos(deg)*158*32*2/dy/raFix;
            ty=py/2 - sin(deg)*158*32*2/dy/raFix;
            int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;

            int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
            int red=All_Textures[pixel+0]*0.7;
            int green=All_Textures[pixel+1]*0.7;
            int blue=All_Textures[pixel+2]*0.7;
            
            glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,y);glEnd();
            
            // draw roof
            mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
            pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
            red=All_Textures[pixel+0];
            green=All_Textures[pixel+1];
            blue=All_Textures[pixel+2];
            
            if(mp>=0){glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,640-y);glEnd();}
        
        }
        ra=FixAng(ra-0.5);
    }
}

void DrawSkybox()
{
    // Draws sky from texture and rotates it when player angle changes
    glPointSize(8);
    glBegin(GL_POINTS);
    int x,y;
    for(y=0;y<40;y++)
    {
        for(x=0;x<120;x++)
        {
            int xo=(int)(pa*2-x); if(xo<0){xo+=120;} xo=xo%120;
            int pixel=(y*120+xo)*3;
            int red=Skybox[pixel+0];
            int green=Skybox[pixel+1];  
            int blue=Skybox[pixel+2];
            
            glColor3ub(red,green,blue);glVertex2i(x*8,y*8);
        }
    }
    glEnd();        
}

// ---- initialization ----
float frame1,frame2,fps;

void init()
{
    glClearColor(0.3,0.3,0.3,0);
    gluOrtho2D(0,960,640,0);
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

    if(key=='e') { 
        int xo=0; if(pdx<0) { xo=-25; } else { xo=25; }
        int yo=0; if(pdy<0) { yo=-25; } else { yo=25; }

        int ipx=px/64.0, ipxa_xo=(px+xo)/64.0;
        int ipy=py/64.0, ipya_yo=(py+yo)/64.0;

        if(mapW[ipya_yo*mapX+ipxa_xo]==doorValue) { mapW[ipya_yo*mapX+ipxa_xo]=0;}
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
    glutReshapeWindow(960,640);
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
        pmom_f=1;
        if(pmom_accel<1) { pmom_accel+=0.005*fps; }
        if(pmom_accel>1) { pmom_accel=1; }
        if(mapW[ipy*mapX+ipxa_xo]==0){ px+=pdx*0.2*fps*pmom_accel;}
        if(mapW[ipya_yo*mapX+ipx]==0){ py+=pdy*0.2*fps*pmom_accel;}
    }
    else if(Keys.s==1)
    {  
        pmom_b=1;
        if(pmom_accel<1) { pmom_accel+=0.005*fps; }
        if(pmom_accel>1) { pmom_accel=1; }
        if(mapW[ipy*mapX+ipxs_xo]==0){ px-=pdx*0.2*fps*pmom_accel;}
        if(mapW[ipys_yo*mapX+ipx]==0){ py-=pdy*0.2*fps*pmom_accel;}
    }
    else 
    { 
        if(pmom_f==1) { pmom_f=pmom_accel; } if(pmom_b==1) { pmom_b=pmom_accel; }
        pmom_accel=0; 
    }

    if((Keys.w==0 & Keys.s==0)&pmom_b>0)
    {
        if(pmom_b-0.1==0) { pmom_b=0; }
        if(mapW[ipy*mapX+ipxs_xo]==0){ px-=pdx*0.2*fps*pmom_b*mom_multiplier;}
        if(mapW[ipys_yo*mapX+ipx]==0){ py-=pdy*0.2*fps*pmom_b*mom_multiplier;}
        pmom_b-=0.1;
    }

    if((Keys.w==0 & Keys.s==0)&pmom_f>0)
    {
        if(pmom_f-0.07==0) { pmom_f=0; }
        if(mapW[ipy*mapX+ipxa_xo]==0){ px+=pdx*0.2*fps*pmom_f*mom_multiplier;}
        if(mapW[ipya_yo*mapX+ipx]==0){ py+=pdy*0.2*fps*pmom_f*mom_multiplier;}
        pmom_f-=0.07;
    }


    glutPostRedisplay();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    // these will draw 2d top-down world map, created for debugging purposes
    //drawMap2D();
    //drawPlayer2D();
    DrawSkybox();
    drawRays2D();
    glutSwapBuffers();  
}
int main(int argc, char* argv[])
{ 
    mom_multiplier=1.0;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(960,640);
    glutCreateWindow("Atlanta3D Engine 2");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
    glutMainLoop();
}
