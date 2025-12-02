#pragma once
#include "raylib.h"

#define PATH_NODE_COUNT   (8)  // Max 2 digits
#define PATH_INVALID_NODE ((unsigned char)-1)

typedef Vector2 vector2;

typedef struct path {
    vector2 Nodes[PATH_NODE_COUNT];
    vector2 Directions[PATH_NODE_COUNT];
    float   Lengths[PATH_NODE_COUNT];
} path;

void PathInit(path *Path);
vector2 PathGetTarget(const path *Path, unsigned AIId);
void PathDraw(const path *Path);
