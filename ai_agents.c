#include "ai_agents.h"

#include "colours.h"
#include "consts.h"
#include <float.h>
#include "maths.h"
#include "path.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define MAX_ACCELERATION (512.f)
#define MAX_SPEED        (256.f)
#define MAX_ANGULAR       (32.f)
#define MAX_PREDICTION     (4.f)
#define MAX_ROTATION    (TWO_PI)

ai_agents AIAgents;
extern path Path;
extern bool bDebug;

static void Seek(unsigned Id) {
    AIAgents.Acceleration[Id] = Vector2Subtract(AIAgents.Target[Id], AIAgents.Position[Id]);
    AIAgents.Acceleration[Id] = Vector2Normalize(AIAgents.Acceleration[Id]);
    AIAgents.Acceleration[Id] = Vector2Scale(AIAgents.Acceleration[Id], MAX_ACCELERATION);
}

static void Flee(unsigned Id) {
    AIAgents.Acceleration[Id] = Vector2Subtract(AIAgents.Position[Id], AIAgents.Target[Id]);
    AIAgents.Acceleration[Id] = Vector2Normalize(AIAgents.Acceleration[Id]);
    AIAgents.Acceleration[Id] = Vector2Scale(AIAgents.Acceleration[Id], MAX_ACCELERATION);
}

static void VelocityMatch(unsigned Id, const vector2 TargetVelocity, const float Factor) {
    AIAgents.Acceleration[Id] = Vector2Scale(Vector2Subtract(TargetVelocity, AIAgents.Velocity[Id]), Factor);
    if (Vector2LengthSqr(AIAgents.Acceleration[Id]) > MAX_ACCELERATION * MAX_ACCELERATION) {
        AIAgents.Acceleration[Id] = Vector2Scale(Vector2Normalize(AIAgents.Acceleration[Id]), MAX_ACCELERATION);
    }
}

static void Arrive(unsigned Id) {
    const vector2 Delta    = Vector2Subtract(AIAgents.Target[Id], AIAgents.Position[Id]);
    const float   Distance = Vector2Length(Delta);
    if (Distance < 2.f) {
        AIAgents.Acceleration[Id] = AIAgents.Velocity[Id] = Vector2Zero();
        return;
    }
    const float   TargetSpeed    = (Distance > AI_SLOW_RADIUS) ? MAX_SPEED : MAX_SPEED * Distance / AI_SLOW_RADIUS;
    const vector2 TargetVelocity = Vector2Scale(Vector2Normalize(Delta), TargetSpeed);
    const float   AccFactor      = 8.f;
    VelocityMatch(Id, TargetVelocity, AccFactor);
}

static void PreparePursueEvade(unsigned Id) {
    const vector2 Direction   = Vector2Subtract(AIAgents.Target[Id], AIAgents.Position[Id]);
    const float   Distance    = Vector2Length(Direction);
    const float   Speed       = Vector2Length(AIAgents.Velocity[Id]);
    const float   Prediction  = (Speed <= Distance / MAX_PREDICTION) ? MAX_PREDICTION : Distance / Speed;
    const vector2 NewTarget   = Vector2Add(AIAgents.Target[Id], Vector2Scale(GetMouseDelta(), Prediction / GetFrameTime()));
    const vector2 ToOldTarget = Vector2Subtract(AIAgents.Target[Id], AIAgents.Position[Id]);
    const vector2 ToNewTarget = Vector2Subtract(NewTarget, AIAgents.Position[Id]);

    // Avoids: fending off the agent when pursuing, the agent giving up when evading.
    if (Vector2DotProduct(Vector2Normalize(ToOldTarget), Vector2Normalize(ToNewTarget)) >= -HALF_ROOT_2) {
        AIAgents.Target[Id] = NewTarget;
    }
}

static void Align(unsigned Id, const float Target) {
    const float Rotation     = MapToRangeRad(Target - AIAgents.Orientation[Id]);
    const float RotationSize = fabsf(Rotation);
    if (RotationSize < FLT_EPSILON) {
        AIAgents.Angular[Id] = AIAgents.Rotation[Id] = 0.f;
        return;
    }
    float TargetRotation;
    TargetRotation = (RotationSize > HALF_PI) ? MAX_ROTATION : MAX_ROTATION * RotationSize / HALF_PI;
    TargetRotation = copysignf(TargetRotation, Rotation);

    const float AngFactor = 16.f;
    AIAgents.Angular[Id] = (TargetRotation - AIAgents.Rotation[Id]) * AngFactor;
    if (fabsf(AIAgents.Angular[Id]) > MAX_ANGULAR) {
        AIAgents.Angular[Id] = copysignf(MAX_ANGULAR, AIAgents.Angular[Id]);
    }
}

