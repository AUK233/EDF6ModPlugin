#include <d3dcompiler.h>

#include "pch.h"
#include "utiliy.h"
#include "commonNOP.h"

#include "Base/g_criFS.h"
#include "Base/g_DXresource.h"
#include "Base/mod_inlineData.hpp"

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
	// edf.dll+11E3786
	CallGameBlock((void*)(hmodDLL + 0x11E3786), (uintptr_t)D3D11_FullAO_MixGroundAndSky);

	if (!Config_FullAO) return;

	using namespace D3D;
	g_EnhanceFullAO = (PEnhanceFullAO)_aligned_malloc(sizeof(EnhanceFullAO_t), 16);
	if (!g_EnhanceFullAO) return;

	//WriteHookToProcess((void*)(hmodDLL + 0x1129B01), (void*)&nop2, 2U);
	//WriteHookToProcess((void*)(hmodDLL + 0x1129B0E), (void*)&nop2, 2U);

	WriteHookToProcess((void*)(hmodDLL + 0x117750F), (void*)&nop2, 2U);
	WriteHookToProcess((void*)(hmodDLL + 0x117752A), (void*)&nop2, 2U);
	WriteHookToProcess((void*)(hmodDLL + 0x117765A), (void*)&nop6, 6U);

	// edf.dll+11782F6
	// load ao cs
	static_assert(sizeof(FullAOShaderBin) <= 0x608);
	int newCSOSize = sizeof(FullAOShaderBin);
	WriteHookToProcess((void*)(hmodDLL + 0x11782F0 + 2), &newCSOSize, 4);
	WriteHookToProcess((void*)(hmodDLL + 0x1FEBAB0), FullAOShaderBin, newCSOSize);

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
	WriteHookToProcess((void*)(hmodDLL + i_setCSshader), setCSshader, 20U);
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
	auto pAO = g_EnhanceFullAO;
	if (!pAO) return;


	auto pD3D = DXGI_D3D11Device2137130();
	auto device = pD3D->device;

	LPCWSTR mainName = L"./subtitle/0MainColorCS.hlsl";
	if (std::filesystem::exists(mainName)) {
		pAO->ReloadMainComputeShader(device, mainName);
	}

	LPCWSTR volLightName = L"./subtitle/0MixSkyCS.hlsl";
	if (std::filesystem::exists(volLightName)) {
		pAO->ReloadVolLightComputeShader(device, volLightName);
	}

	LPCWSTR lightningName = L"./subtitle/0LightningCS.hlsl";
	if (std::filesystem::exists(lightningName)) {
		pAO->ReloadLightningComputeShader(device, lightningName);
	}
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

	// move to D3D11_FullAO_MixGroundAndSky
	/*
	auto pPP = GetAddPostProcessPointer();
	if (pPP && g_StreamLineProcessor) {
		auto isReset = g_StreamLineProcessor->m_srReset;
		bool isClearMV = (isReset || pPP->m_playerCount != 1);
		pPP->CalculateMV(pRender, pD3D, ThreadGroupCount, isClearMV);
	}*/
	// end
}

