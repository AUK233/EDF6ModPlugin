#pragma once
#include <d3d11.h>
#pragma comment(lib, "D3D11.lib")

void HookFunction_D3D11(PBYTE hmodEXE);

extern "C" {
    HRESULT WINAPI module_InitializeD3D11(
        DXGI_SWAP_CHAIN_DESC* pChainDesc,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        _COM_Outptr_opt_ ID3D11Device** ppDevice,
        _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
        _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);
}

void New_OMSetRenderTargets(ID3D11DeviceContext* pContext, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView);