static void Face(unsigned Id) {
    const vector2 Direction = Vector2Subtract(AIAgents.Target[Id], AIAgents.Position[Id]);
    if (Vector2LengthSqr(Direction) < FLT_EPSILON) {
        return;
    }
    Align(Id, Vector2ToRad(Direction));
}

static void LookVelocity(unsigned Id) {
    if (Vector2LengthSqr(AIAgents.Velocity[Id]) < FLT_EPSILON) {
        AIAgents.Angular[Id] = AIAgents.Rotation[Id] = 0.f;
        return;
    }
    Align(Id, Vector2ToRad(AIAgents.Velocity[Id]));
}

static void Wander(unsigned Id) {
    const float WanderOffset = 2.f * AI_SLOW_RADIUS;
    const float WanderRadius = AI_SLOW_RADIUS;
    const float WanderRate   = 16.f;

    vector2 WanderCircle;
    WanderCircle = RadToVector2(AIAgents.Orientation[Id]);
    WanderCircle = Vector2Scale(WanderCircle, WanderOffset);
    WanderCircle = Vector2Add(AIAgents.Position[Id], WanderCircle);
    float TargetRad;
    TargetRad  = Vector2ToRad(Vector2Subtract(AIAgents.Target[Id], WanderCircle));
    TargetRad += RandomBinomal() * WanderRate * GetFrameTime();
    AIAgents.Target[Id] = Vector2Add(AIAgents.Position[Id], Vector2Scale(RadToVector2(AIAgents.Orientation[Id]), WanderOffset));
    AIAgents.Target[Id] = Vector2Add(AIAgents.Target[Id], Vector2Scale(RadToVector2(TargetRad), WanderRadius));
    Face(Id);

    AIAgents.Acceleration[Id] = Vector2Scale(RadToVector2(AIAgents.Orientation[Id]), MAX_ACCELERATION);

    if (bDebug) {
        DrawLineEx(AIAgents.Position[Id], WanderCircle, 2, GUPPIE_GREEN);
        DrawRing(WanderCircle, WanderRadius + 1.f, WanderRadius - 1.f, 0.f, 360.f, 64, GUPPIE_GREEN);
    }
}

static void Separation(unsigned Id) {
    const float Thresh = 4.f * AI_SLOW_RADIUS * AI_SLOW_RADIUS;
    const float Decay  = Thresh * Thresh;
    for (unsigned i = 0; i < AIAgents.Count; i++) {
        if (i == Id) {
            continue;
        }
        vector2 Direction = Vector2Subtract(AIAgents.Position[Id], AIAgents.Position[i]);

        const float DistSqr = Vector2LengthSqr(Direction);
        if (DistSqr < Thresh && DistSqr >= FLT_EPSILON) {
            float Strength;
            Strength  = Decay / DistSqr;
            Strength  = min(Strength, MAX_ACCELERATION);
            Direction = Vector2Scale(Direction, Strength / sqrtf(DistSqr));
            AIAgents.Acceleration[Id] = Vector2Add(AIAgents.Acceleration[Id], Direction);
        }
    }
}

