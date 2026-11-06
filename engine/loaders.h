#ifndef LOADERS_H
#define LOADERS_H

#include <stdio.h>
#include "structs.h"

EngineConfig loadConfig(void);
static int readMap(FILE *fp, int *map, size_t count, int isLastMap);
void freeScene(Scene *scene);
Scene loadScene(int sceneNumber);
Shader loadShader(int shaderNumber);

#endif