#include "ai_agents.h"
#include "box.h"
#include "colours.h"
#include "consts.h"
#include "maths.h"
#include "path.h"
#include <stdlib.h>
#include <time.h>

path Path;
bool bDebug = true;
extern ai_agents AIAgents = { .Count = 1, .State = AI_STATE_SEEK };
extern boxes     Boxes    = { .Count = 0 };

void SetScene();


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game AI Programming");
    SetTargetFPS(120);
    srand(time(NULL));

	const int RandMin[] = {   64,  192 };
	const int RandMax[] = { 2496, 1280 };

    vector2 AIStartPosition = { SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1 };
    AIAgentInit(0, AIStartPosition, AZURE);
    for (unsigned i = 1; i < AI_AGENTS_ALLOC; i++) {
        AIStartPosition = (vector2){
            rand() % (RandMax[0] - RandMin[0]) + RandMin[0],
            rand() % (RandMax[1] - RandMin[1]) + RandMin[1]
        };
        AIAgentInit(i, AIStartPosition, (colour){ 0, (i & 0x0f) + 0x77, 0xff, 0xff });
    }
    PathInit(&Path);
    for (unsigned i = 0; i < BOXES_ALLOC; i++) {
        const vector2 BoxPosition = (vector2){
            rand() % (RandMax[0] - RandMin[0]) + RandMin[0],
            rand() % (RandMax[1] - RandMin[1]) + RandMin[1]
        };
        if (i & 1) {
            BoxInit(i, BoxPosition, (rand() & 255) + 32, (rand() & 31) + 32, (float)rand() / RAND_MAX * TWO_PI, AMBER);
        } else {
            BoxInit(i, BoxPosition, (rand() &  31) + 64, (rand() & 31) + 64, (float)rand() / RAND_MAX * TWO_PI, AMBER);
        }
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, VIOLENT_VIOLET, MAGENTA);

        // Controls
        if (IsKeyPressed(KEY_RIGHT)) {
            if (AIAgents.State < AI_STATE_COMBINED) {
                AIAgents.State++;
            }
            SetScene();
        } else if (IsKeyPressed(KEY_LEFT)) {
            if (AIAgents.State) {
                AIAgents.State--;
            }
            SetScene();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            PathInit(&Path);
            BoxesInit();
            for (unsigned i = 0; i < AIAgents.Count; i++) {
                AIAgents.PathNodeId[i] = PATH_INVALID_NODE;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            bDebug = !bDebug;
        }

        // Path
        if (AIAgents.State == AI_STATE_PATH || AIAgents.State == AI_STATE_COMBINED) {
            PathDraw(&Path);
        }

        // Obstacles / Walls
        BoxesDraw();

        // AI agents
        AIAgentsUpdate();
        AIAgentsDraw();

        // Slow radius
        const float VisualRadius = AI_SLOW_RADIUS - AIAgents.Radius[0];
        DrawRing(GetMousePosition(), VisualRadius, VisualRadius - 4.f, 0.f, 360.f, 64, GUPPIE_GREEN);

        // Text
        DrawText(GetAIStateString(), 64, 16, 64, AMBER);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}


void SetScene() {
    switch (AIAgents.State) {
    case AI_STATE_SEEK:   break;
    case AI_STATE_FLEE:   break;
    case AI_STATE_PURSUE: break;
    case AI_STATE_EVADE:  break;
    case AI_STATE_ARRIVE: break;
    case AI_STATE_WANDER: break;
    case AI_STATE_PATH:
        AIAgents.Count = 1;
        Boxes.Count    = 0;
        for (unsigned i = 0; i < AIAgents.Count; i++) {
            AIAgents.PathNodeId[i] = PATH_INVALID_NODE;
        }
        break;
    case AI_STATE_SEPARATION:
        AIAgents.Count = AI_AGENTS_ALLOC;
        Boxes.Count    = 0;
        break;
    case AI_STATE_COLLISION:
        AIAgents.Count = AI_AGENTS_ALLOC;
        Boxes.Count    = 0;
        break;
    case AI_STATE_OBSTACLE:
        AIAgents.Count = 1;
        Boxes.Count    = BOXES_ALLOC;
        break;
    case AI_STATE_COMBINED:
        AIAgents.Count = AI_AGENTS_ALLOC;
        Boxes.Count    = BOXES_ALLOC;
        break;
    }
}