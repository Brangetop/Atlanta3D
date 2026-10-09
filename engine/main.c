#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <GL/glut.h>

#include "textures/textures32.ppm"
#include "textures/skybox.ppm"

#define mapX  8
#define mapY  8
#define mapS 64

// header file later
typedef struct
{
    const char *windowTitle;
    float sensitivityLR;
    float sensitivityMV;
} EngineConfig;

typedef struct
{
    int width;
    int height;
    int *mapW;
    int *mapF;
    int *mapC;
    int *mapD;
} Scene;

typedef struct
{
    EngineConfig engineConfig;
    Scene scene;
    int currentScene;
    
} GameState;


// Need to code the loadScene function that would take number from gameState as an arguement
// And load the scene needed to global game state

// ----- Global scope -----
// Yes, its kinda wrong
// No, you wont stop me from using globals
// static EngineConfig engineConfig;

static GameState gameState;

// loaders.c file later
EngineConfig loadConfig(void) 
{
    FILE *fp;
    char s[64];
    int linecount=0;

    //fp=fopen("config.txt", "r");
    
    //while(fgets(s, sizeof s, fp)!=NULL)
    
    EngineConfig config = 
    {
        .windowTitle="Atlanta3D Engine V2",
        .sensitivityLR=0.2f,
        .sensitivityMV=0.2f
    };

    return config;
}

// NOTICE!
// --- Move this function into file handling C file later on ---
static int readMap(FILE *fp, int *map, size_t count, int isLastMap)
{
    for(size_t i=0; i<count; i++)
    {
        if(fscanf(fp," %d",&map[i])!=1) { return 0; } // Unable to read the number
         
        if(!(isLastMap && i==count-1))
        {
            int ch;
            do { ch=fgetc(fp); } while(ch!=EOF && isspace((unsigned char)ch));

            if(ch!=',') { return 0; }
        }
    }
    return 1;
}

void freeScene(Scene *scene)
{
    free(scene->mapW);
    free(scene->mapF);
    free(scene->mapC);
    free(scene->mapD);

    scene->mapW=NULL;
    scene->mapF=NULL;
    scene->mapC=NULL;
    scene->mapD=NULL;

    scene->width=0;
    scene->height=0;
}

Scene loadScene(int sceneNumber)
{
    Scene scene={0};
    char filename[64];
    
    snprintf(filename, sizeof filename, "scenes/scene%d.txt", sceneNumber);

    FILE *fp=fopen(filename,"r");
    if(fp==NULL) { perror(filename); return scene; }

    if(fscanf(fp,"%d %d",&scene.width,&scene.height)!=2 || scene.width<=0 || scene.height<=0)
    {
        //fprintf("errrors occured while loading scene %s(invalid dimensions)\n",filename);
        fclose(fp);
        return scene;
    }
    
    // Calculating actual scene size and allocating memory
    size_t count=(size_t)scene.width*(size_t)scene.height;

    scene.mapW=malloc(count*sizeof *scene.mapW);
    scene.mapF=malloc(count*sizeof *scene.mapF);
    scene.mapC=malloc(count*sizeof *scene.mapC);
    scene.mapD=malloc(count*sizeof *scene.mapD);
    // add !scene.map* error handling later

    int ok=
        readMap(fp,scene.mapW,count,0) &&
        readMap(fp,scene.mapF,count,0) &&
        readMap(fp,scene.mapC,count,0) &&
        readMap(fp,scene.mapD,count,1);

    fclose(fp);

    if(!ok)
    {
        fprintf(stderr,"Error while reading scene file: %s",filename);
        memset(&scene,0,sizeof scene);
    }

    return scene;
}

GameState initGameState(void)
{
    GameState state={0};

    state.engineConfig=loadConfig();
    state.currentScene=1;
    state.scene=loadScene(state.currentScene);

    return state;
}


// Maybe too? 
typedef struct main
{
    int w,a,s,d;
}ButtonKeys; ButtonKeys Keys;

float sensitivityLR = 0.2;
float sensitivityMV = 0.2;

void drawMap2D(void)
{
    int x,y,xo,yo,index;
    const Scene *scene=&gameState.scene;
    for(y=0;y<scene->height;y++)
    {
        for(x=0;x<scene->width;x++)
        {
            index=y*scene->width+x;
            if(scene->mapW[index]>0){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}
            if(scene->mapW[index]==4){ glColor3f(1,0.7,0.3);}
            xo=x*mapS; yo=y*mapS;
            glBegin(GL_QUADS);
            glVertex2i(xo+1,yo+1);
            glVertex2i(xo+1,yo+mapS-1);
            glVertex2i(xo+mapS-1,yo+mapS-1);
            glVertex2i(xo+mapS-1,yo+1);
            glEnd();
        }
    }
}


