#include "path.h"

#include "ai_agent.h"
#include "colours.h"
#include "consts.h"
#include <float.h>
#include <stdlib.h>
#include "raymath.h"

extern bool bDebug;

void PathInit(path *Path) {
	const int RandMin[] = {   64,  192 };
	const int RandMax[] = { 2496, 1280 };
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		Path->Nodes[i] = (vector2){
			rand() % (RandMax[0] - RandMin[0]) + RandMin[0],
			rand() % (RandMax[1] - RandMin[1]) + RandMin[1]
		};
	}
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) * (i + 1);
		Path->Directions[i] = Vector2Subtract(Path->Nodes[Next], Path->Nodes[i]);

		const float Length = Vector2Length(Path->Directions[i]);
		Path->Directions[i] = Vector2Scale(Path->Directions[i], 1.f / Length);
		Path->Lengths[i]    = Length;
	}
}

vector2 PathGetTarget(const path *Path, ai_agent *AIAgent) {
	float NodeParam;

	// Find the closest point on path
	vector2 ClosestPoint;
	if (AIAgent->PathNodeId != PATH_INVALID_NODE) {
		const vector2 Delta = Vector2Subtract(AIAgent->Position, Path->Nodes[AIAgent->PathNodeId]);
		NodeParam    = Clamp(Vector2DotProduct(Delta, Path->Directions[AIAgent->PathNodeId]), 0.f, Path->Lengths[AIAgent->PathNodeId]);
		ClosestPoint = Vector2Add(Path->Nodes[AIAgent->PathNodeId], Vector2Scale(Path->Directions[AIAgent->PathNodeId], NodeParam));
	} else {
		float PrevDistSqr = FLT_MAX;
		for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
			const vector2 Delta = Vector2Subtract(AIAgent->Position, Path->Nodes[i]);
			NodeParam = Clamp(Vector2DotProduct(Delta, Path->Directions[i]), 0.f, Path->Lengths[i]);

			const vector2 PointOnLine = Vector2Add(Path->Nodes[i], Vector2Scale(Path->Directions[i], NodeParam));
			const float   DistSqr     = Vector2DistanceSqr(AIAgent->Position, PointOnLine);
			if (DistSqr <= PrevDistSqr) {
				AIAgent->PathNodeId = i;
				PrevDistSqr  = DistSqr;
				ClosestPoint = PointOnLine;
			}
		}
	}
	if (bDebug) {
		DrawCircleV(ClosestPoint, 12.f, MY_ORANGE);
	}

	// Find target on path
	const float Offset = AI_SLOW_RADIUS;
	unsigned char TargetNodeId = AIAgent->PathNodeId;
	NodeParam += Offset;
	while (NodeParam >= Path->Lengths[TargetNodeId]) {
		NodeParam   -= Path->Lengths[TargetNodeId];
		TargetNodeId = (TargetNodeId < PATH_NODE_COUNT - 1) * (TargetNodeId + 1);
		if (TargetNodeId == AIAgent->PathNodeId) {
			break;
		}
	}

	// Check if go to new node
	const vector2 Target = Vector2Add(Path->Nodes[TargetNodeId], Vector2Scale(Path->Directions[TargetNodeId], NodeParam));
	const float   Thresh = 0.5f * AI_SLOW_RADIUS;
	if (Vector2DistanceSqr(AIAgent->Position, Target) <= Thresh * Thresh) {
		AIAgent->PathNodeId = TargetNodeId;
	}

	return Target;
}

void PathDraw(const path *Path) {
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) * (i + 1);
		DrawLineEx(Path->Nodes[i], Path->Nodes[Next], 2, GUPPIE_GREEN);

		if (bDebug) {
			DrawCircleV(Path->Nodes[i], 16.f, GUPPIE_GREEN);

			const char Number[2] = { '1' + i, '\0' };
			DrawText(Number, Path->Nodes[i].x + 16.f, Path->Nodes[i].y + 16.f, 32, GUPPIE_GREEN);
		}
	}
}
