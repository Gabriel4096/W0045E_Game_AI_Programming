#pragma once
#include "raylib.h"
#include "raymath.h"

#define BOXES_ALLOC (4)

typedef Vector2 vector2;
typedef Color   colour;

typedef struct boxes {
    vector2  Points[BOXES_ALLOC][4];
    unsigned Count;
    colour   Colour[BOXES_ALLOC];
} boxes;

typedef struct ray_hit {
    vector2 Point, Normal;
    float   Distance;
    bool    bHit;
} ray_hit;

void BoxInit(unsigned Id, vector2 Position, float XRadius, float YRadius, float Orientation, colour Colour);
void BoxesInit();
ray_hit BoxRayIntersect(unsigned BoxId, vector2 RayStart, vector2 RayDelta);
void BoxesDraw();
