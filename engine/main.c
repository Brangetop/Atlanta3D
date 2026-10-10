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

float degToRad(float a);

// header file later
typedef struct
{
    const char *windowTitle;
    float sensitivityLR;
    float sensitivityMV;
} EngineConfig;

typedef struct
{
    float r_bright_ws,g_bright_ws,b_bright_ws;
    float r_dark_ws,g_dark_ws,b_dark_ws;
    float r_fs,g_fs,b_fs;
    float r_cs,g_cs,b_cs;
    float r_sb,g_sb,b_sb;
} Shader;

typedef struct
{
    float x,y,ang;
    float dx,dy;
} Player;

typedef struct
{
    int width;
    int height;
    int *mapW;
    int *mapF;
    int *mapC;
    int *mapD;

    Player spawn;
} Scene;

typedef struct
{
    // Data for logic part
    EngineConfig engineConfig;
    int currentScene;
    Player player;
    // data for renderer
    // need to just pass ts to renderer so its separated
    Scene scene;
    Shader shader; // NOTE: shader is linked to a scene! if you have multiple scenes, you must have a shader for them all.
} GameState;

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
void updatePlayerDirection(Player *player)
{
    player->dx = cosf(degToRad(player->ang));
    player->dy = -sinf(degToRad(player->ang));
}

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

    if (fscanf(fp,"%f %f %f",&scene.spawn.x,&scene.spawn.y,&scene.spawn.ang) != 3) 
    {
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

Shader loadShader(int shaderNumber)
{
    Shader shader={0.0f,0.0f,0.0f,0.0f};
    char filename[64];

    snprintf(filename,sizeof(filename),"shaders/shader%d.txt",shaderNumber);

    FILE *fp=fopen(filename,"r");
    if(fp==NULL) {perror(filename); return shader;}

    int read_count=fscanf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", // Maybe color struct lol?
        &shader.r_bright_ws,&shader.g_bright_ws,&shader.b_bright_ws,
        &shader.r_dark_ws,  &shader.g_dark_ws,  &shader.b_dark_ws,
        &shader.r_fs,       &shader.g_fs,       &shader.b_fs,
        &shader.r_cs,       &shader.g_cs,       &shader.b_cs,
        &shader.r_sb,       &shader.g_sb,       &shader.b_sb);

    if(read_count!=15) { fprintf(stderr,"error while reading from %s",filename); return shader; }
    fclose(fp);

    return shader;
}

void updateScene(void)
{
    freeScene(&gameState.scene);
    gameState.scene=loadScene(gameState.currentScene);
    gameState.player=gameState.scene.spawn;
    
    updatePlayerDirection(&gameState.player);
    gameState.shader=loadShader(gameState.currentScene);
}


