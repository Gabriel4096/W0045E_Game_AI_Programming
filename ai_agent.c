#include "ai_agent.h"

#include "colours.h"
#include "consts.h"
#include "maths.h"
#include "raylib.h"
#include "raymath.h"

#define MAX_ACCELERATION (512.f)
#define MAX_SPEED        (256.f)
#define SLOW_RADIUS      (128.f)
#define MAX_ANGULAR      ( 32.f)
#define MAX_PREDICTION   (  4.f)
#define MAX_ROTATION     (TWO_PI)

extern bool bDebug;

static void Seek(ai_agent *AIAgent) {
    AIAgent->Acceleration = Vector2Subtract(AIAgent->Target, AIAgent->Position);
    AIAgent->Acceleration = Vector2Normalize(AIAgent->Acceleration);
    AIAgent->Acceleration = Vector2Scale(AIAgent->Acceleration, MAX_ACCELERATION);
}

static void Flee(ai_agent *AIAgent) {
    AIAgent->Acceleration = Vector2Subtract(AIAgent->Position, AIAgent->Target);
    AIAgent->Acceleration = Vector2Normalize(AIAgent->Acceleration);
    AIAgent->Acceleration = Vector2Scale(AIAgent->Acceleration, MAX_ACCELERATION);
}

static void VelocityMatch(ai_agent *AIAgent, const vector2 TargetVelocity, const float Factor) {
    AIAgent->Acceleration = Vector2Scale(Vector2Subtract(TargetVelocity, AIAgent->Velocity), Factor);
    if (Vector2LengthSqr(AIAgent->Acceleration) > MAX_ACCELERATION * MAX_ACCELERATION) {
        AIAgent->Acceleration = Vector2Scale(Vector2Normalize(AIAgent->Acceleration), MAX_ACCELERATION);
    }
}

static void Arrive(ai_agent *AIAgent) {
    const vector2 Delta    = Vector2Subtract(AIAgent->Target, AIAgent->Position);
    const float   Distance = Vector2Length(Delta);
    if (Distance < 2.f) {
        AIAgent->Acceleration = AIAgent->Velocity = Vector2Zero();
        return;
    }
    const float   TargetSpeed    = (Distance > SLOW_RADIUS) ? MAX_SPEED : MAX_SPEED * Distance / SLOW_RADIUS;
    const vector2 TargetVelocity = Vector2Scale(Vector2Normalize(Delta), TargetSpeed);
    const float   AccFactor      = 8.f;
    VelocityMatch(AIAgent, TargetVelocity, AccFactor);
}

static void PreparePursueEvade(ai_agent *AIAgent) {
    const vector2 Direction   = Vector2Subtract(AIAgent->Target, AIAgent->Position);
    const float   Distance    = Vector2Length(Direction);
    const float   Speed       = Vector2Length(AIAgent->Velocity);
    const float   Prediction  = (Speed <= Distance / MAX_PREDICTION) ? MAX_PREDICTION : Distance / Speed;
    const vector2 NewTarget   = Vector2Add(AIAgent->Target, Vector2Scale(GetMouseDelta(), Prediction / GetFrameTime()));
    const vector2 ToOldTarget = Vector2Subtract(AIAgent->Target, AIAgent->Position);
    const vector2 ToNewTarget = Vector2Subtract(NewTarget, AIAgent->Position);

    // Avoids: fending off the agent when pursuing, the agent giving up when evading.
    if (Vector2DotProduct(Vector2Normalize(ToOldTarget), Vector2Normalize(ToNewTarget)) >= -HALF_ROOT_2) {
        AIAgent->Target = NewTarget;
        AIAgent->Colour = AZURE;
    } else if (bDebug) {
        //AIAgent->Target = NewTarget;
        AIAgent->Colour = AMBER;
    }
}

