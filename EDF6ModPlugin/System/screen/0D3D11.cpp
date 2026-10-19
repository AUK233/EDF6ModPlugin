#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include "Base/mod_inlineData.hpp"
#include "DLSS/0SetDLSS.h"
#include "DLSS/1SetBufferSize.h"
#include "1FullAO.h"
#include "0D3D11.h"
#include "DLSS/0SL_wapper.h"

extern "C" {
	extern int Config_PostProcess;
	extern int Config_DLSS;
	extern int Config_OnDX12;

	void __fastcall ASMdx11CreateDevice();
	uintptr_t dx11CreateDeviceRetAddr;

	void __fastcall ASMgetPlayerCountInHQ(); // HUiHQCurrentStatus

	void __fastcall ASMRenderBufferToScreenBuffer();
	uintptr_t RenderBufferToScreenBufferRetAddr;
	uintptr_t RenderBufferToScreenBufferRetAddr2;

	void __fastcall ASMCall_IDXGISwapChain_GetBuffer();
	uintptr_t Call_IDXGISwapChain_GetBufferRetAddr;

	void __fastcall ASMCall_IDXGISwapChain_Present();
	uintptr_t Call_IDXGISwapChain_PresentRetAddr;

	void __fastcall ASMCall_IDXGISwapChain_ResizeBuffer();
	uintptr_t Call_IDXGISwapChain_ResizeBufferRetAddr;

	void __fastcall ASMCall_IDXGISwapChain_ExitGame();
	uintptr_t Call_IDXGISwapChain_ExitGameRetAddr;

	void __fastcall ASMGetDXGISwapChain();
	uintptr_t GetDXGISwapChainRetAddr;
}

void HookFunction_D3D11(PBYTE hmodEXE){
	// EDF.dll+111B588
	int i_dx11CreateDevice = 0x111B588;
	hookGameBlockWithInt3((void*)(hmodEXE + i_dx11CreateDevice), (uintptr_t)ASMdx11CreateDevice);
	WriteHookToProcess((void*)(hmodEXE + i_dx11CreateDevice + 15), (void*)&nop3, 3U);
	dx11CreateDeviceRetAddr = (uintptr_t)(hmodEXE + i_dx11CreateDevice + 18);

	// edf.dll+1131138
	int i_setCSshader = 0x1131138;
	BYTE setCSshader[] = {
		0x48, 0xB8, 0xC7, 0x19, 0x46, 0x96, 0x02, 0x00, 0x00, 0x00,
		0x48, 0x89, 0xDA, // mov rdx, rbx
		0x48, 0x89, 0xF9, // mov rcx, rdi
		0xFF, 0xD0, // call rax
		0xEB, 0x53  // jmp
	};
	MOD::WritePTRData(&setCSshader[2], D3D11_FullAO_Execute);
	WriteHookToProcess((void*)(hmodEXE + i_setCSshader), setCSshader, 20U);

	if (Config_DLSS || Config_PostProcess) {
		HookFunction_SetD3DBufferSize(hmodEXE);

		// EDF.dll+8776DB, Sys_Exit_Game
		//hookGameBlock14((void*)(hmodEXE + 0x8776DB), (uintptr_t)ASMsysExitGame);

		// EDF.dll+883DE3
		int i_HQ = 0x883DE3;
		hookGameBlockWithInt3((void*)(hmodEXE + i_HQ), (uintptr_t)ASMgetPlayerCountInHQ);
		WriteHookToProcess((void*)(hmodEXE + i_HQ + 15), (void*)&nop3, 3U);

		// EDF.dll+1151F3B
		int i_rb2sb = 0x1151F3B;
		hookGameBlockWithInt3((void*)(hmodEXE + i_rb2sb), (uintptr_t)ASMRenderBufferToScreenBuffer);
		WriteHookToProcess((void*)(hmodEXE + i_rb2sb + 15), (void*)&nop1, 1U);
		RenderBufferToScreenBufferRetAddr = (uintptr_t)(hmodEXE + i_rb2sb + 16);
		RenderBufferToScreenBufferRetAddr2 = (uintptr_t)(hmodEXE + i_rb2sb + 231);

		// EDF.dll+111BE0E
		int i_dxgi_rb = 0x111BE0E;
		hookGameBlockWithInt3((void*)(hmodEXE + i_dxgi_rb), (uintptr_t)ASMCall_IDXGISwapChain_ResizeBuffer);
		WriteHookToProcess((void*)(hmodEXE + i_dxgi_rb + 15), (void*)&nop5, 5U);
		Call_IDXGISwapChain_ResizeBufferRetAddr = (uintptr_t)(hmodEXE + i_dxgi_rb + 46);

		// EDF.dll+111C0CC
		int i_dxgi_relse = 0x111C0CC;
		hookGameBlockWithInt3((void*)(hmodEXE + i_dxgi_relse), (uintptr_t)ASMCall_IDXGISwapChain_ExitGame);
		WriteHookToProcess((void*)(hmodEXE + i_dxgi_relse + 15), (void*)&nop3, 3U);
		Call_IDXGISwapChain_ExitGameRetAddr = (uintptr_t)(hmodEXE + i_dxgi_relse + 18);
	}

	if (Config_DLSS) {
		streamline_InitializeSLPointers();

		// EDF.dll+1120F06
		int i_dxgi_gb = 0x1120F06;
		hookGameBlockWithInt3((void*)(hmodEXE + i_dxgi_gb), (uintptr_t)ASMCall_IDXGISwapChain_GetBuffer);
		Call_IDXGISwapChain_GetBufferRetAddr = (uintptr_t)(hmodEXE + i_dxgi_gb + 31);

		// EDF.dll+111B2EF
		int i_dxgi_gsc = 0x111B2EF;
		hookGameBlockWithInt3((void*)(hmodEXE + i_dxgi_gsc), (uintptr_t)ASMGetDXGISwapChain);
		GetDXGISwapChainRetAddr = (uintptr_t)(hmodEXE + 0x111B362);

		// EDF.dll+1118B25
		int i_dxgi_ps = 0x1118B25;
		hookGameBlockWithInt3((void*)(hmodEXE + i_dxgi_ps), (uintptr_t)ASMCall_IDXGISwapChain_Present);
		WriteHookToProcess((void*)(hmodEXE + i_dxgi_ps + 15), (void*)&nop2, 2U);
		Call_IDXGISwapChain_PresentRetAddr = (uintptr_t)(hmodEXE + 0x1118B5C);

	}

	//MessageBoxW(NULL, L"just test", L"debug", MB_OK);

	// It must be executed when game starts, otherwise buffer size will be only half.
	HookFunction_D3D11_FullAO();
}

