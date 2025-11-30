#include "path.h"

#include "ai_agents.h"
#include "colours.h"
#include "consts.h"
#include <float.h>
#include <stdlib.h>
#include "raymath.h"

extern ai_agents AIAgents;
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
		Path->Lengths[i]    = Length;
		Path->Directions[i] = (Length >= FLT_EPSILON) ? Vector2Scale(Path->Directions[i], 1.f / Length) : Vector2Zero();
	}
}

vector2 PathGetTarget(const path *Path, unsigned AIId) {
	float NodeParam;

	// Find the closest point on path
	vector2 ClosestPoint;
	if (AIAgents.PathNodeId[AIId] != PATH_INVALID_NODE) {
		const vector2 Delta = Vector2Subtract(AIAgents.Position[AIId], Path->Nodes[AIAgents.PathNodeId[AIId]]);
		NodeParam    = Clamp(Vector2DotProduct(Delta, Path->Directions[AIAgents.PathNodeId[AIId]]), 0.f, Path->Lengths[AIAgents.PathNodeId[AIId]]);
		ClosestPoint = Vector2Add(Path->Nodes[AIAgents.PathNodeId[AIId]], Vector2Scale(Path->Directions[AIAgents.PathNodeId[AIId]], NodeParam));
	} else {
		float PrevDistSqr = FLT_MAX;
		for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
			const vector2 Delta = Vector2Subtract(AIAgents.Position[AIId], Path->Nodes[i]);
			NodeParam = Clamp(Vector2DotProduct(Delta, Path->Directions[i]), 0.f, Path->Lengths[i]);

			const vector2 PointOnLine = Vector2Add(Path->Nodes[i], Vector2Scale(Path->Directions[i], NodeParam));
			const float   DistSqr     = Vector2DistanceSqr(AIAgents.Position[AIId], PointOnLine);
			if (DistSqr <= PrevDistSqr) {
				AIAgents.PathNodeId[AIId] = i;
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
	unsigned char TargetNodeId = AIAgents.PathNodeId[AIId];
	NodeParam += Offset;
	while (NodeParam >= Path->Lengths[TargetNodeId]) {
		NodeParam   -= Path->Lengths[TargetNodeId];
		TargetNodeId = (TargetNodeId < PATH_NODE_COUNT - 1) * (TargetNodeId + 1);
		if (TargetNodeId == AIAgents.PathNodeId[AIId]) {
			break;
		}
	}

	// Check if go to new node
	const vector2 Target = Vector2Add(Path->Nodes[TargetNodeId], Vector2Scale(Path->Directions[TargetNodeId], NodeParam));
	const float   Thresh = 0.5f * AI_SLOW_RADIUS;
	if (Vector2DistanceSqr(AIAgents.Position[AIId], Target) <= Thresh * Thresh) {
		AIAgents.PathNodeId[AIId] = TargetNodeId;
	}

	return Target;
}

void PathDraw(const path *Path) {
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) * (i + 1);
		DrawLineEx(Path->Nodes[i], Path->Nodes[Next], 2, GUPPIE_GREEN);

		if (bDebug) {
			DrawCircleV(Path->Nodes[i], 16.f, GUPPIE_GREEN);

			const unsigned char Number = i + 1;
			const char NumberText[] = { '0' + Number / 10, '0' + Number % 10, '\0' };
			DrawText(NumberText, Path->Nodes[i].x + 16.f, Path->Nodes[i].y + 16.f, 32, GUPPIE_GREEN);
		}
	}
}