void __fastcall D3D11_FullAO_MixGroundAndSky(void* noRCX, Pg_D3D11DeviceInfo pD3D, Pxgl_gs_TextureBase outTexture, Pxgl_gs_TextureBase inTexture) {
	using namespace D3D;
	//edf.dll+11E378D 
	auto pRender = DXGI_GetRenderer2139A98();
	auto csUtility = pRender->m_csUtility;
	auto context = pD3D->context;

	ID3D11ShaderResourceView* inT0 = nullptr;
	auto pIn0 = inTexture->pInfo;
	inT0 = pIn0->pSRV;
	context->CSSetShaderResources(0, 1, &inT0);

	context->CSSetShaderResources(2, 1, &pRender->GBuffer.RT[2].pInfo->pSRV);
	context->CSSetShaderResources(4, 1, &pRender->GBuffer.RT[5].pInfo->pSRV);
	context->CSSetShaderResources(15, 1, &pRender->g_shadow_depth.pInfo->pSRV);

	context->CSSetConstantBuffers(0, 1, &pRender->CB_xgl_system.m10->pBuffer);
	context->CSSetConstantBuffers(2, 1, &pRender->CB_xgl_additional_light_setting.m10->pBuffer);

	ID3D11UnorderedAccessView* outU0 = nullptr;
	auto pOut0 = outTexture->pInfo;
	if (pOut0) outU0 = pOut0->pUAV;
	context->CSSetUnorderedAccessViews(0, 1, &outU0, nullptr);

	ID3D11ComputeShader* mixCS = nullptr;
	auto pAO = g_EnhanceFullAO;
	if (pAO && pAO->m_level > 2){
		mixCS = pAO->m_csMixSky;
	}
	else {
		auto pCS = csUtility->m_MixGroundAndSky.pShader;
		if (pCS) mixCS = pCS->cs;
	}

	context->CSSetShader(mixCS, nullptr, 0);

	AddPostProcessRes_t ThreadGroupCount;
	ThreadGroupCount.Width = (pIn0->width + 15) / 16;
	ThreadGroupCount.Height = (pIn0->height + 15) / 16;
	context->Dispatch(ThreadGroupCount.Width, ThreadGroupCount.Height, 1);

	if (pAO) {
		auto hasLightning = pAO->UpdateLightning(pRender, context);
		if (hasLightning) {
			context->CSSetShaderResources(0, 1, &pRender->GBuffer.RT[4].pInfo->pSRV);
			context->Dispatch(ThreadGroupCount.Width, ThreadGroupCount.Height, 1);
		}
	}

	auto pPP = GetAddPostProcessPointer();
	if (pPP && g_StreamLineProcessor) {
		auto isReset = g_StreamLineProcessor->m_srReset;
		bool isClearMV = (isReset || pPP->m_playerCount != 1);
		pPP->CalculateMV(pRender, pD3D, ThreadGroupCount, isClearMV);
	}

	// ==========================================================================
	// end
	pD3D->pCurrentCSShader = 0;
	pD3D->pCurrentCSShaderResourceView[0] = 0;
	pD3D->pCurrentCSShaderResourceView[2] = 0;
	pD3D->pCurrentCSShaderResourceView[4] = 0;
	pD3D->pCurrentCSShaderResourceView[15] = 0;
	pD3D->pCurrentCSUnorderedAccessViews[0] = 0;
	pD3D->pCurrentCSBuffer[0] = 0;
	pD3D->pCurrentCSBuffer[1] = 0;
	pD3D->pCurrentCSBuffer[2] = 0;

	ID3D11Buffer* nullBuffer = nullptr;
	context->CSSetConstantBuffers(0, 1, &nullBuffer);
	context->CSSetConstantBuffers(1, 1, &nullBuffer);
	context->CSSetConstantBuffers(2, 1, &nullBuffer);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->CSSetShaderResources(0, 1, &nullSRV);
	context->CSSetShaderResources(2, 1, &nullSRV);
	context->CSSetShaderResources(4, 1, &nullSRV);
	context->CSSetShaderResources(15, 1, &nullSRV);

	ID3D11ComputeShader* nullCS = nullptr;
	context->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}

namespace D3D {
	void AddLightningState_t::Reset() {
		phase = Phase::Disable;
		flashCount = 0;
		flashIndex = 0;
		startTime = 60*60*24;
		strikeDuration = 0.0;
		fadeDuration = 0.0;
		flashInterval = 0.0;
		strikeInterval = 1.0;

		ResetCB();
	}

	void AddLightningState_t::ResetCB() {
		CB.g_lightning_color[0] = 0.85;
		CB.g_lightning_color[1] = 0.9;
		CB.g_lightning_color[2] = 1.0;
		CB.g_lightning_fade = 0.0;

		CB.g_lightning_intensity = 1.0;
		CB.g_lightning_shadow_darken = 0.5;
		CB.g_lightning_sky_amount = 1.2;
	}

	void AddLightningState_t::UpdatePhase(const float* pTime) {
		float time = *pTime;
		auto base_value = *(UINT32*)pTime;
		base_value *= 2654435761u; // Knuth

		switch (phase) {
		case Phase::Idle:
			CB.g_lightning_fade = 0;
			if (startTime < time) {
				phase = Phase::Strike;
				flashCount = 1 + (base_value % 4);
				flashIndex = 0;

				auto h = base_value ^ (base_value >> 16);
				UINT32 r1 = h % 16;
				float durTime = 0.0167f * r1;
				strikeDuration = durTime;

				UINT32 r2 = (h >> 8) % 16;
				fadeDuration = 0.5f + (durTime * 0.5f) + (r2 * 0.008f);

				if (flashCount > 1) {
					UINT32 r3 = (h >> 4) % 32;
					flashInterval = r3 * 0.0167f;
				} else {
					flashInterval = 0;
				}

				CB.g_lightning_fade = 1;
			}
			return;
		case Phase::Wait:
			CB.g_lightning_fade = 0;
			if (startTime < time) {
				auto count = flashIndex + 1;
				if (count < flashCount) {
					phase = Phase::Strike;
					CB.g_lightning_fade = 1;
				} else {
					phase = Phase::Idle;

					auto h = base_value ^ (base_value >> 16);
					UINT32 r = h % 64;
					startTime = time + (strikeInterval * r);
				}
				flashIndex = count;
			}
			return;
		case Phase::Strike: {
			auto dTime = time - startTime;
			if (dTime >= strikeDuration) {
				phase = Phase::Fade;
				startTime = time;
			}
			return;
		}
		case Phase::Fade: {
			auto dTime = time - startTime;
			if (dTime >= fadeDuration){
				phase = Phase::Wait;
				startTime = time + flashInterval;
			}
			else {
				//CB.g_lightning_fade = 1.025f - (dTime / fadeDuration);
				float t = dTime / fadeDuration;
				const float k = 4.0f;
				const float ek = 0.0183156389f; // expf(-k)
				CB.g_lightning_fade = (std::expf(-k * t) - ek) / (1.0f - ek);
			}
			return;
		}
		case Phase::Disable: return Reset();
		default: return Reset();
		}
	}


