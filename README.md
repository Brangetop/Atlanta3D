# Atlanta 3D - 2.5D(don't ask) raycaster engine. Written in C and OpenGL/FreeGLUT. 

## Features:
* Wall texturing
* Floor and ceiling texturing
* Custom textures via custom format(for now they need to be compiled)
* Skybox

## V2 Focus
In V1 main focus was to create a functional basic raycasted game. V2's main goal is to shift from a game with hardcoded values to an actual "engine" with real-time map loading, texture loading etc. while making tooling required to build games without diving into my custom texture and map formats.
It's important to understand that (for now) im not making a general purpose product and will keep some limitations such as:
* 960x640 resolution
* 32x32 textures
## Compilation
For linux compilation install all the libraries and run build.sh

For windows you'll have to fuck with MinGW to make it work with freeGLUT and GL, then run 
$ gcc main.c -o Atlanta3D.exe -lfreeglut -lopengl32 -lglu32 -mwindows

## Huge thanks for the help with this project:
* the 3D Sage Youtube channel for some amazing tutorials and textures
* Rockstar Games for the textures. Textures are property of **Rockstar Games** (extracted and pixelated from *GTA: San Andreas*). I do not claim ownership of these assets. 