// Helper functions
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
    const Scene *scene=&gameState.scene;

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
            rx=(((int)px/mapS)*mapS)+mapS;
            ry=(px-rx)*Tan+py; xo=mapS; yo=-xo*Tan;
        }
        else if(cos(degToRad(ra))<-0.001)
        { 
            rx=(((int)px/mapS)*mapS)-0.0001; 
            ry=(px-rx)*Tan+py; xo=-mapS; 
            yo=-xo*Tan;
        }
        else { rx=px; ry=py; dof=8;} 

        while(dof<8)
        { 
            mx=(int)floorf(rx/mapS); my=(int)floorf(ry/mapS); 
            
            if(mx<0 || mx>=scene->width || my<0 || my>=scene->height) { break; }
            
            mp=my*scene->width+mx;
            if(scene->mapW[mp]>0)
            { 
                vmt=scene->mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py); // hit
            }   
            else{ rx+=xo; ry+=yo; dof++; }
        } 
        vx=rx; vy=ry;

        //horizontal rays
        dof=0; disH=100000;
        Tan=1.0/Tan; 
        if(sin(degToRad(ra))> 0.001){ 
            ry=(((int)py/mapS)*mapS)-0.0001; 
            rx=(py-ry)*Tan+px; yo=-mapS; 
            xo=-yo*Tan;
        }
        else if(sin(degToRad(ra))<-0.001){ 
            ry=(((int)py/mapS)*mapS)+mapS;
            rx=(py-ry)*Tan+px; 
            yo=mapS; 
            xo=-yo*Tan;
        }
        else{ rx=px; ry=py; dof=8;}
        
        while(dof<8) 
        { 
            mx=(int)floorf(rx/mapS); my=(int)floorf(ry/mapS);

            if(mx<0 || mx>=scene->width || my<0 || my>=scene->height) { break; }

            mp=my*scene->width+mx;
            if(scene->mapW[mp]>0){ hmt=scene->mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit        
            else{ rx+=xo; ry+=yo; dof++;}
        } 
        
        //draw the shortest one

        if(disV==100000 && disH==100000)
        {
            ra=FixAng(ra-0.5);
            continue;
        }

        float shade=1;
        if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV;}
        //glLineWidth(2); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd(); // top down rays
        
        // draw 3D
        int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca)); // fisheye fix

        if(disH<0.001f) disH=0.001f;

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
            tx=(int)floorf(rx/2)%32; if (ra>180){tx=31-tx;}   
        } 
        else 
        {
            tx=(int)floorf(ry/2)%32; if (ra>90&&ra<270){tx=31-tx;} 
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
        // something here causes devision by 0. too bad!
        for(y=lineOff+lineH;y<640;y++)
        {
            // ts was causing division by 0
            float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
            if (fabsf(dy) < 0.001f || fabsf(raFix) < 0.001f)
                continue;
        
            tx=px/2 + cos(deg)*158*32*2/dy/raFix;
            ty=py/2 - sin(deg)*158*32*2/dy/raFix;

            mx=(int)floorf(tx/32.0);
            my=(int)floorf(ty/32.0);

            if(mx<0 || mx>=scene->width || my<0 || my>=scene->height)
                continue;

            mp=my*scene->width+mx;
            int texture=scene->mapF[mp]*32*32;

            int pixel=(((int)ty&31)*32+((int)tx&31))*3+texture*3;
            int red=All_Textures[pixel+0]*0.7;
            int green=All_Textures[pixel+1]*0.7;
            int blue=All_Textures[pixel+2]*0.7;
            
            glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,y);glEnd();
            
            // draw roof
            texture=scene->mapC[mp]*32*32;
            pixel=(((int)ty&31)*32+((int)tx&31))*3+texture*3;
            red=All_Textures[pixel+0];
            green=All_Textures[pixel+1];
            blue=All_Textures[pixel+2];
            
            if(texture>=0){glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,640-y);glEnd();}
        
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
    // todo: register keys not like chars
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

        if(gameState.scene.mapW[ipya_yo*mapX+ipxa_xo]==3) { gameState.scene.mapW[ipya_yo*mapX+ipxa_xo]=0;}
    }
    glutPostRedisplay();
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
    glutPostRedisplay();
}

void resize(int w, int h)
{
    glutReshapeWindow(960,640);
}

void display()
{
    float sensLR=gameState.engineConfig.sensitivityLR;
    float sensMV=gameState.engineConfig.sensitivityMV;
    
    frame2=glutGet(GLUT_ELAPSED_TIME);
    fps=(frame2-frame1);
    frame1=glutGet(GLUT_ELAPSED_TIME);
    
    int xo=0; if(pdx<0) { xo=-20; } else { xo=20; }
    int yo=0; if(pdy<0) { yo=-20; } else { yo=20; }
    int ipx=px/64.0, ipxa_xo=(px+xo)/64.0, ipxs_xo=(px-xo)/64.0;
    int ipy=py/64.0, ipya_yo=(py+yo)/64.0, ipys_yo=(py-yo)/64.0;
    
    if(Keys.a==1){ pa+=sensLR*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
    if(Keys.d==1){ pa-=sensLR*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 
    
    if(Keys.w==1)
    {  
        if(gameState.scene.mapW[ipy*gameState.scene.width+ipxa_xo]==0){ px+=pdx*sensMV*fps;}
        if(gameState.scene.mapW[ipya_yo*gameState.scene.width+ipx]==0){ py+=pdy*sensMV*fps;}
    }
    else if(Keys.s==1)
    { 
        if(gameState.scene.mapW[ipy*gameState.scene.width+ipxs_xo]==0){ px-=pdx*sensMV*fps;}
        if(gameState.scene.mapW[ipys_yo*gameState.scene.width+ipx]==0){ py-=pdy*sensMV*fps;}
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
    // Initializing structures
    // engineConfig=loadConfig();
    gameState=initGameState();

    // Initializing GLUT and GL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(960,640);
    glutCreateWindow(gameState.engineConfig.windowTitle);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
    glutMainLoop();
}
