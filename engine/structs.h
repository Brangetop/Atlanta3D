#ifndef STRUCTS_H
#define STRUCTS_H

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

#endif