GameState initGameState(void)
{
    GameState state={0};

    state.engineConfig=loadConfig();
    state.currentScene=1;

    state.scene=loadScene(state.currentScene);
    state.shader=loadShader(state.currentScene);

    state.player = state.scene.spawn;
    updatePlayerDirection(&state.player);

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

//float px,py,gameState.player.dx,pdy,pa;

void drawPlayer2D()
{
    glColor3f(1,1,0);
    glPointSize(8);
    glLineWidth(4);
    glBegin(GL_POINTS);
    glVertex2i(gameState.player.x,gameState.player.y);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(gameState.player.x,gameState.player.y);
    glVertex2i(gameState.player.x+gameState.player.dx*20,gameState.player.y+gameState.player.dy*20);
    glEnd();
}

void Buttons(unsigned char key,int x,int y)
{
    if(key=='a'){ gameState.player.ang+=5; gameState.player.ang=FixAng(gameState.player.ang); gameState.player.dx=cos(degToRad(gameState.player.ang)); gameState.player.dy=-sin(degToRad(gameState.player.ang));} 	
    if(key=='d'){ gameState.player.ang-=5; gameState.player.ang=FixAng(gameState.player.ang); gameState.player.dx=cos(degToRad(gameState.player.ang)); gameState.player.dy=-sin(degToRad(gameState.player.ang));} 
    if(key=='w'){ gameState.player.x+=gameState.player.dx*5; gameState.player.y+=gameState.player.dy*5;}
    if(key=='s'){ gameState.player.x-=gameState.player.dx*5; gameState.player.y-=gameState.player.dy*5;}

    glutPostRedisplay();
}

void drawRays2D()
{
    const Scene *scene=&gameState.scene;
    const int MAX_DOF=(scene->width>scene->height)?scene->width:scene->height;

    int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
    
    ra=FixAng(gameState.player.ang+30);
    
    for(r=0;r<120;r++)
    {
        int vmt=0,hmt=0;
        dof=0; side=0; disV=100000;
        float Tan=tan(degToRad(ra));

        // vertical rays
        if(cos(degToRad(ra))> 0.001)
        { 
            rx=(((int)gameState.player.x/mapS)*mapS)+mapS;
            ry=(gameState.player.x-rx)*Tan+gameState.player.y; xo=mapS; yo=-xo*Tan;
        }
        else if(cos(degToRad(ra))<-0.001)
        { 
            rx=(((int)gameState.player.x/mapS)*mapS)-0.0001; 
            ry=(gameState.player.x-rx)*Tan+gameState.player.y; xo=-mapS; 
            yo=-xo*Tan;
        }
        else { rx=gameState.player.x; ry=gameState.player.y; dof=MAX_DOF;} 

        while(dof<MAX_DOF)
        { 
            mx=(int)floorf(rx/mapS); my=(int)floorf(ry/mapS); 
            
            if(mx<0 || mx>=scene->width || my<0 || my>=scene->height) { break; }
            
            mp=my*scene->width+mx;
            if(scene->mapW[mp]>0)
            { 
                vmt=scene->mapW[mp]-1; dof=MAX_DOF; disV=cos(degToRad(ra))*(rx-gameState.player.x)-sin(degToRad(ra))*(ry-gameState.player.y); // hit
            }   
            else{ rx+=xo; ry+=yo; dof++; }
        } 
        vx=rx; vy=ry;

        //horizontal rays
        dof=0; disH=100000;
        Tan=1.0/Tan; 
        if(sin(degToRad(ra))> 0.001){ 
            ry=(((int)gameState.player.y/mapS)*mapS)-0.0001; 
            rx=(gameState.player.y-ry)*Tan+gameState.player.x; yo=-mapS; 
            xo=-yo*Tan;
        }
        else if(sin(degToRad(ra))<-0.001){ 
            ry=(((int)gameState.player.y/mapS)*mapS)+mapS;
            rx=(gameState.player.y-ry)*Tan+gameState.player.x; 
            yo=mapS; 
            xo=-yo*Tan;
        }
        else{ rx=gameState.player.x; ry=gameState.player.y; dof=MAX_DOF;}
        
        while(dof<MAX_DOF) 
        { 
            mx=(int)floorf(rx/mapS); my=(int)floorf(ry/mapS);

            if(mx<0 || mx>=scene->width || my<0 || my>=scene->height) { break; }

            mp=my*scene->width+mx;
            if(scene->mapW[mp]>0){ hmt=scene->mapW[mp]-1; dof=MAX_DOF; disH=cos(degToRad(ra))*(rx-gameState.player.x)-sin(degToRad(ra))*(ry-gameState.player.y);}//hit        
            else{ rx+=xo; ry+=yo; dof++;}
        } 
        
        //draw the shortest one

        if(disV==100000 && disH==100000)
        {
            ra=FixAng(ra-0.5);
            continue;
        }

        //float shade=gameState.shader.bright_ws;
        int is_vertical=0;

        if(disV<disH){ hmt=vmt; is_vertical=1; rx=vx; ry=vy; disH=disV;}
        //glLineWidth(2); glBegin(GL_LINES); glVertex2i(gameState.player.x,gameState.player.y); glVertex2i(rx,ry); glEnd(); // top down rays
        
        // draw 3D
        int ca=FixAng(gameState.player.ang-ra); disH=disH*cos(degToRad(ca)); // fisheye fix

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
        if(!is_vertical)
        {
            tx=(int)floorf(rx/2)%32; if (ra>180){tx=31-tx;}   
        } 
        else 
        {
            tx=(int)floorf(ry/2)%32; if (ra>90&&ra<270){tx=31-tx;} 
        }

        // rgb multipliers just for shading
        float sh_r, sh_g, sh_b;
        if (!is_vertical) {
            sh_r = gameState.shader.r_bright_ws;
            sh_g = gameState.shader.g_bright_ws;
            sh_b = gameState.shader.b_bright_ws;
        } else {
            sh_r = gameState.shader.r_dark_ws;
            sh_g = gameState.shader.g_dark_ws;
            sh_b = gameState.shader.b_dark_ws;
        }

        for(y=0;y<lineH;y++) 
        {
            int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
            int red=All_Textures[pixel+0]*sh_r;
            int green=All_Textures[pixel+1]*sh_g;
            int blue=All_Textures[pixel+2]*sh_b;
            
            glPointSize(8);glColor3ub(red,green,blue);glBegin(GL_POINTS);glVertex2i(r*8,y+lineOff);glEnd();
            ty+=ty_step;
        }

        // --- Draw floors and roof ---
        // something here causes devision by 0. too bad!
        glPointSize(8);
        glBegin(GL_POINTS);
        for(y=lineOff+lineH;y<640;y++)
        {
            // ts was causing division by 0
            float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(gameState.player.ang-ra)));
            if (fabsf(dy) < 0.001f || fabsf(raFix) < 0.001f)
                continue;
        
            tx=gameState.player.x/2 + cos(deg)*158*32*2/dy/raFix;
            ty=gameState.player.y/2 - sin(deg)*158*32*2/dy/raFix;

            mx=(int)floorf(tx/32.0);
            my=(int)floorf(ty/32.0);

            if(mx<0 || mx>=scene->width || my<0 || my>=scene->height)
                continue;

            mp=my*scene->width+mx;
            int texture=scene->mapF[mp]*32*32;

            int pixel=(((int)ty&31)*32+((int)tx&31))*3+texture*3;
            int red=All_Textures[pixel+0]*gameState.shader.r_fs;
            int green=All_Textures[pixel+1]*gameState.shader.g_fs;
            int blue=All_Textures[pixel+2]*gameState.shader.b_fs;
            
            glColor3ub(red,green,blue);glVertex2i(r*8,y);
            
            // draw roof
            texture=scene->mapC[mp]*32*32;
            pixel=(((int)ty&31)*32+((int)tx&31))*3+texture*3;
            red=All_Textures[pixel+0]*gameState.shader.r_cs;
            green=All_Textures[pixel+1]*gameState.shader.g_cs;
            blue=All_Textures[pixel+2]*gameState.shader.b_cs;
            
            if(texture>=0){glColor3ub(red,green,blue);glVertex2i(r*8,640-y);}
        
        }
        glEnd();
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
            int xo=(int)(gameState.player.ang*2-x); if(xo<0){xo+=120;} xo=xo%120;
            int pixel=(y*120+xo)*3;
            int red=Skybox[pixel+0]*gameState.shader.r_sb;
            int green=Skybox[pixel+1]*gameState.shader.g_sb;  
            int blue=Skybox[pixel+2]*gameState.shader.b_sb;
            
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
        int xo=0; if(gameState.player.dx<0) { xo=-25; } else { xo=25; }
        int yo=0; if(gameState.player.dy<0) { yo=-25; } else { yo=25; }

        int ipx=gameState.player.x/64.0, ipxa_xo=(gameState.player.x+xo)/64.0;
        int ipy=gameState.player.y/64.0, ipya_yo=(gameState.player.y+yo)/64.0;

        if(gameState.scene.mapW[ipya_yo*gameState.scene.width+ipxa_xo]==3) { gameState.scene.mapW[ipya_yo*gameState.scene.width+ipxa_xo]=0;}

        
        if(gameState.scene.mapD[ipya_yo*gameState.scene.width+ipxa_xo]>0) 
        { 
            gameState.currentScene=gameState.scene.mapD[ipya_yo*gameState.scene.width+ipxa_xo];

            updateScene();
        }
        
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
    
    int xo=0; if(gameState.player.dx<0) { xo=-20; } else { xo=20; }
    int yo=0; if(gameState.player.dy<0) { yo=-20; } else { yo=20; }
    int ipx=gameState.player.x/64.0, ipxa_xo=(gameState.player.x+xo)/64.0, ipxs_xo=(gameState.player.x-xo)/64.0;
    int ipy=gameState.player.y/64.0, ipya_yo=(gameState.player.y+yo)/64.0, ipys_yo=(gameState.player.y-yo)/64.0;
    
    if(Keys.a==1){ gameState.player.ang+=sensLR*fps; gameState.player.ang=FixAng(gameState.player.ang); gameState.player.dx=cos(degToRad(gameState.player.ang)); gameState.player.dy=-sin(degToRad(gameState.player.ang));} 	
    if(Keys.d==1){ gameState.player.ang-=sensLR*fps; gameState.player.ang=FixAng(gameState.player.ang); gameState.player.dx=cos(degToRad(gameState.player.ang)); gameState.player.dy=-sin(degToRad(gameState.player.ang));} 
    
    if(Keys.w==1)
    {  
        if(gameState.scene.mapW[ipy*gameState.scene.width+ipxa_xo]==0){ gameState.player.x+=gameState.player.dx*sensMV*fps;}
        if(gameState.scene.mapW[ipya_yo*gameState.scene.width+ipx]==0){ gameState.player.y+=gameState.player.dy*sensMV*fps;}
    }
    else if(Keys.s==1)
    { 
        if(gameState.scene.mapW[ipy*gameState.scene.width+ipxs_xo]==0){ gameState.player.x-=gameState.player.dx*sensMV*fps;}
        if(gameState.scene.mapW[ipys_yo*gameState.scene.width+ipx]==0){ gameState.player.y-=gameState.player.dy*sensMV*fps;}
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
