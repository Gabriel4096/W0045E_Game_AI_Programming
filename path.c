#include "path.h"

#include "colours.h"

void PathDraw(vector2 Nodes[]) {
	for (unsigned char i = 0; i < PATH_NODE_COUNT; i++) {
		DrawCircleV(Nodes[i], 16.f, GUPPIE_GREEN);
		
		const unsigned char Next = (i < PATH_NODE_COUNT - 1) ? i + 1 : 0;
		DrawLineEx(Nodes[i], Nodes[Next], 2, GUPPIE_GREEN);

		const char Number[2] = { '1' + i, '\0'};
		DrawText(Number, Nodes[i].x + 16.f, Nodes[i].y + 16.f, 32, GUPPIE_GREEN);
	}
}