static void Collision(unsigned Id) {
    float    ShortestTime = FLT_MAX;
    unsigned FirstTarget  = AI_INVALID_ID;
    float   FirstMinSeparation;
    float   FirstDistance;
    vector2 FirstRelPos;
    vector2 FirstRelVel;
    for (unsigned i = 0; i < AIAgents.Count; i++) {
        if (i == Id) {
            continue;
        }
        if (Vector2Distance(AIAgents.Position[i], AIAgents.Position[Id]) < AIAgents.Radius[i] + AIAgents.Radius[Id]) {
            DrawRing(AIAgents.Position[Id], AIAgents.Radius[Id], AIAgents.Radius[Id] - 2.f, 0.f, 360.f, 32, AMBER);
            //AIAgents.Velocity[Id] = Vector2Subtract(AIAgents.Velocity[Id], FirstRelPos);
        }
        const vector2 RelativePos     = Vector2Subtract(AIAgents.Position[i], AIAgents.Position[Id]);
        const vector2 RelativeVel     = Vector2Subtract(AIAgents.Velocity[i], AIAgents.Velocity[Id]);
        const float   RelSpeedSqr     = Vector2LengthSqr(RelativeVel);
        const float   TimeToCollision = Vector2DotProduct(RelativePos, RelativeVel) / RelSpeedSqr;
        const float   Distance        = Vector2Length(RelativePos);
        const float   MinSeparation   = Distance - sqrtf(RelSpeedSqr) * TimeToCollision;
        const float   Diameter        = AIAgents.Radius[i] + AIAgents.Radius[Id];
        if (MinSeparation > Diameter) {
            continue;
        }
        if (TimeToCollision > 0.f && TimeToCollision < ShortestTime) {
            ShortestTime       = TimeToCollision;
            FirstTarget        = i;
            FirstMinSeparation = MinSeparation;
            FirstDistance      = Distance;
            FirstRelPos        = RelativePos;
            FirstRelVel        = RelativeVel;
        }
    }
    if (FirstTarget == AI_INVALID_ID) {
        return;
    }
    //const vector2 RelativePos = (FirstMinSeparation <= 0.f || FirstDistance < AIAgents.Radius[FirstTarget] + AIAgents.Radius[Id]) ?
    //      Vector2Subtract(AIAgents.Position[Id], AIAgents.Position[FirstTarget])
    //    : Vector2Add(FirstRelPos, Vector2Scale(FirstRelVel, ShortestTime));
    //if (Vector2Distance(AIAgents.Position[FirstTarget], AIAgents.Position[Id]) < AIAgents.Radius[FirstTarget] + AIAgents.Radius[Id]) {
    //    DrawRing(AIAgents.Position[Id], AIAgents.Radius[Id], AIAgents.Radius[Id] - 2.f, 0.f, 360.f, 32, AMBER);
    //    //AIAgents.Velocity[Id] = Vector2Subtract(AIAgents.Velocity[Id], FirstRelPos);
    //}
    vector2 RelativePos;
    if (FirstMinSeparation <= 0.f || FirstDistance < AIAgents.Radius[FirstTarget] + AIAgents.Radius[Id]) {
        RelativePos = Vector2Subtract(AIAgents.Position[Id], AIAgents.Position[FirstTarget]);
        DrawLineV(AIAgents.Position[Id], Vector2Add(AIAgents.Position[Id], RelativePos), MY_RED);
    } else {
        RelativePos = Vector2Add(FirstRelPos, Vector2Scale(FirstRelVel, ShortestTime));
        DrawLineV(AIAgents.Position[Id], Vector2Add(AIAgents.Position[Id], RelativePos), GUPPIE_GREEN);
    }
    AIAgents.Acceleration[Id] = Vector2Scale(Vector2Normalize(RelativePos), MAX_ACCELERATION);
    AIAgents.Angular[Id]      = 0.f;
    if (bDebug) {
        //DrawLineV(AIAgents.Position[Id], Vector2Add(AIAgents.Position[Id], AIAgents.Acceleration[Id]), AMBER);
    }
}


void AIAgentInit(unsigned Id, vector2 StartPosition, colour Colour) {
    AIAgents.Position[Id]     = StartPosition;
    AIAgents.Velocity[Id]     = Vector2Zero();
    AIAgents.Acceleration[Id] = Vector2Zero();
    AIAgents.Target[Id]       = (vector2){ SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1 };
    AIAgents.Orientation[Id]  =  0.f;
    AIAgents.Rotation[Id]     =  0.f;
    AIAgents.Angular[Id]      =  0.f;
    AIAgents.Radius[Id]       = 48.f;
    AIAgents.Colour[Id]       = Colour;
    AIAgents.PathNodeId[Id]   = PATH_INVALID_NODE;
}

