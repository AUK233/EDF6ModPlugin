#include <d3dcompiler.h>

#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"
#include "Base/g_criFS.h"
#include "Base/g_DXresource.h"

#include "cmd/cmd_GetINI.h"
#include "1FullAOData.hpp"
#include "1FullAO.h"

extern "C" {
	extern int Config_FullAO;
}
extern PBYTE hmodDLL;

ID3D11ComputeShader* pFullAOCSshader;
ID3D11ComputeShader* pMainCSshader;
static uintptr_t ao_HmodDLL;
static ID3D11Device* ao_Device;

void HookFunction_D3D11_FullAO() {
	if (!Config_FullAO) return;

	//WriteHookToProcess((void*)(hmodDLL + 0x1129B01), (void*)&nop2, 2U);
	//WriteHookToProcess((void*)(hmodDLL + 0x1129B0E), (void*)&nop2, 2U);

	WriteHookToProcess((void*)(hmodDLL + 0x117750F), (void*)&nop2, 2U);
	WriteHookToProcess((void*)(hmodDLL + 0x117752A), (void*)&nop2, 2U);
	WriteHookToProcess((void*)(hmodDLL + 0x117765A), (void*)&nop6, 6U);
}

void HookFunction_D3D11_FullAO_Install() {
	if (!Config_FullAO) return;

	auto pD3D = DXGI_D3D11Device2137130();
	auto device = pD3D->device;

	pFullAOCSshader = 0;
	device->CreateComputeShader(FullAOShaderBin, sizeof(FullAOShaderBin), nullptr, &pFullAOCSshader);

	// ================================================

	CriFileSystemGet_t getFile;
	auto pFile = getFile.Open(L"app:/mod/system/MainColorCS.hlsl");

	ID3DBlob* cs_blob = nullptr;
	ID3DBlob* error_blob = nullptr;
	auto macros = D3D11_FullAO_GetShaderMacro(Config_FullAO);
	D3DCompile(pFile->fs->data, pFile->fs->data_size, "MainColorCS.hlsl", macros, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
	device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &pMainCSshader);
	cs_blob->Release();
	if (error_blob) error_blob->Release();

	// ================================================

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


	// other

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

	// edf.dll+11B541E
	BYTE noUndergroundCS[] = {
		0x31, 0xC0
	};
	WriteHookToProcess((void*)(hmodDLL + 0x11B541E), noUndergroundCS, 2);
}

const D3D_SHADER_MACRO* D3D11_FullAO_GetShaderMacro(UINT index) {
	switch (index) {
	case 2: {
		static const D3D_SHADER_MACRO macros2[] = {
			{"_AddMetallic", "1"},
			{nullptr, nullptr}
		};
		return macros2;
	}
	case 3: {
		static const D3D_SHADER_MACRO macros3[] = {
			{"_AddMetallic", "1"},
			{"_AddBlackBorder", "1"},
			{nullptr, nullptr}
		};
		return macros3;
	}
	default:
		return nullptr;
	}
	// end
}

void __fastcall D3D11_FullAO_ReloadComputeShader() {
	if (std::filesystem::exists(L"./modtest/mod/system/MainColorCS.hlsl") == false) return;

	auto pD3D = DXGI_D3D11Device2137130();
	auto device = pD3D->device;

	if (pMainCSshader) {
		pMainCSshader->Release();
		pMainCSshader = 0;
	}

	ID3DBlob* cs_blob = nullptr;
	ID3DBlob* error_blob = nullptr;
	auto macros = D3D11_FullAO_GetShaderMacro(INIConfig_ReadFullAO());
	D3DCompileFromFile(L"./modtest/mod/system/MainColorCS.hlsl", macros, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
	device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &pMainCSshader);
	cs_blob->Release();
	if (error_blob) error_blob->Release();


	int i_setCSshader = 0x1131138;
	BYTE setCSshader[] = {
		0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xBA // mov rdx
	};
	WriteHookToProcess((void*)(hmodDLL + i_setCSshader), setCSshader, 12);
	WriteHookToProcess((void*)(hmodDLL + i_setCSshader + 12), &pMainCSshader, 8);
}