typedef void(__stdcall* ID3D11DeviceContext_OMSetRenderTargets)(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView);
ID3D11DeviceContext_OMSetRenderTargets fnID3D11DeviceContext_OMSetRenderTargets = 0;

HRESULT WINAPI module_InitializeD3D11(DXGI_SWAP_CHAIN_DESC* pChainDesc, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	//auto debugText = std::format(L"SampleDesc, count: {0}", pChainDesc->SampleDesc.Count);
	//MessageBoxW(NULL, debugText.c_str(), L"debug", MB_OK); 

	if (Config_OnDX12) {
		streamline_CreateD3D12Device();
	}

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	auto result = D3D11CreateDevice(0, DriverType, Software, Flags, featureLevels, _countof(featureLevels), SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	if (result < 0) return result;

	if (Config_DLSS || Config_PostProcess) {
		pChainDesc->SampleDesc.Count = 1; // old is 8
		pChainDesc->SampleDesc.Quality = 0;

		if (!fnID3D11DeviceContext_OMSetRenderTargets) {
			auto pVTable = *(ID3D11DeviceContext_OMSetRenderTargets**)(*ppImmediateContext);
			auto addrOMSetRenderTargets = &pVTable[33]; // vft+0x108
			fnID3D11DeviceContext_OMSetRenderTargets = *addrOMSetRenderTargets;

			//auto addrToHook = (uintptr_t)New_OMSetRenderTargets;
			//WriteHookToProcess(addrOMSetRenderTargets, &addrToHook, 8U);
		}

		//DLSS_Initialization(ppDevice, ppImmediateContext, pChainDesc);
		streamline_Initialize(*ppDevice, *ppImmediateContext);
	}

	return result;
}

void New_OMSetRenderTargets(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView) {
	//DLSS_GetBuffer(pContext, NumViews, ppRenderTargetViews, pDepthStencilView);
	return fnID3D11DeviceContext_OMSetRenderTargets(pContext, NumViews, ppRenderTargetViews, pDepthStencilView);
}
