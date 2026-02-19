#pragma once
#include "SSE.hpp"

typedef struct XGS_Object_t {
	void* vf_table;
	char pad8[0x58];
	Matrix3D_t transform;
	char padA0[0xD0];
	float camera[2][4]; // 2 xyz to determine direction
} *PXGS_Object;
#if 1
static_assert(offsetof(XGS_Object_t, transform) == 0x60);
static_assert(offsetof(XGS_Object_t, camera) == 0x170);
#endif