static void Align(ai_agent *AIAgent, const float Target) {
    const float Rotation     = MapToRangeRad(Target - AIAgent->Orientation);
    const float RotationSize = fabsf(Rotation);
    if (RotationSize < EPSILON) {
        AIAgent->Angular = AIAgent->Rotation = 0.f;
        return;
    }
    float TargetRotation;
    TargetRotation = (RotationSize > HALF_PI) ? MAX_ROTATION : MAX_ROTATION * RotationSize / HALF_PI;
    TargetRotation = copysignf(TargetRotation, Rotation);

    const float AngFactor = 16.f;
    AIAgent->Angular = (TargetRotation - AIAgent->Rotation) * AngFactor;
    if (fabsf(AIAgent->Angular) > MAX_ANGULAR) {
        AIAgent->Angular = copysignf(MAX_ANGULAR, AIAgent->Angular);
    }
}

static void Face(ai_agent *AIAgent) {
    const vector2 Direction = Vector2Subtract(AIAgent->Target, AIAgent->Position);
    if (Vector2LengthSqr(Direction) < EPSILON) {
        return;
    }
    Align(AIAgent, Vector2ToRad(Direction));
}

static void LookVelocity(ai_agent *AIAgent) {
    if (Vector2LengthSqr(AIAgent->Velocity) < EPSILON) {
        AIAgent->Angular = AIAgent->Rotation = 0.f;
        return;
    }
    Align(AIAgent, Vector2ToRad(AIAgent->Velocity));
}

static void Wander(ai_agent *AIAgent) {
    const float WanderOffset = 2.f * SLOW_RADIUS;
    const float WanderRadius = SLOW_RADIUS;
    const float WanderRate   = 16.f;

    vector2 WanderCircle;
    WanderCircle = RadToVector2(AIAgent->Orientation);
    WanderCircle = Vector2Scale(WanderCircle, WanderOffset);
    WanderCircle = Vector2Add(AIAgent->Position, WanderCircle);
    float TargetRad;
    TargetRad  = Vector2ToRad(Vector2Subtract(AIAgent->Target, WanderCircle));
    TargetRad += RandomBinomal() * WanderRate * GetFrameTime();
    AIAgent->Target = Vector2Add(AIAgent->Position, Vector2Scale(RadToVector2(AIAgent->Orientation), WanderOffset));
    AIAgent->Target = Vector2Add(AIAgent->Target, Vector2Scale(RadToVector2(TargetRad), WanderRadius));
    Face(AIAgent);

    AIAgent->Acceleration = Vector2Scale(RadToVector2(AIAgent->Orientation), MAX_ACCELERATION);

    if (bDebug) {
        DrawLineEx(AIAgent->Position, WanderCircle, 2, GUPPIE_GREEN);
        DrawRing(WanderCircle, WanderRadius + 1.f, WanderRadius - 1.f, 0.f, 360.f, 64, GUPPIE_GREEN);
    }
}


void AIAgentInit(ai_agent *AIAgent, const vector2 StartPosition) {
    AIAgent->Position     = StartPosition;
    AIAgent->Velocity     = Vector2Zero();
    AIAgent->Acceleration = Vector2Zero();
    AIAgent->Target       = (vector2){ SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1 };
    AIAgent->Orientation  = 0.f;
    AIAgent->Rotation     = 0.f;
    AIAgent->Angular      = 0.f;
    AIAgent->Radius       = 48.f;
    AIAgent->State        = AI_STATE_SEEK;
    AIAgent->Colour       = AZURE;
}

