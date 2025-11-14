#pragma once

typedef struct XGS_Object_t {
	void* vf_table;
	char pad8[0x168];
	float camera[2][4]; // 2 xyz to determine direction
} *PXGS_Object;
static_assert(offsetof(XGS_Object_t, camera) == 0x170);
