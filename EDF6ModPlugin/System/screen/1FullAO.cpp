#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"

#include <d3dcompiler.h>
#include "1FullAOData.hpp"
#include "1FullAO.h"

ID3D11ComputeShader* pFullAOCSshader;
ID3D11ComputeShader* pMainCSshader;

void HookFunction_D3D11_FullAO(PBYTE hmodDLL) {
    pFullAOCSshader = (ID3D11ComputeShader*)hmodDLL; // Use it temporarily
}

void HookFunction_D3D11_FullAO_Install(ID3D11Device* pDevice) {
	uintptr_t hmodDLL = (uintptr_t)pFullAOCSshader;

    //WriteHookToProcess((void*)(hmodDLL + 0x1129B01), (void*)&nop2, 2U);
    //WriteHookToProcess((void*)(hmodDLL + 0x1129B0E), (void*)&nop2, 2U);

    WriteHookToProcess((void*)(hmodDLL + 0x117750F), (void*)&nop2, 2U);
    WriteHookToProcess((void*)(hmodDLL + 0x117752A), (void*)&nop2, 2U);
    WriteHookToProcess((void*)(hmodDLL + 0x117765A), (void*)&nop6, 6U);

    pFullAOCSshader = 0;
    pDevice->CreateComputeShader(FullAOShaderBin, sizeof(FullAOShaderBin), nullptr, &pFullAOCSshader);

    pMainCSshader = 0;
    //pDevice->CreateComputeShader(MainCSShaderBin, sizeof(MainCSShaderBin), nullptr, &pMainCSshader);

    ID3DBlob* cs_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
    D3DCompileFromFile(L"Z:/TEMP/1.hlsl", nullptr, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
    pDevice->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &pMainCSshader);
    cs_blob->Release();
    if (error_blob) error_blob->Release();


    // Calculate Linear Depth
    // edf.dll+1177701 set cs shader

    /**/
    BYTE dispatchX[] = {
        0xC1, 0xE8, 0x04,
        0x41, 0xF6, 0xC5, 0x0F
    };
    WriteHookToProcess((void*)(hmodDLL + 0x11777D1), dispatchX, 7);

    BYTE dispatchY[] = {
        0xC1, 0xEA, 0x04,
        0x41, 0xF6, 0xC4, 0x0F
    };
    WriteHookToProcess((void*)(hmodDLL + 0x11777EC), dispatchY, 7);

    int i_setCSshader = 0x11776D0;
    BYTE setCSshader[] = {
        0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x48, 0xBA // mov rdx
    };
    WriteHookToProcess((void*)(hmodDLL + i_setCSshader), setCSshader, 12);
    WriteHookToProcess((void*)(hmodDLL + i_setCSshader + 12), &pFullAOCSshader, 8);

    // edf.dll+1131138
    i_setCSshader = 0x1131138;
    WriteHookToProcess((void*)(hmodDLL + i_setCSshader), setCSshader, 12);
    WriteHookToProcess((void*)(hmodDLL + i_setCSshader + 12), &pMainCSshader, 8);

}
