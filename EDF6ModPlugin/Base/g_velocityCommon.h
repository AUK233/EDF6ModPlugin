#pragma once
#include "SSE.hpp"

// edf.dll+4D940
void* __fastcall Game_GetNormalizedVelocity(float* data, float speed);
// theta: cos, sin, -sin, cos
// standard xy rotate: x*c - y*s, x*s + y*c
// so zx rotate: z*c - -x*s, -(z*s + -x*c)
//        => xz: x*c - z*s, x*s + z*c
// As angle is inverted, we must use inverse sin
void* __fastcall Velocity_GetNewOrientation(float* data, const float* theta);
// edf.dll+4E1F0
float __fastcall Velocity_GetNewAngle(const float* pos);
