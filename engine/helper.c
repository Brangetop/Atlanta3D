#include "helper.h"
#include <math.h>

// Helper functions
float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a)
{
    if(a>359){ a-=360;}
    if(a<0){ a+=360;}
    
    return a;
}