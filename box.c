#include "box.h"
#include "colours.h"
#include <float.h>
#include "maths.h"

boxes Boxes;

void BoxInit(unsigned Id, vector2 Position, float XRadius, float YRadius, float Orientation, colour Colour) {
    const vector2 Up = RadToVector2(Orientation);
    Boxes.Points[Id][0].x = Position.x - XRadius * Up.y - YRadius * Up.x;
    Boxes.Points[Id][0].y = Position.y + XRadius * Up.x - YRadius * Up.y;
    Boxes.Points[Id][1].x = Position.x - XRadius * Up.y + YRadius * Up.x;
    Boxes.Points[Id][1].y = Position.y + XRadius * Up.x + YRadius * Up.y;
    Boxes.Points[Id][2].x = Position.x + XRadius * Up.y + YRadius * Up.x;
    Boxes.Points[Id][2].y = Position.y - XRadius * Up.x + YRadius * Up.y;
    Boxes.Points[Id][3].x = Position.x + XRadius * Up.y - YRadius * Up.x;
    Boxes.Points[Id][3].y = Position.y - XRadius * Up.x - YRadius * Up.y;
    Boxes.Colour[Id] = Colour;
}

void BoxesInit() {
	const int RandMin[] = {   64,  192 };
	const int RandMax[] = { 2496, 1280 };
    for (unsigned i = 0; i < Boxes.Count; i++) {
        const vector2 BoxPosition = (vector2){
            rand() % (RandMax[0] - RandMin[0]) + RandMin[0],
            rand() % (RandMax[1] - RandMin[1]) + RandMin[1]
        };
        if (i & 1) {
            // Wall-like
            BoxInit(i, BoxPosition, (rand() & 255) + 32, (rand() & 31) + 32, (float)rand() / RAND_MAX * TWO_PI, AMBER);
        } else {
            // Box-like
            BoxInit(i, BoxPosition, (rand() &  31) + 64, (rand() & 31) + 64, (float)rand() / RAND_MAX * TWO_PI, AMBER);
        }
    }
}

ray_hit BoxRayIntersect(unsigned BoxId, vector2 RayStart, vector2 RayDelta) {
    ray_hit Result     = { .bHit = false, .Distance = FLT_MAX };
    char    BoxPointId = -1;
    vector2 FirstLine;
    for (char i = 0; i < 4; i++) {
        const char    Next = (i < 3) * (i + 1);
        const vector2 Line = Vector2Subtract(Boxes.Points[BoxId][Next], Boxes.Points[BoxId][i]);
        float Denominator = Line.x * RayDelta.y - Line.y * RayDelta.x;
        if (Denominator == 0.f) {
            continue;
        }
        const float y31 = Boxes.Points[BoxId][i].y - RayStart.y;
        const float x31 = Boxes.Points[BoxId][i].x - RayStart.x;
        Denominator = 1.f / Denominator;

        const float RayFactor = (Line.x * y31 - Line.y * x31) * Denominator;
        if (RayFactor < 0.f || RayFactor > 1.f) {
            continue;
        }
        const float LineFactor = (RayDelta.x * y31 - RayDelta.y * x31) * Denominator;
        if (LineFactor < 0.f || LineFactor > 1.f) {
            continue;
        }
        const vector2 Intersect = Vector2Add(RayStart, Vector2Scale(RayDelta, RayFactor));
        const float   DistSqr   = Vector2DistanceSqr(RayStart, Intersect);
        if (DistSqr < Result.Distance) {
            Result.Point    = Intersect;
            Result.Distance = DistSqr;
            BoxPointId      = i;
            FirstLine       = Line;
        }
    }
    if (BoxPointId != -1) {
        Result.Normal   = Vector2Normalize((vector2){ -FirstLine.y, FirstLine.x });
        Result.Distance = sqrtf(Result.Distance);
        Result.bHit     = true;
    }
    return Result;
}

void BoxesDraw() {
    for (unsigned i = 0; i < Boxes.Count; i++) {
        DrawTriangle(Boxes.Points[i][0], Boxes.Points[i][1], Boxes.Points[i][2], Boxes.Colour[i]);
        DrawTriangle(Boxes.Points[i][0], Boxes.Points[i][2], Boxes.Points[i][3], Boxes.Colour[i]);
    }
}