void AIAgentUpdate(ai_agent *AIAgent) {
	switch (AIAgent->State) {
    case AI_STATE_SEEK:
        AIAgent->Target = GetMousePosition();
        Seek(AIAgent);
        Face(AIAgent);
        break;
    case AI_STATE_FLEE:
        AIAgent->Target = GetMousePosition();
        Flee(AIAgent);
        LookVelocity(AIAgent);
        break;
    case AI_STATE_PURSUE:
        AIAgent->Target = GetMousePosition();
        PreparePursueEvade(AIAgent);
        Seek(AIAgent);
        Face(AIAgent);
        break;
    case AI_STATE_EVADE:
        AIAgent->Target = GetMousePosition();
        PreparePursueEvade(AIAgent);
        Flee(AIAgent);
        LookVelocity(AIAgent);
        break;
    case AI_STATE_ARRIVE:
        AIAgent->Target = GetMousePosition();
        Arrive(AIAgent);
        Face(AIAgent);
        break;
    case AI_STATE_WANDER:
        Wander(AIAgent);
        Face(AIAgent);
        break;
    case AI_STATE_PATH:
        break;
	default:
        return;
	}

    const float DeltaTime = GetFrameTime();

    // Update 0th derivatives
    AIAgent->Position.x  += DeltaTime * AIAgent->Velocity.x;
    AIAgent->Position.y  += DeltaTime * AIAgent->Velocity.y;
    AIAgent->Orientation += DeltaTime * AIAgent->Rotation;

    // Update 1st derivatives
    AIAgent->Velocity.x += DeltaTime * AIAgent->Acceleration.x;
    AIAgent->Velocity.y += DeltaTime * AIAgent->Acceleration.y;
    AIAgent->Rotation   += DeltaTime * AIAgent->Angular;

    // Speed cap
    if (Vector2LengthSqr(AIAgent->Velocity) > MAX_SPEED * MAX_SPEED) {
        AIAgent->Velocity = Vector2Scale(Vector2Normalize(AIAgent->Velocity), MAX_SPEED);
    }
}

void AIAgentDraw(ai_agent *AIAgent) {
    const vector2 Forward        = RadToVector2(AIAgent->Orientation);
    const vector2 Right          = { -Forward.y, Forward.x };
    const float   DiagonalRadius = HALF_ROOT_2 * AIAgent->Radius;
    const vector2 FrontPoint     = Vector2Scale(Forward, AIAgent->Radius);
    const vector2 LeftPoint      = Vector2Add(Vector2Scale(Forward, -DiagonalRadius), Vector2Scale(Right, -DiagonalRadius));
    const vector2 RightPoint     = Vector2Add(Vector2Scale(Forward, -DiagonalRadius), Vector2Scale(Right,  DiagonalRadius));
    DrawTriangle(Vector2Add(AIAgent->Position, FrontPoint),
                 Vector2Add(AIAgent->Position, LeftPoint),
                 Vector2Add(AIAgent->Position, RightPoint),
                 AIAgent->Colour);

    if (bDebug) {
        //DrawLineEx(AIAgent->Position, Vector2Add(AIAgent->Position, Vector2Scale(Forward, 2.f * AIAgent->Radius)), 4, GUPPIE_GREEN);
        //DrawLineEx(AIAgent->Position, Vector2Add(AIAgent->Position, Vector2Scale(Right, 2.f * AIAgent->Radius)), 4, AZURE);
        //DrawLineEx(AIAgent->Position, Vector2Add(AIAgent->Position, AIAgent->Acceleration), 2, GUPPIE_GREEN);

        // TargetPosition
        DrawCircleV(AIAgent->Target, 16.f, MY_RED);

        // Slow radius
        const float VisualRadius = SLOW_RADIUS - AIAgent->Radius;
        DrawRing(GetMousePosition(), VisualRadius, VisualRadius - 4.f, 0.f, 360.f, 64, GUPPIE_GREEN);
    }
}

const char *GetAIStateString(ai_agent *AIAgent) {
    switch (AIAgent->State) {
    case AI_STATE_SEEK:   return "AI state: Seek";
    case AI_STATE_FLEE:   return "AI state: Flee";
    case AI_STATE_PURSUE: return "AI state: Pursue";
    case AI_STATE_EVADE:  return "AI state: Evade";
    case AI_STATE_ARRIVE: return "AI state: Arrive";
    case AI_STATE_WANDER: return "AI state: Wander";
    case AI_STATE_PATH:   return "AI state: Path";
    default: return "INVALID STATE!";
    }
}
