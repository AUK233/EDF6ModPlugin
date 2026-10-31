#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "Base/mod/mod_GlobalManager.h"
#include "System/screen/DLSS/0SL_wapper.h"
#include "System/screen/1FullAO.h"

#include "System/screen/DLSS/1SetPostProcess.h"

#include "ASFunc_HookMap.h"
#include "ASFunc_Graphic.h"

extern D3D::PD3DPostProcess pD3DPostProcess;
extern D3D::PEnhanceFullAO g_EnhanceFullAO;
extern PMODGlobalManager g_MODGlobalManager;

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
	streamline_Reset();

	if (g_MODGlobalManager) {
		g_MODGlobalManager->Reset();
	}

	auto pRenderer = DXGI_GetRenderer2139A98();
	pRenderer->xgl_additional_light_setting.g_shadow_intensity = 1;
	pRenderer->xgl_additional_light_setting.g_outline_color = 0;

	auto pAO = g_EnhanceFullAO;
	if (pAO) {
		pAO->m_lightning.Reset();
		//pAO->m_lightning.phase = D3D::AddLightningState_t::Phase::Idle;
		//pAO->m_lightning.startTime = pRenderer->xgl_system.g_xgl_time + 2;
	}

	if (pD3DPostProcess) {
		pD3DPostProcess->SetPostProcessInfo();
	}
}
