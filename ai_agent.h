#pragma once
#include "raylib.h"

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
    AI_STATES_COUNT
} ai_states;

typedef struct ai_agent {
    vector2   Position, Velocity, Acceleration;
    Vector2   Target;
    float     Orientation, Rotation, Angular;
    float     Radius;
    ai_states State;
    colour    Colour;
    unsigned char PathNodeId;
} ai_agent;

void AIAgentInit(ai_agent *AIAgent, const vector2 StartPosition);
void AIAgentUpdate(ai_agent *AIAgent);
void AIAgentDraw(ai_agent *AIAgent);
const char *GetAIStateString(ai_agent *AIAgent);
