#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "System/0GDXGI.h"
#include "System/screen/1FullAO.h"
#include "System/screen/DLSS/0SL_PostProcess.h"

#include "ASFunc_HookIsPs5.h"

extern PBYTE hmodDLL;

void __fastcall ASFunc_HookIsPs5_Initialize() {
	// edf.dll+70EEA0
	hookGameBlock((void*)(hmodDLL + 0x70EEA0), (uintptr_t)ASFunc_HookIsPs5_IsPs5);
}

bool ASFunc_HookIsPs5_IsPs5() {
	static int initialized = 0;
	if (initialized) return false;

	initialized = 0x55;

	auto pDXGI = DXGI_GetGameDXGIRender();
	auto device = pDXGI->pD3D11Device;

	HookFunction_D3D11_FullAO_Install(device);

	auto pPP = D3D::GetAddPostProcessPointer();
	if (pPP) pPP->LUTBuffer_Load(device);

	return false;
}
