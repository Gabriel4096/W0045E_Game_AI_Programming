#pragma once
#include "raylib.h"

#define AI_AGENTS_ALLOC (16)
#define AI_SLOW_RADIUS (128.f)
#define AI_INVALID_ID ((unsigned)-1)

typedef Vector2 vector2;
typedef Color   colour;

typedef enum ai_states {
    AI_STATE_SEEK,
    AI_STATE_FLEE,
    AI_STATE_PURSUE,
    AI_STATE_EVADE,
    AI_STATE_ARRIVE,
    AI_STATE_WANDER,
    AI_STATE_PATH,
    AI_STATE_SEPARATION,
    AI_STATE_COLLISION,
    AI_STATES_COUNT
} ai_states;

typedef struct ai_agents {
    vector2   Position[AI_AGENTS_ALLOC], Velocity[AI_AGENTS_ALLOC], Acceleration[AI_AGENTS_ALLOC];
    Vector2   Target[AI_AGENTS_ALLOC];
    float     Orientation[AI_AGENTS_ALLOC], Rotation[AI_AGENTS_ALLOC], Angular[AI_AGENTS_ALLOC];
    float     Radius[AI_AGENTS_ALLOC];
    ai_states State;
    unsigned short Count;
    colour    Colour[AI_AGENTS_ALLOC];
    unsigned char PathNodeId[AI_AGENTS_ALLOC];
} ai_agents;

void AIAgentInit(unsigned Id, vector2 StartPosition, colour Colour);
void AIAgentsUpdate();
void AIAgentsDraw();
const char *GetAIStateString();
