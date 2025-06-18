#pragma once

typedef struct EDFWString_t {
	wchar_t* text;
	void* pad8;
	size_t size;
	size_t capacity;
} *PEDFWString;
