#include <d3dcompiler.h>

#include "../../pch.h"
#include "../../utiliy.h"
#include "../../commonNOP.h"
#include "Base/g_criFS.h"
#include "Base/g_DXresource.h"

#include "DLSS/0SL_executor.h"

#include "cmd/cmd_GetINI.h"
#include "1FullAOData.hpp"
#include "1FullAO.h"

extern "C" {
	extern int Config_FullAO;
}
extern PBYTE hmodDLL;
extern D3D::PStreamLineProcessor g_StreamLineProcessor;

D3D::PEnhanceFullAO g_EnhanceFullAO = 0;

void __fastcall HookFunction_D3D11_FullAO() {
	if (!Config_FullAO) return;

	using namespace D3D;
	g_EnhanceFullAO = (PEnhanceFullAO)_aligned_malloc(sizeof(EnhanceFullAO_t), 16);

	//WriteHookToProcess((void*)(hmodDLL + 0x1129B01), (void*)&nop2, 2U);
	//WriteHookToProcess((void*)(hmodDLL + 0x1129B0E), (void*)&nop2, 2U);

	WriteHookToProcess((void*)(hmodDLL + 0x117750F), (void*)&nop2, 2U);
	WriteHookToProcess((void*)(hmodDLL + 0x117752A), (void*)&nop2, 2U);
	WriteHookToProcess((void*)(hmodDLL + 0x117765A), (void*)&nop6, 6U);

	if (!g_EnhanceFullAO) return;
	// edf.dll+11782F6
	// load ao cs
	static_assert(sizeof(FullAOShaderBin) <= 0x608);
	int newCSOSize = sizeof(FullAOShaderBin);
	WriteHookToProcess((void*)(hmodDLL + 0x11782F0 + 2), &newCSOSize, 4);
	WriteHookToProcess((void*)(hmodDLL + 0x1FEBAB0), FullAOShaderBin, newCSOSize);
}

void __fastcall HookFunction_D3D11_FullAO_Install(ID3D11Device* device) {
	if (!g_EnhanceFullAO) return;

	g_EnhanceFullAO->Initialize(device);
	g_EnhanceFullAO->GetLevel(Config_FullAO);

	//if(!g_EnhanceFullAO->m_enable) return;

	// ================================================


	// Calculate Linear Depth
	// edf.dll+1177701 set cs shader
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

void __fastcall D3D11_FullAO_ReloadComputeShader() {
	if (!g_EnhanceFullAO) return;

	LPCWSTR pFileName = L"./subtitle/0MainColorCS.hlsl";
	if (std::filesystem::exists(pFileName) == false) return;

	auto pD3D = DXGI_D3D11Device2137130();
	auto device = pD3D->device;

	g_EnhanceFullAO->ReloadComputeShader(device, pFileName);
}

void __fastcall D3D11_FullAO_Execute(PG_XglSystemRenderer pRender, Pg_D3D11DeviceInfo pD3D) {
	using namespace D3D;
	auto context = pD3D->context;

	ID3D11ComputeShader* mainCS = nullptr;
	auto pAO = g_EnhanceFullAO;
	if (pAO) {
		mainCS = pAO->m_reload_cs;
		if(!mainCS) mainCS = pAO->m_cs[pAO->m_level];
	} else {
		auto pMainCS = pRender->mainColorCS.pShader;
		if (pMainCS) mainCS = pMainCS->cs;
	}

	if (mainCS != pD3D->pCurrentCSShader){
		context->CSSetShader(mainCS, nullptr, 0);
		pD3D->pCurrentCSShader = mainCS;
	}

	auto pTexture6A8 = pRender->pTexture6A8;

	AddPostProcessRes_t ThreadGroupCount;
	ThreadGroupCount.Width = (pTexture6A8.pInfo->width + 15) / 16;
	ThreadGroupCount.Height = (pTexture6A8.pInfo->height + 15) / 16;
	// main color
	context->Dispatch(ThreadGroupCount.Width, ThreadGroupCount.Height, 1);

	auto pPP = GetAddPostProcessPointer();
	if (pPP && g_StreamLineProcessor) {
		auto isReset = g_StreamLineProcessor->m_srReset;
		bool isClearMV = (isReset || pPP->m_playerCount != 1);
		pPP->CalculateMV(pRender, pD3D, ThreadGroupCount, isClearMV);
	}
	// end
}

namespace D3D {
	void EnhanceFullAO_t::Initialize(ID3D11Device* device) {
		m_enable = true;
		m_level = 0;

		for (UINT i = 0; i < MAXFullAOLevel; i++) {
			m_cs[i] = 0;
			LoadComputeShader(device, i);
		}

		m_reload_cs = 0;
	}

	void EnhanceFullAO_t::LoadComputeShader(ID3D11Device* device, UINT level) {
		if (level >= MAXFullAOLevel) return;

		const std::wstring filePath[MAXFullAOLevel] = {
			L"app:/mod/system/MainColorCS_L1.cso",
			L"app:/mod/system/MainColorCS_L2.cso",
			L"app:/mod/system/MainColorCS_L3.cso"
		};

		CriFileSystemGet_t getFile;
		auto pFile = getFile.Open(filePath[level]);

		if (!pFile->manager) return;

		device->CreateComputeShader(pFile->fs->data, pFile->fs->data_size, nullptr, &m_cs[level]);
	}

	void EnhanceFullAO_t::GetLevel(int in) {
		if (!in) {
			m_enable = false;
			m_level = 0;
			return;
		}

		UINT level = in;
		if (level >= MAXFullAOLevel) level = MAXFullAOLevel - 1;

		m_level = level;
	}

	const D3D_SHADER_MACRO* EnhanceFullAO_t::GetShaderMacro(UINT index) {
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

	void EnhanceFullAO_t::ReloadComputeShader(ID3D11Device* device, LPCWSTR pFileName) {
		if (m_reload_cs) {
			m_reload_cs->Release();
			m_reload_cs = 0;
		}

		ID3DBlob* cs_blob = nullptr;
		ID3DBlob* error_blob = nullptr;
		auto macros = GetShaderMacro(INIConfig_ReadFullAO());
		D3DCompileFromFile(pFileName, macros, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
		device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &m_reload_cs);
		cs_blob->Release();
		if (error_blob) error_blob->Release();
	}
}
