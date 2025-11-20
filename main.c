#include "ai_agent.h"
#include "colours.h"

#define SCREEN_WIDTH   (2560)
#define SCREEN_HEIGHT  (1440)

bool bDebug = true;

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game AI Programming");
    SetTargetFPS(120);

    ai_agent AIAgent = {
        { 1280.f, 720.f }, { 0.f, 0.f }, { 0.f, 0.f },
        0.f, 0.f, 0.f,
        48.f,
        AI_STATE_SEEK,
        AZURE
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, VIOLENT_VIOLET, MAGENTA);

        // Controls
        if (IsKeyPressed(KEY_RIGHT)) {
            if (AIAgent.State < AI_STATE_ARRIVE) {
                AIAgent.State++;
            }
        } else if (IsKeyPressed(KEY_LEFT)) {
            if (AIAgent.State) {
                AIAgent.State--;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            bDebug = !bDebug;
        }

        // AI agents
        AIAgentUpdate(&AIAgent);
        AIAgentDraw(&AIAgent);

        // Text
        DrawText("Change AI mode with the Right and Left key.", 1280 - 16 * 44, 16, 64, AMBER);
        DrawText("Press Enter to toggle debug visualizations.", 1280 - 16 * 44, 96, 64, AMBER);
        DrawText(GetAIStateString(&AIAgent), 64, 1440 - 2 * 64, 64, AMBER);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
