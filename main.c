#include "ai_agents.h"
#include "colours.h"
#include "consts.h"
#include "path.h"
#include <stdlib.h>
#include <time.h>

path Path;
extern ai_agents AIAgents = { .Count = 1, .State = AI_STATE_SEEK };
bool bDebug = true;


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game AI Programming");
    SetTargetFPS(120);
    srand(time(NULL));

    vector2 AIStartPosition = { SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1 };
    AIAgentInit(0, AIStartPosition, AZURE);
	const int RandMin[] = {   64,  192 };
	const int RandMax[] = { 2496, 1280 };
    for (unsigned i = 1; i < AI_AGENTS_ALLOC; i++) {
        AIStartPosition = (vector2){
            rand() % (RandMax[0] - RandMin[0]) + RandMin[0],
            rand() % (RandMax[1] - RandMin[1]) + RandMin[1]
        };
        AIAgentInit(i, AIStartPosition, (colour){ 0, (i & 0x0f) + 0x77, 0xff, 0xff });
    }
    PathInit(&Path);

    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, VIOLENT_VIOLET, MAGENTA);

        // Controls
        if (IsKeyPressed(KEY_RIGHT)) {
            if (AIAgents.State < AI_STATES_COUNT) {
                AIAgents.State++;
            }
        } else if (IsKeyPressed(KEY_LEFT)) {
            if (AIAgents.State) {
                AIAgents.State--;
            }
        }
        if (IsKeyPressed(KEY_SPACE)) {
            PathInit(&Path);
            for (unsigned i = 0; i < AIAgents.Count; i++) {
                AIAgents.PathNodeId[i] = -1;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            bDebug = !bDebug;
        }

        // Path
        if (AIAgents.State == AI_STATE_PATH) {
            PathDraw(&Path);
            AIAgents.Count = AI_AGENTS_ALLOC;
        } else {
            for (unsigned i = 0; i < AIAgents.Count; i++) {
                AIAgents.PathNodeId[i] = -1;
            }
            //AIAgents.Count = 1;
        }

        if (AIAgents.State == AI_STATE_SEPARATION) {
            AIAgents.Count = 2;
        }

        // AI agents
        AIAgentsUpdate();
        AIAgentsDraw();

        // Slow radius
        const float VisualRadius = AI_SLOW_RADIUS - AIAgents.Radius[0];
        DrawRing(GetMousePosition(), VisualRadius, VisualRadius - 4.f, 0.f, 360.f, 64, GUPPIE_GREEN);


        // Text
        DrawText("Change AI mode with the Right and Left key.", (SCREEN_WIDTH >> 1) - 16 * 44, 16, 64, AMBER);
        DrawText("Press Enter to toggle debug visualizations.", (SCREEN_WIDTH >> 1) - 16 * 44, 96, 64, AMBER);
        DrawText(GetAIStateString(), 64, SCREEN_HEIGHT - 96, 64, AMBER);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
