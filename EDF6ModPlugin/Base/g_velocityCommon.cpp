#include "../pch.h"

#include "g_velocityCommon.h"

void* __fastcall Game_GetNormalizedVelocity(float* data, float speed) {
	float saveS3 = data[3];

	if (speed <= 0.0f) {
		ZeroMemory(data, 16);
		data[3] = saveS3;
		return data;
	}

	__m128 v_velocity = _mm_loadu_ps(data);
	__m128 v_distance = _mm_dp_ps(v_velocity, v_velocity, 0b01110111);

	if (v_distance.m128_f32[0] < 0.0001f) return data;

	__m128 v_factor = _mm_div_ps(v_velocity, _mm_sqrt_ps(v_distance));
	__m128 v_normalized = _mm_mul_ps(_mm_set_ps1(speed), v_factor);

	_mm_storeu_ps(data, v_normalized);
	data[3] = saveS3;

	return data;
}

void* __fastcall Velocity_GetNewOrientation(float* data, const float* theta) {
	__m128 v_velocity = _mm_loadu_ps(data);
	v_velocity = _mm_shuffle_ps(v_velocity, v_velocity, MY_SHUFFLE(0, 2, 0, 2));

	__m128 v_theta = _mm_loadu_ps(theta);
	v_velocity = _mm_mul_ps(v_velocity, v_theta);
	// 0+1, 2+3
	v_velocity = _mm_hadd_ps(v_velocity, v_velocity);

	data[0] = v_velocity.m128_f32[0];
	data[2] = v_velocity.m128_f32[1];

	return data;
}

float __fastcall Velocity_GetNewAngle(const float* pos) {
	if (pos[0] == 0.0f && pos[2] == 0.0f)
		return 0.0f;

	float angle = std::atan2f(pos[0], pos[2]);
	return angle;
}
