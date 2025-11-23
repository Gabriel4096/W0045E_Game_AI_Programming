#pragma once
#include <math.h>
#include <stdlib.h>

#define TWO_PI      (6.283185307179586476925286766559f)
#define HALF_PI     (1.5707963267948966192313216916398f)
#define HALF_ROOT_2 (0.70710678118654752440084436210485f)

// Maps the input to [-PI, PI) interval.
inline float MapToRangeRad(float Rad) {
	if (fabsf(Rad) > PI) {
		Rad -= TWO_PI * roundf(Rad / TWO_PI);
	}
	return Rad;
}

inline float RandomBinomal(void) {
	return (float)(rand() - rand()) / RAND_MAX;
}

inline float Vector2ToRad(const vector2 V) {
	return atan2f(-V.x, -V.y);
}

inline vector2 RadToVector2(const float Rad) {
	return (vector2){ -sinf(Rad), -cosf(Rad) };
}
