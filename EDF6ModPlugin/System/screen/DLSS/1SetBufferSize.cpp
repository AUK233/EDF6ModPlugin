#include "pch.h"
#include "commonNOP.h"
#include "utiliy.h"

#include "Base/g_DXresource.h"
#include "System/0GDXGI.h"
#include "0SL_PostProcess.h"

#include "1SetBufferSize.h"

extern "C" {
	extern int Config_EnhanceAO;

	void __fastcall ASMsetPlayerGraphicsSize();
	uintptr_t setPlayerGraphicsSizeRetAddr;

	void __fastcall ASMsetPlayerSightGlobalResolution();
	uintptr_t setPlayerSightGlobalResolutionRetAddr;

	void __fastcall ASMsetPlayerLockonGlobalResolution();
	uintptr_t setPlayerLockonGlobalResolutionRetAddr;
}

void HookFunction_SetD3DBufferSize(PBYTE hmodEXE) {
	// EDF.dll+1DC67B
	int i_PlayerGraphic = 0x1DC67B;
	hookGameBlockWithInt3((void*)(hmodEXE + i_PlayerGraphic), (uintptr_t)ASMsetPlayerGraphicsSize);
	WriteHookToProcess((void*)(hmodEXE + i_PlayerGraphic + 15), (void*)&nop2, 2U);
	setPlayerGraphicsSizeRetAddr = (uintptr_t)(hmodEXE + 0x1DC67B + 58);

	// EDF.dll+118E025
	int i_PlayerSight = 0x118E025;
	hookGameBlockWithInt3((void*)(hmodEXE + i_PlayerSight), (uintptr_t)ASMsetPlayerSightGlobalResolution);
	WriteHookToProcess((void*)(hmodEXE + i_PlayerSight + 15), (void*)&nop2, 2U);
	setPlayerSightGlobalResolutionRetAddr = (uintptr_t)(hmodEXE + 0x118E06A);

	// EDF.dll+82BA5
	int i_PlayerLockon = 0x82BA5;
	hookGameBlockWithInt3((void*)(hmodEXE + i_PlayerLockon), (uintptr_t)ASMsetPlayerLockonGlobalResolution);
	WriteHookToProcess((void*)(hmodEXE + i_PlayerLockon + 15), (void*)&nop2, 2U);
	setPlayerLockonGlobalResolutionRetAddr = (uintptr_t)(hmodEXE + i_PlayerLockon + 17);

	// EDF.dll+1195E20
	int i_PlayerShortcutChat = 0x1195E20;
	hookGameBlockRAXWithInt3((void*)(hmodEXE + i_PlayerShortcutChat), (uintptr_t)GetPlayerImageOutResolution);
}

void __fastcall GetPlayerD3DBufferNewSize(Pg_D3D_ResourceInfo pResource, int playerCount, int playerIndex, UINT* out) {
	auto pPP = D3D::GetAddPostProcessPointer();
	if (pPP) {
		if (playerIndex > 0) {
			out[0] = pPP->m_playerFullResolution.Width;
		}

		out[2] = pPP->m_playerResolution.Width;
		out[3] = pPP->m_playerResolution.Height;
		return;
	}

	auto pDXGI = DXGI_GetGameDXGIRender();
	UINT width = pDXGI->DXGISwapChainDesc.BufferDesc.Width;
	if (playerCount == 2) {
		width /= 2;
		out[0] = width * playerIndex;
	}
	out[2] = width;
	out[3] = pDXGI->DXGISwapChainDesc.BufferDesc.Height;
	/*
	int width = pResource->width;

	if (playerCount > 1){
		width /= 2;
		out[0] = width * playerIndex;
	}

	out[2] = width / 2;
	out[3] = pResource->height / 2;*/
}

float __fastcall GetPlayerNewSightGlobalResolution(UINT* out, const UINT* in) {
	memcpy(out, in, 8);

	__m128i v_Ires;
	auto pPP = D3D::GetAddPostProcessPointer();
	if (pPP) {
		v_Ires = _mm_loadu_si64(&pPP->m_playerFullResolution.Width);
	} else {
		v_Ires = _mm_loadu_si64(&in[2]);
	}

	_mm_storeu_si64(&out[2], v_Ires);

	__m128 v_res = _mm_cvtepi32_ps(v_Ires);

	return v_res.m128_f32[0] / v_res.m128_f32[1];
}

void __fastcall GetPlayerNewLockonGlobalResolution(UINT* out, const UINT* in) {
	memcpy(out, in, 8);
	auto pPP = D3D::GetAddPostProcessPointer();
	if (pPP) {
		out[2] = pPP->m_playerFullResolution.Width;
		out[3] = pPP->m_playerFullResolution.Height;
	} else {
		out[2] = in[2];
		out[3] = in[3];
	}
}

bool __fastcall GetPlayerImageOutResolution(void* pSys, void* out, int playerIndex) {
	auto sys = (PXGS_System)pSys;
	auto player = &sys->player[playerIndex];

	struct OUT_t {
		PBYTE p00;
		PBYTE p08;
		UINT resolution[4];
		bool b20;
	};
	static_assert(offsetof(OUT_t, b20) == 0x20);

	auto pOut = (OUT_t*)out;
	bool retValue;
	if (player->int64_20) {
		auto pRAW = (PBYTE)player;
		pOut->p00 = pRAW + 0x5C0 - 0x4D0;
		pOut->p08 = pRAW + 0x5E0 - 0x4D0;
		retValue = true;
	} else {
		pOut->p00 = nullptr;
		retValue = false;
	}
	pOut->b20 = retValue;
	
	memcpy(pOut->resolution, player->m_resolution, 16);
	auto pPP = D3D::GetAddPostProcessPointer();
	if (pPP) {
		pOut->resolution[2] = pPP->m_playerFullResolution.Width;
		pOut->resolution[3] = pPP->m_playerFullResolution.Height;
	}

	return retValue;
}
