#include "loaders.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// loaders.c file later
EngineConfig
loadConfig(void)
{
  FILE* fp;
  char s[64];
  int linecount = 0;

  // fp=fopen("config.txt", "r");

  // while(fgets(s, sizeof s, fp)!=NULL)

  EngineConfig config = { .windowTitle = "Atlanta3D Engine V2",
                          .sensitivityLR = 0.2f,
                          .sensitivityMV = 0.2f };

  return config;
}

static int
readMap(FILE* fp, int* map, size_t count, int isLastMap)
{
  for (size_t i = 0; i < count; i++) {
    if (fscanf(fp, " %d", &map[i]) != 1) {
      return 0;
    } // Unable to read the number

    if (!(isLastMap && i == count - 1)) {
      int ch;
      do {
        ch = fgetc(fp);
      } while (ch != EOF && isspace((unsigned char)ch));

      if (ch != ',') {
        return 0;
      }
    }
  }
  return 1;
}

void
freeScene(Scene* scene)
{
  free(scene->mapW);
  free(scene->mapF);
  free(scene->mapC);
  free(scene->mapD);

  scene->mapW = NULL;
  scene->mapF = NULL;
  scene->mapC = NULL;
  scene->mapD = NULL;

  scene->width = 0;
  scene->height = 0;
}

Scene
loadScene(int sceneNumber)
{
  Scene scene = { 0 };
  char filename[64];

  snprintf(filename, sizeof filename, "scenes/scene%d.txt", sceneNumber);

  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    perror(filename);
    return scene;
  }

  if (fscanf(fp, "%d %d", &scene.width, &scene.height) != 2 ||
      scene.width <= 0 || scene.height <= 0) {
    // fprintf("errrors occured while loading scene %s(invalid
    // dimensions)\n",filename);
    fclose(fp);
    return scene;
  }

  if (fscanf(
        fp, "%f %f %f", &scene.spawn.x, &scene.spawn.y, &scene.spawn.ang) !=
      3) {
    fclose(fp);
    return scene;
  }

  // Calculating actual scene size and allocating memory
  size_t count = (size_t)scene.width * (size_t)scene.height;

  scene.mapW = malloc(count * sizeof *scene.mapW);
  scene.mapF = malloc(count * sizeof *scene.mapF);
  scene.mapC = malloc(count * sizeof *scene.mapC);
  scene.mapD = malloc(count * sizeof *scene.mapD);
  // add !scene.map* error handling later

  int ok =
    readMap(fp, scene.mapW, count, 0) && readMap(fp, scene.mapF, count, 0) &&
    readMap(fp, scene.mapC, count, 0) && readMap(fp, scene.mapD, count, 1);

  fclose(fp);

  if (!ok) {
    fprintf(stderr, "Error while reading scene file: %s", filename);
    memset(&scene, 0, sizeof scene);
  }

  return scene;
}

Shader
loadShader(int shaderNumber)
{
  Shader shader = { 0.0f, 0.0f, 0.0f, 0.0f };
  char filename[64];

  snprintf(filename, sizeof(filename), "shaders/shader%d.txt", shaderNumber);

  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    perror(filename);
    return shader;
  }

  int read_count =
    fscanf(fp,
           "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", // Maybe color struct
                                                           // lol?
           &shader.r_bright_ws,
           &shader.g_bright_ws,
           &shader.b_bright_ws,
           &shader.r_dark_ws,
           &shader.g_dark_ws,
           &shader.b_dark_ws,
           &shader.r_fs,
           &shader.g_fs,
           &shader.b_fs,
           &shader.r_cs,
           &shader.g_cs,
           &shader.b_cs,
           &shader.r_sb,
           &shader.g_sb,
           &shader.b_sb);

  if (read_count != 15) {
    fprintf(stderr, "error while reading from %s", filename);
    return shader;
  }
  fclose(fp);

  return shader;
}