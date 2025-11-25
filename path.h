#pragma once
#include "raylib.h"

#define PATH_NODE_COUNT (8)  // Only 1 digit

typedef Vector2 vector2;

typedef struct path {
    vector2 Nodes[PATH_NODE_COUNT];
    //vector2 Directions[PATH_NODE_COUNT];
    //float   Lengths[PATH_NODE_COUNT];
} path;

void PathInit(path *Path);
vector2 PathFindClosestPoint(const path *Path, const vector2 Point);
void PathDraw(path *Path);
