#include "ai_agent.h"
#include "raymath.h"
#include "colours.h"
#include "maths.h"

#define MAX_ACCELERATION  (512.f)
#define MAX_SPEED         (256.f)
#define SLOW_RADIUS       (128.f)
#define MAX_ANGULAR       ( 32.f)
#define MAX_PREDICTION    (  4.f)
#define MAX_ROTATION      (TWO_PI)

extern bool bDebug;

Vector2 TargetPosition;

static void Seek(ai_agent *AIAgent, const Vector2 Target) {
    AIAgent->Acceleration = Vector2Subtract(Target, AIAgent->Position);
    AIAgent->Acceleration = Vector2Normalize(AIAgent->Acceleration);
    AIAgent->Acceleration = Vector2Scale(AIAgent->Acceleration, MAX_ACCELERATION);
}

static void Flee(ai_agent *AIAgent, const Vector2 Target) {
    AIAgent->Acceleration = Vector2Subtract(AIAgent->Position, Target);
    AIAgent->Acceleration = Vector2Normalize(AIAgent->Acceleration);
    AIAgent->Acceleration = Vector2Scale(AIAgent->Acceleration, MAX_ACCELERATION);
}

static void VelocityMatch(ai_agent *AIAgent, const Vector2 TargetVelocity, const float Factor) {
    AIAgent->Acceleration = Vector2Scale(Vector2Subtract(TargetVelocity, AIAgent->Velocity), Factor);
    if (Vector2LengthSqr(AIAgent->Acceleration) > MAX_ACCELERATION * MAX_ACCELERATION) {
        AIAgent->Acceleration = Vector2Scale(Vector2Normalize(AIAgent->Acceleration), MAX_ACCELERATION);
    }
}

static void Arrive(ai_agent *AIAgent, const Vector2 Target) {
    const Vector2 Delta    = Vector2Subtract(Target, AIAgent->Position);
    const float   Distance = Vector2Length(Delta);
    if (Distance < 2.f) {
        AIAgent->Acceleration = AIAgent->Velocity = Vector2Zero();
        return;
    }
    const float   TargetSpeed    = (Distance > SLOW_RADIUS) ? MAX_SPEED : MAX_SPEED * Distance / SLOW_RADIUS;
    const Vector2 TargetVelocity = Vector2Scale(Vector2Normalize(Delta), TargetSpeed);
    const float   AccFactor      = 8.f;
    VelocityMatch(AIAgent, TargetVelocity, AccFactor);
}

static void Align(ai_agent *AIAgent, const Vector2 Target) {
    const float Rotation = MapToRangeRad(atan2f(-Target.x, -Target.y) - AIAgent->Orientation);
    float RotationSize = fabsf(Rotation);
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

static void PreparePursueEvade(ai_agent *AIAgent, Vector2 *Target) {
    const Vector2 Direction   = Vector2Subtract(*Target, AIAgent->Position);
    const float   Distance    = Vector2Length(Direction);
    const float   Speed       = Vector2Length(AIAgent->Velocity);
    const float   Prediction  = (Speed <= Distance / MAX_PREDICTION) ? MAX_PREDICTION : Distance / Speed;
    const Vector2 NewTarget   = Vector2Add(*Target, Vector2Scale(GetMouseDelta(), Prediction / GetFrameTime()));
    const Vector2 ToOldTarget = Vector2Subtract(*Target, AIAgent->Position);
    const Vector2 ToNewTarget = Vector2Subtract(NewTarget, AIAgent->Position);

    // Avoids: fending off the agent when pursuing, the agent giving up when evading.
    if (Vector2DotProduct(Vector2Normalize(ToOldTarget), Vector2Normalize(ToNewTarget)) >= -HALF_ROOT_2) {
        *Target = NewTarget;
        AIAgent->Colour = AZURE;
    } else if (bDebug) {
        //*Target = NewTarget;
        AIAgent->Colour = AMBER;
    }
}


void AIAgentUpdate(ai_agent *AIAgent) {
    const float DeltaTime = GetFrameTime();
    Vector2 TargetOrientation;
    TargetPosition = GetMousePosition();

	switch (AIAgent->State) {
    case AI_STATE_SEEK:
        Seek(AIAgent, TargetPosition);
        TargetOrientation = Vector2Subtract(TargetPosition, AIAgent->Position);
        break;
    case AI_STATE_FLEE:
        Flee(AIAgent, TargetPosition);
        TargetOrientation = Vector2Subtract(AIAgent->Position, TargetPosition);
        break;
    case AI_STATE_PURSUE:
        PreparePursueEvade(AIAgent, &TargetPosition);
        Seek(AIAgent, TargetPosition);
        TargetOrientation = AIAgent->Velocity;
        break;
    case AI_STATE_EVADE:
        PreparePursueEvade(AIAgent, &TargetPosition);
        Flee(AIAgent, TargetPosition);
        TargetOrientation = AIAgent->Velocity;
        break;
    case AI_STATE_ARRIVE:
        Arrive(AIAgent, TargetPosition);
        TargetOrientation = Vector2Subtract(TargetPosition, AIAgent->Position);
        break;
    case AI_STATE_WANDER:
        break;
	default:
		break;
	}
    Align(AIAgent, TargetOrientation);

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
    const Vector2 Forward        = { -sinf(AIAgent->Orientation), -cosf(AIAgent->Orientation) };
    const Vector2 Right          = { -Forward.y, Forward.x };
    const float   DiagonalRadius = HALF_ROOT_2 * AIAgent->Radius;
    const Vector2 FrontPoint     = Vector2Scale(Forward, AIAgent->Radius);
    const Vector2 LeftPoint      = Vector2Add(Vector2Scale(Forward, -DiagonalRadius), Vector2Scale(Right, -DiagonalRadius));
    const Vector2 RightPoint     = Vector2Add(Vector2Scale(Forward, -DiagonalRadius), Vector2Scale(Right,  DiagonalRadius));
    DrawTriangle(Vector2Add(AIAgent->Position, FrontPoint),
                 Vector2Add(AIAgent->Position, LeftPoint),
                 Vector2Add(AIAgent->Position, RightPoint),
                 AIAgent->Colour);

    if (bDebug) {
        //DrawLineEx(AIAgent->Position, Vector2Add(AIAgent->Position, Vector2Scale(Forward, 2.f * AIAgent->Radius)), 4, GUPPIE_GREEN);
        //DrawLineEx(AIAgent->Position, Vector2Add(AIAgent->Position, Vector2Scale(Right, 2.f * AIAgent->Radius)), 4, AZURE);
        //DrawLineEx(AIAgent->Position, Vector2Add(AIAgent->Position, AIAgent->Acceleration), 2, GUPPIE_GREEN);

        const float VisualRadius = SLOW_RADIUS - AIAgent->Radius;
        DrawRing(GetMousePosition(), VisualRadius, VisualRadius - 4.f, 0.f, 360.f, 64, GUPPIE_GREEN);

        // TargetPosition
        DrawCircleV(TargetPosition, 16.f, MY_RED);
    }
}

const char *GetAIStateString(ai_agent *AIAgent) {
    switch (AIAgent->State) {
    case AI_STATE_SEEK:
        return "AI state: Seek";
    case AI_STATE_FLEE:
        return "AI state: Flee";
    case AI_STATE_PURSUE:
        return "AI state: Pursue";
    case AI_STATE_EVADE:
        return "AI state: Evade";
    case AI_STATE_ARRIVE:
        return "AI state: Arrive";
    case AI_STATE_WANDER:
        return "AI state: Wander";
    default:
        return "INVALID STATE!";
    }
}