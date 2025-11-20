#pragma once

#define TWO_PI       (6.283185307179586476925286766559f)
#define HALF_PI      (1.5707963267948966192313216916398f)
#define HALF_ROOT_2  (0.70710678118654752440084436210485f)

// Maps the input to [-PI, PI) interval.
inline float MapToRangeRad(float Rad) {
	if (fabsf(Rad) > PI) {
		Rad -= TWO_PI * roundf(Rad / TWO_PI);
	}
	return Rad;
}
