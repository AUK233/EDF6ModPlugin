#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "System/screen/1FullAO.h"
#include "System/screen/DLSS/1SetPostProcess.h"

#include "ASFunc_HookIsPs5.h"

extern PBYTE hmodDLL;
extern D3D::PD3DPostProcess pD3DPostProcess;

void __fastcall ASFunc_HookIsPs5_Initialize() {
	// edf.dll+70EEA0
	hookGameBlock((void*)(hmodDLL + 0x70EEA0), (uintptr_t)ASFunc_HookIsPs5_IsPs5);
}

bool ASFunc_HookIsPs5_IsPs5() {
	static int initialized = 0;
	if (initialized) return false;

	initialized = 0x55;

	HookFunction_D3D11_FullAO_Install();

	if (pD3DPostProcess) pD3DPostProcess->LoadLUTBuffer();

	return false;
}
