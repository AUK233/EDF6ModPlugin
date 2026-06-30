#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "DLSS/0SetDLSS.h"
#include "1FullAO.h"
#include "0D3D11.h"

extern "C" {
	extern int Config_PostProcess;
	extern int Config_DLAA;

	void __fastcall ASMdx11CreateDevice();
	uintptr_t dx11CreateDeviceRetAddr;

	void __fastcall ASMsysExitGame();
	void __fastcall ASMgetPlayerCountInHQ(); // HUiHQCurrentStatus
	void __fastcall ASMRenderBufferToScreenBuffer();
	uintptr_t RenderBufferToScreenBufferRetAddr;
}

void HookFunction_D3D11(PBYTE hmodEXE){
	// EDF.dll+111B588
	int i_dx11CreateDevice = 0x111B588;
	hookGameBlockWithInt3((void*)(hmodEXE + i_dx11CreateDevice), (uintptr_t)ASMdx11CreateDevice);
	WriteHookToProcess((void*)(hmodEXE + i_dx11CreateDevice + 15), (void*)&nop3, 3U);
	dx11CreateDeviceRetAddr = (uintptr_t)(hmodEXE + i_dx11CreateDevice + 18);

	if (Config_DLAA || Config_PostProcess) {
		// EDF.dll+8776DB, Sys_Exit_Game
		hookGameBlock14((void*)(hmodEXE + 0x8776DB), (uintptr_t)ASMsysExitGame);

		// EDF.dll+883DE3
		int i_HQ = 0x883DE3;
		hookGameBlockWithInt3((void*)(hmodEXE + i_HQ), (uintptr_t)ASMgetPlayerCountInHQ);
		WriteHookToProcess((void*)(hmodEXE + i_HQ + 15), (void*)&nop3, 3U);

		// EDF.dll+1151F3B
		int i_rb2sb = 0x1151F3B;
		hookGameBlockWithInt3((void*)(hmodEXE + i_rb2sb), (uintptr_t)ASMRenderBufferToScreenBuffer);
		WriteHookToProcess((void*)(hmodEXE + i_rb2sb + 15), (void*)&nop1, 1U);
		RenderBufferToScreenBufferRetAddr = (uintptr_t)(hmodEXE + i_rb2sb + 16);
	}

	//MessageBoxW(NULL, L"just test", L"debug", MB_OK);
	HookFunction_D3D11_FullAO(hmodEXE);
}

typedef void(__stdcall* ID3D11DeviceContext_OMSetRenderTargets)(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView);
ID3D11DeviceContext_OMSetRenderTargets fnID3D11DeviceContext_OMSetRenderTargets = 0;

HRESULT WINAPI module_InitializeD3D11(DXGI_SWAP_CHAIN_DESC* pChainDesc, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	//auto debugText = std::format(L"SampleDesc, count: {0}", pChainDesc->SampleDesc.Count);
	//MessageBoxW(NULL, debugText.c_str(), L"debug", MB_OK); 

	auto result = D3D11CreateDevice(0, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	if (result < 0) return result;

	// Since g_color_out is calculated based on 2x its resolution, upscaling it won't help.
	HookFunction_D3D11_FullAO_Install(*ppDevice);

	if (Config_DLAA || Config_PostProcess) {
		pChainDesc->SampleDesc.Count = 1; // old is 8
		pChainDesc->SampleDesc.Quality = 0;

		if (!fnID3D11DeviceContext_OMSetRenderTargets) {
			auto pVTable = *(ID3D11DeviceContext_OMSetRenderTargets**)(*ppImmediateContext);
			auto addrOMSetRenderTargets = &pVTable[33]; // vft+0x108
			fnID3D11DeviceContext_OMSetRenderTargets = *addrOMSetRenderTargets;

			//auto addrToHook = (uintptr_t)New_OMSetRenderTargets;
			//WriteHookToProcess(addrOMSetRenderTargets, &addrToHook, 8U);
		}

		DLSS_Initialization(ppDevice, ppImmediateContext, pChainDesc);
	}

	return result;
}

void New_OMSetRenderTargets(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView) {
	//DLSS_GetBuffer(pContext, NumViews, ppRenderTargetViews, pDepthStencilView);
	return fnID3D11DeviceContext_OMSetRenderTargets(pContext, NumViews, ppRenderTargetViews, pDepthStencilView);
}
