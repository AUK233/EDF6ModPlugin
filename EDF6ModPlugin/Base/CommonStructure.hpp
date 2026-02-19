#pragma once
#include "stl/string.h"

typedef void(__fastcall* void_fastcallNoParameter)();

typedef struct EDFVector_t {
	void* pData;
	size_t capacity;
	size_t size;
} *PEDFVector;
