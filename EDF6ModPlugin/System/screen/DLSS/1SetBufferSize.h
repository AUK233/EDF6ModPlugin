#pragma once
#include <d3d11.h>

void HookFunction_SetD3DBufferSize(PBYTE hmodEXE);

extern "C" {
	void __fastcall GetPlayerD3DBufferNewSize(Pg_D3D_ResourceInfo pResource, int playerCount, int playerIndex, UINT* out);

	float __fastcall GetPlayerNewSightGlobalResolution(UINT* out, const UINT* in);
	void __fastcall GetPlayerNewLockonGlobalResolution(UINT* out, const UINT* in);
}

bool __fastcall GetPlayerImageOutResolution(void* pSys, void* out, int playerIndex);
