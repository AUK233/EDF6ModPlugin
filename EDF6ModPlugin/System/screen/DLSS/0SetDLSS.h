#pragma once
#include <d3d11.h>
#include "Base/g_DXresource.h"

extern "C" {
	void __fastcall DLSS_CreateFeature(int playerCount);
	void* __fastcall DLSS_Draw(ID3D11DeviceContext* pContext, int* saveRAX);
	void __fastcall DLSS_Release();
}

void __fastcall DLSS_Initialization(ID3D11Device** ppDevice, ID3D11DeviceContext** ppImmediateContext, DXGI_SWAP_CHAIN_DESC* pChainDesc);
void __fastcall DLSS_TriggerFailureResult(UINT32 slresult, int FreeDLSS);
void __fastcall DLSS_SetFeature(ID3D11DeviceContext* pContext, UINT Width, UINT Height);
void __fastcall DLSS_GetBuffer(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView);
void __fastcall DLSS_ClearBuffer();
void __fastcall DLSS_Reset();
float __fastcall DLSS_Halton(int index, int base);
void __fastcall DLSS_GetJitter(float* out);
void __fastcall DLSS_Evaluate();

void __fastcall DLSS_Reload();
