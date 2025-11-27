#include "ai_agent.h"
#include "colours.h"
#include "consts.h"
#include "path.h"
#include <stdlib.h>
#include <time.h>

bool bDebug = true;
path Path;


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game AI Programming");
    SetTargetFPS(120);
    srand(time(NULL));

    const vector2 StartPosition = { SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1 };
    ai_agent AIAgent;
    AIAgentInit(&AIAgent, StartPosition);
    PathInit(&Path);

    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, VIOLENT_VIOLET, MAGENTA);

        // Controls
        if (IsKeyPressed(KEY_RIGHT)) {
            if (AIAgent.State < AI_STATE_PATH) {
                AIAgent.State++;
            }
        } else if (IsKeyPressed(KEY_LEFT)) {
            if (AIAgent.State) {
                AIAgent.State--;
            }
        }
        if (IsKeyPressed(KEY_SPACE)) {
            PathInit(&Path);
            AIAgent.PathNodeId = -1;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            bDebug = !bDebug;
        }

        // Path
        if (AIAgent.State == AI_STATE_PATH) {
            PathDraw(&Path);
        } else {
            AIAgent.PathNodeId = -1;
        }

        // AI agents
        AIAgentUpdate(&AIAgent);
        AIAgentDraw(&AIAgent);

        // Text
        DrawText("Change AI mode with the Right and Left key.", (SCREEN_WIDTH >> 1) - 16 * 44, 16, 64, AMBER);
        DrawText("Press Enter to toggle debug visualizations.", (SCREEN_WIDTH >> 1) - 16 * 44, 96, 64, AMBER);
        DrawText(GetAIStateString(&AIAgent), 64, SCREEN_HEIGHT - 96, 64, AMBER);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
