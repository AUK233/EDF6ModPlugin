#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "System/screen/DLSS/0SetDLSS.h"

#include "ASFunc_HookMap.h"

extern "C" {
	void __fastcall ASMasFunc_Hook_Map();
}

void __fastcall ASFunc_HookMapSeries_Initialize(PBYTE hmodDLL) {
	// edf.dll+1B634B
	int i_asFunc_Hook_Map = 0x1B634B;
	hookGameBlockWithInt3((void*)(hmodDLL + i_asFunc_Hook_Map), (uintptr_t)ASMasFunc_Hook_Map);
	WriteHookToProcess((void*)(hmodDLL + i_asFunc_Hook_Map + 15), (void*)&nop3, 3U);
}

void __fastcall ASFunc_ToNew_Map() {
	DLSS_Reset();
}
