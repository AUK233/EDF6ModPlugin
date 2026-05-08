#include "pch.h"

#include "g_DXresource.h"

extern "C" {
	uintptr_t pOutRenderColor2139A78; // edf.dll+2139A78
}

void __fastcall XGS_DXresource_Initialize(PBYTE hmodDLL) {
	pOutRenderColor2139A78 = (uintptr_t)(hmodDLL + 0x2139A78);
}