void AIAgentsUpdate() {
    for (unsigned i = 0; i < AIAgents.Count; i++) {
        switch (AIAgents.State) {
        case AI_STATE_SEEK:
            AIAgents.Target[i] = GetMousePosition();
            Seek(i);
            Face(i);
            break;
        case AI_STATE_FLEE:
            AIAgents.Target[i] = GetMousePosition();
            Flee(i);
            LookVelocity(i);
            break;
        case AI_STATE_PURSUE:
            AIAgents.Target[i] = GetMousePosition();
            PreparePursueEvade(i);
            Seek(i);
            Face(i);
            break;
        case AI_STATE_EVADE:
            AIAgents.Target[i] = GetMousePosition();
            PreparePursueEvade(i);
            Flee(i);
            LookVelocity(i);
            break;
        case AI_STATE_ARRIVE:
            AIAgents.Target[i] = GetMousePosition();
            Arrive(i);
            Face(i);
            break;
        case AI_STATE_WANDER:
            Wander(i);
            Face(i);
            break;
        case AI_STATE_PATH:
            AIAgents.Target[i] = PathGetTarget(&Path, i);
            const vector2 Delta = Vector2Subtract(AIAgents.Target[i], AIAgents.Position[i]);
            const vector2 TargetVelocity = Vector2Scale(Vector2Normalize(Delta), MAX_SPEED);
            const float   AccFactor = 8.f;
            VelocityMatch(i, TargetVelocity, AccFactor);
            Face(i);
            break;
        case AI_STATE_SEPARATION:
            AIAgents.Target[i] = GetMousePosition();
            Seek(i);
            Separation(i);
            LookVelocity(i);
            break;
        case AI_STATE_COLLISION:
            AIAgents.Target[i] = GetMousePosition();
            Seek(i);
            Collision(i);
            LookVelocity(i);
            break;
        default:
            return;
        }

        const float DeltaTime = GetFrameTime();

        // Update 0th derivatives
        AIAgents.Position[i].x += DeltaTime * AIAgents.Velocity[i].x;
        AIAgents.Position[i].y += DeltaTime * AIAgents.Velocity[i].y;
        AIAgents.Orientation[i] += DeltaTime * AIAgents.Rotation[i];

        // Update 1st derivatives
        AIAgents.Velocity[i].x += DeltaTime * AIAgents.Acceleration[i].x;
        AIAgents.Velocity[i].y += DeltaTime * AIAgents.Acceleration[i].y;
        AIAgents.Rotation[i] += DeltaTime * AIAgents.Angular[i];

        // Speed cap
        if (Vector2LengthSqr(AIAgents.Velocity[i]) > MAX_SPEED * MAX_SPEED) {
            AIAgents.Velocity[i] = Vector2Scale(Vector2Normalize(AIAgents.Velocity[i]), MAX_SPEED);
        }

    }
}

void AIAgentsDraw() {
    for (unsigned i = 0; i < AIAgents.Count; i++) {
        const vector2 Forward = RadToVector2(AIAgents.Orientation[i]);
        const vector2 Right = { -Forward.y, Forward.x };
        const float   DiagonalRadius = HALF_ROOT_2 * AIAgents.Radius[i];
        const vector2 FrontPoint = Vector2Scale(Forward, AIAgents.Radius[i]);
        const vector2 LeftPoint = Vector2Add(Vector2Scale(Forward, -DiagonalRadius), Vector2Scale(Right, -DiagonalRadius));
        const vector2 RightPoint = Vector2Add(Vector2Scale(Forward, -DiagonalRadius), Vector2Scale(Right, DiagonalRadius));
        DrawTriangle(Vector2Add(AIAgents.Position[i], FrontPoint),
                     Vector2Add(AIAgents.Position[i], LeftPoint),
                     Vector2Add(AIAgents.Position[i], RightPoint),
                     AIAgents.Colour[i]);

        if (bDebug) {
            //DrawLineEx(AIAgents.Position[i], Vector2Add(AIAgents.Position[i], Vector2Scale(Forward, 2.f * AIAgents.Radius[i])), 4, GUPPIE_GREEN);
            //DrawLineEx(AIAgents.Position[i], Vector2Add(AIAgents.Position[i], Vector2Scale(Right, 2.f * AIAgents.Radius[i])), 4, AZURE);
            //DrawLineEx(AIAgents.Position[i], Vector2Add(AIAgents.Position[i], AIAgents.Acceleration[i]), 2, GUPPIE_GREEN);

            // TargetPosition
            DrawCircleV(AIAgents.Target[i], 12.f, MY_RED);
        }

    }
}

const char *GetAIStateString() {
    switch (AIAgents.State) {
    case AI_STATE_SEEK:       return "AI mode: Seek";
    case AI_STATE_FLEE:       return "AI mode: Flee";
    case AI_STATE_PURSUE:     return "AI mode: Pursue";
    case AI_STATE_EVADE:      return "AI mode: Evade";
    case AI_STATE_ARRIVE:     return "AI mode: Arrive";
    case AI_STATE_WANDER:     return "AI mode: Wander";
    case AI_STATE_PATH:       return "AI mode: Path following";
    case AI_STATE_SEPARATION: return "AI mode: Separation";
    case AI_STATE_COLLISION:  return "AI mode: Collision avoidance";
    default: return "INVALID STATE!";
    }
}
