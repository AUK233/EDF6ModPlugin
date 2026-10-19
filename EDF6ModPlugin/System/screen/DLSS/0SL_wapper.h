#pragma once
#include <d3d11on12.h>
#include "System/0GDXGI.h"

void __fastcall streamline_TriggerFailureResult(UINT32 slresult, void* sl);
void __fastcall streamline_InitializeSLPointers();
bool __fastcall streamline_InitializeD3D12();
bool __fastcall streamline_InitializeVK(ID3D11Device* device);
void __fastcall streamline_CreateD3D12Device();
void __fastcall streamline_InitializePostProcess(ID3D11Device* device, ID3D11DeviceContext* context);
void __fastcall streamline_Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

extern "C"{
	bool __fastcall streamline_CreateSwapChain(DXGI_SWAP_CHAIN_DESC* pChainDesc, IDXGISwapChain1** ppSwapChain, IDXGISwapChain1* pD3D11SwapChain);
	HRESULT __fastcall streamline_SwapChainGetBuffer(IDXGISwapChain* pSwapChain, PGameDXGIRender pGameDXGI, ID3D11Texture2D** pOut);
	// false is need to call Present()
	bool __fastcall streamline_SwapChainPresent(PGameDXGIRender pGameDXGI, UINT SyncInterval);
	//
	HRESULT __fastcall streamline_SwapChainResizeBuffer(PGameDXGIRender pGameDXGI, UINT Width, UINT Height);

	void __fastcall streamline_Release(IDXGISwapChain* pDXGISwapChain);
	void __fastcall streamline_Reset();

	// playerCount is -1 means forced resolution change.
	void __fastcall streamline_SetFeature(int playerCount, UINT* pRes);
	// execute post-processing and super-resolution functions.
	void* __fastcall streamline_ExecuteSR(Pg_D3D11DeviceInfo pD3D, int OutOffset, void* saveRCX);
}

// 0 is PostProcess, 1 is DLAA, 2 is DLSSFG
// 3 is change post process lut
// 4 is change mfg count
void __fastcall streamline_SwitchStatus(int index);
