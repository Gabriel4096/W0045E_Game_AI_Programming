#include "path.h"

#include "colours.h"
#include "consts.h"
#include <float.h>
#include <stdlib.h>
#include "raymath.h"

void PathInit(path *Path) {
	Path->Nodes[0] = (vector2){ rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT };
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) * (i + 1);
		if (Next) {
			Path->Nodes[Next] = (vector2){ rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT };
		}
		//Path->Directions[i] = Vector2Subtract(Path->Nodes[Next], Path->Nodes[i]);

		//const float Length = Vector2Length(Path->Directions[i]);
		//Path->Directions[i].x /= Length;
		//Path->Directions[i].y /= Length;
		//Path->Lengths[i] = Length;
	}
}

vector2 PathFindClosestPoint(const path *Path, const vector2 Point) {
	vector2 *Node;
	vector2  ClosestPoint;
	float    PrevDistSqr = FLT_MAX;
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) * (i + 1);
		const vector2 Direction = Vector2Subtract(Path->Nodes[Next], Path->Nodes[i]);
		const vector2 Delta     = Vector2Subtract(Point, Path->Nodes[i]);
		const float   Param     = Vector2DotProduct(Delta, Direction);
		vector2 PointOnLine;
		if (Param <= 0.f) {
			PointOnLine = Path->Nodes[i];
		} else if (Vector2DotProduct(Vector2Subtract(Point, Path->Nodes[Next]), Vector2Negate(Direction)) <= 0.f) {
			PointOnLine = Path->Nodes[Next];
		} else {
			PointOnLine = Vector2Add(Path->Nodes[i], Vector2Scale(Direction, Param / Vector2LengthSqr(Direction)));
		}

		//const vector2 Delta       = Vector2Subtract(Point, Path->Nodes[i]);
		//const float   Param       = Clamp(Vector2DotProduct(Delta, Path->Directions[i]), 0.f, Path->Lengths[i]);
		//const vector2 PointOnLine = Vector2Add(Path->Nodes[i], Vector2Scale(Path->Directions[i], Param));
		const float DistSqr = Vector2DistanceSqr(Point, PointOnLine);
		if (DistSqr < PrevDistSqr) {
			PrevDistSqr  = DistSqr;
			ClosestPoint = PointOnLine;
			Node         = Path->Nodes + i;
		}
	}
	DrawCircleV(ClosestPoint, 12.f, MY_ORANGE);
	return ClosestPoint;
}

void PathDraw(path *Path) {
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		DrawCircleV(Path->Nodes[i], 16.f, GUPPIE_GREEN);
		
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) * (i + 1);
		DrawLineEx(Path->Nodes[i], Path->Nodes[Next], 2, GUPPIE_GREEN);

		const char Number[2] = { '1' + i, '\0' };
		DrawText(Number, Path->Nodes[i].x + 16.f, Path->Nodes[i].y + 16.f, 32, GUPPIE_GREEN);
	}
}
