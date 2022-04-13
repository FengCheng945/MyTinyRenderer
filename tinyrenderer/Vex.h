#ifndef __VEX_H__
#define __VEX_H__
#include "geometry.h"

struct Vex
{
    Vector3f screen_coords[3];
    Vector3f world_coords[3];
    Vector2f texture_coords[3];
    Vector3f normal_coords[3];
    float intensity[3];
};




#endif //__VEX_H__
