#pragma once
#include "raylib.h"

typedef enum ai_states {
    AI_STATE_SEEK,
    AI_STATE_FLEE,
    AI_STATE_PURSUE,
    AI_STATE_EVADE,
    AI_STATE_ARRIVE,
    AI_STATE_WANDER,
    AI_STATES_COUNT
} ai_states;

typedef struct ai_agent {
    Vector2   Position, Velocity, Acceleration;
    float     Orientation, Rotation, Angular;
    float     Radius;
    ai_states State;
    Color     Colour;
} ai_agent;

void AIAgentUpdate(ai_agent *AIAgent);
void AIAgentDraw(ai_agent *AIAgent);
const char *GetAIStateString(ai_agent *AIAgent);
