#pragma once

typedef struct EDFWString_t {
	union {
		wchar_t* pText;
		wchar_t text[8];
	};
	size_t size;
	size_t capacity;

	wchar_t* c_str();
} *PEDFWString;