	void EnhanceFullAO_t::Initialize(ID3D11Device* device) {
		m_enable = true;
		m_level = 0;

		for (UINT i = 0; i < MAXFullAOLevel; i++) {
			m_cs[i] = 0;
			LoadMainComputeShader(device, i);
		}

		m_reload_cs = 0;

		m_csMixSky = 0;
		LoadVolLightComputeShader(device);

		CB_Lightning = 0;
		m_csLightning = 0;
		LoadLightningComputeShader(device);

		ZeroMemory(&m_lightning, sizeof(AddLightningState_t));
	}

	void EnhanceFullAO_t::LoadMainComputeShader(ID3D11Device* device, UINT level) {
		if (level >= MAXFullAOLevel) return;

		static const wchar_t* filePath[MAXFullAOLevel] = {
			L"app:/mod/system/MainColorCS_L1.cso",
			L"app:/mod/system/MainColorCS_L2.cso",
			L"app:/mod/system/MainColorCS_L3.cso",
			L"app:/mod/system/MainColorCS_L4.cso",
			L"app:/mod/system/MainColorCS_L5.cso"
		};

		CriFileSystemGet_t getFile;
		auto pFile = getFile.Open(filePath[level]);

		if (!pFile->manager) return;

		device->CreateComputeShader(pFile->fs->data, pFile->fs->data_size, nullptr, &m_cs[level]);
	}

	void EnhanceFullAO_t::LoadVolLightComputeShader(ID3D11Device* device) {
		CriFileSystemGet_t getFile;
		auto pFile = getFile.Open(L"app:/mod/system/MixSkyCS.cso");

		if (!pFile->manager) return;

		device->CreateComputeShader(pFile->fs->data, pFile->fs->data_size, nullptr, &m_csMixSky);
	}

	void EnhanceFullAO_t::LoadLightningComputeShader(ID3D11Device* device) {
		D3D11_BUFFER_DESC cb_desc0 = {};
		cb_desc0.Usage = D3D11_USAGE_DEFAULT;
		cb_desc0.ByteWidth = sizeof(AddLightningStateConstants_t);
		cb_desc0.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc0.CPUAccessFlags = 0;
		device->CreateBuffer(&cb_desc0, nullptr, &CB_Lightning);

		CriFileSystemGet_t getFile;
		auto pFile = getFile.Open(L"app:/mod/system/LightningCS.cso");

		if (!pFile->manager) return;

		device->CreateComputeShader(pFile->fs->data, pFile->fs->data_size, nullptr, &m_csLightning);
	}

	void EnhanceFullAO_t::GetLevel(int in) {
		if (!in) {
			m_enable = false;
			m_level = 0;
			return;
		}

		UINT level = in - 1;
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
		case 4: {
			static const D3D_SHADER_MACRO macros4[] = {
				{"_AddMetallic", "1"},
				{"_AddVolumetricLight", "1"},
				{nullptr, nullptr}
			};
			return macros4;
		}
		case 5: {
			static const D3D_SHADER_MACRO macros5[] = {
				{"_AddMetallic", "1"},
				{"_AddVolumetricLight", "1"},
				{"_AddBlackBorder", "1"},
				{nullptr, nullptr}
			};
			return macros5;
		}
		default:
			return nullptr;
		}
		// end
	}

	void EnhanceFullAO_t::ReloadMainComputeShader(ID3D11Device* device, LPCWSTR pFileName) {
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

	void EnhanceFullAO_t::ReloadVolLightComputeShader(ID3D11Device* device, LPCWSTR pFileName) {
		if (m_csMixSky) {
			m_csMixSky->Release();
			m_csMixSky = 0;
		}

		ID3DBlob* cs_blob = nullptr;
		ID3DBlob* error_blob = nullptr;
		D3DCompileFromFile(pFileName, nullptr, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
		device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &m_csMixSky);
		cs_blob->Release();
		if (error_blob) error_blob->Release();
	}

	void EnhanceFullAO_t::ReloadLightningComputeShader(ID3D11Device* device, LPCWSTR pFileName) {
		if (m_csLightning) {
			m_csLightning->Release();
			m_csLightning = 0;
		}

		ID3DBlob* cs_blob = nullptr;
		ID3DBlob* error_blob = nullptr;
		D3DCompileFromFile(pFileName, nullptr, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
		device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &m_csLightning);
		cs_blob->Release();
		if (error_blob) error_blob->Release();
	}

	bool EnhanceFullAO_t::UpdateLightning(PG_XglSystemRenderer pRender, ID3D11DeviceContext* context) {
		if (m_lightning.phase == AddLightningState_t::Phase::Disable) return false;

		m_lightning.UpdatePhase(&pRender->xgl_system.g_xgl_time);

		if (m_lightning.CB.g_lightning_fade <= 0.0f) return false;

		context->UpdateSubresource(CB_Lightning, 0, nullptr, &m_lightning.CB, 0, 0);
		context->CSSetConstantBuffers(1, 1, &CB_Lightning);

		context->CSSetShader(m_csLightning, nullptr, 0);
		return true;
	}
}
