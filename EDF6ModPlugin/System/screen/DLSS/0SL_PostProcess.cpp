#include "pch.h"
#include <d3dcompiler.h>
#include "DirectXTK/DDSTextureLoader.h"
#include "System/0GDXGI.h"

#include "Base/g_system.h"
#include "Base/g_criFS.h"
#include "../shader/1SetPostProcess_Render.hpp"
#include "../shader/1SetPostProcess_CS.hpp"
#include "../shader/1SetPostProcess_MV.hpp"

#include "0SL_PostProcess.h"

extern "C" {
	extern int Config_PostProcess;
	extern int Config_PostProcessTexIndex;
}

extern D3D::PAddPostProcess g_AddPostProcess;

namespace D3D {
	void AddPostProcess_t::Initialize(ID3D11Device* device, ID3D11DeviceContext* context){

		// create constant buffer
		D3D11_BUFFER_DESC cb_desc0 = {};
		cb_desc0.Usage = D3D11_USAGE_DEFAULT;
		cb_desc0.ByteWidth = sizeof(xgl_system_CB_t);
		cb_desc0.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc0.CPUAccessFlags = 0;
		device->CreateBuffer(&cb_desc0, nullptr, &CB_Previous_xgl_system);

		cb_desc0.ByteWidth = sizeof(AddPostProcessRenderConstants_t);
		device->CreateBuffer(&cb_desc0, nullptr, &CB_RenderConstants);

		// load compute shader
		ComPtr<ID3DBlob> cs_blob;
		ComPtr<ID3DBlob> error_blob;

		if (Config_PostProcess == 2) {
			auto hr = D3DCompileFromFile(L"./subtitle/test.hlsl", nullptr, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
			if (hr == S_OK) {
				device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &m_PostProcessCS);
			}
		} else {
			device->CreateComputeShader(D3DPostProcess_ComputeShader, sizeof(D3DPostProcess_ComputeShader), nullptr, &m_PostProcessCS);
		}

		device->CreateComputeShader(D3DPostProcess_ToRenderBuffer, sizeof(D3DPostProcess_ToRenderBuffer), nullptr, &m_ToRenderCS);
		device->CreateComputeShader(D3DPostProcess_MotionVector, sizeof(D3DPostProcess_MotionVector), nullptr, &m_MotionVectorCS);
	}

	void AddPostProcess_t::Buffer_Create(ID3D11Device* device, ID3D12Device* device12, UINT DLSS_Level) {

		D3D11_TEXTURE2D_DESC outDesc = {};
		outDesc.Width = m_playerResolution.Width;
		outDesc.Height = m_playerResolution.Height;
		outDesc.MipLevels = 1;
		outDesc.ArraySize = 1;
		outDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		outDesc.SampleDesc.Count = 1;
		outDesc.SampleDesc.Quality = 0;
		outDesc.Usage = D3D11_USAGE_DEFAULT;
		outDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
		outDesc.CPUAccessFlags = 0;
		outDesc.MiscFlags = 0;
		m_ColorRes[0].D3D_Create(device, &outDesc, device12);

		D3D11_TEXTURE2D_DESC outDescP2;
		memcpy(&outDescP2, &outDesc, sizeof(D3D11_TEXTURE2D_DESC));
		if (m_playerCount != 2){
			outDescP2.Width = 1;
			outDescP2.Height = 1;
		}
		m_ColorRes[1].D3D_Create(device, &outDescP2, device12);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		if (m_ColorRes[0].d11) {
			device->CreateShaderResourceView(m_ColorRes[0].d11, &srvDesc, &m_ColorSRV[0]);
			device->CreateUnorderedAccessView(m_ColorRes[0].d11, &uavDesc, &m_ColorUAV[0]); 
		}
		if (m_ColorRes[1].d11) {
			device->CreateShaderResourceView(m_ColorRes[1].d11, &srvDesc, &m_ColorSRV[1]);
			device->CreateUnorderedAccessView(m_ColorRes[1].d11, &uavDesc, &m_ColorUAV[1]);
		}

		// set depth buffer
		outDesc.Format = DXGI_FORMAT_R32_FLOAT;
		uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
		m_DepthRes[0].D3D_Create(device, &outDesc, device12);
		if (m_DepthRes[0].d11) device->CreateUnorderedAccessView(m_DepthRes[0].d11, &uavDesc, &m_DepthUAV[0]);

		if (m_playerCount == 2) {
			m_DepthRes[1].D3D_Create(device, &outDesc, device12);

			if (m_DepthRes[1].d11) device->CreateUnorderedAccessView(m_DepthRes[1].d11, &uavDesc, &m_DepthUAV[1]);
		}

		// =================================================================

		if (DLSS_Level < 1) return;
		// set mv buffer
		outDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
		uavDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
		m_MotionVectorRes.D3D_Create(device, &outDesc, device12);
		m_fgMVRes.D3D_Create(device, &outDesc, device12);
		if (m_MotionVectorRes.d11) device->CreateUnorderedAccessView(m_MotionVectorRes.d11, &uavDesc, &m_MotionVectorUAV);
		if (m_fgMVRes.d11) device->CreateUnorderedAccessView(m_fgMVRes.d11, &uavDesc, &m_fgMVUAV);

		// end
	}

	void AddPostProcess_t::Buffer_CreateFull(ID3D11Device* device, ID3D12Device* device12, UINT DLSS_Level) {

		m_playerFullResolution = m_fullResolution;
		if (m_playerCount == 2) m_playerFullResolution.Width /= 2;


		D3D11_TEXTURE2D_DESC outDesc = {};
		outDesc.Width = m_fullResolution.Width;
		outDesc.Height = m_fullResolution.Height;
		outDesc.MipLevels = 1;
		outDesc.ArraySize = 1;
		outDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		outDesc.SampleDesc.Count = 1;
		outDesc.SampleDesc.Quality = 0;
		outDesc.Usage = D3D11_USAGE_DEFAULT;
		outDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
		outDesc.CPUAccessFlags = 0;
		outDesc.MiscFlags = 0;
		m_RenderRes.D3D_Create(device, &outDesc, device12);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		if (m_RenderRes.d11) device->CreateUnorderedAccessView(m_RenderRes.d11, &uavDesc, &m_RenderUAV);

		// set fg color buffer
		m_FGhudLessRes.D3D_Create(device, &outDesc, device12);
		if (m_FGhudLessRes.d11) device->CreateUnorderedAccessView(m_FGhudLessRes.d11, &uavDesc, &m_FGhudLessUAV);

		// =================================================================

		if (DLSS_Level < 1) return;
		// set mid color buffer

		D3D11_TEXTURE2D_DESC outDescP1;
		memcpy(&outDescP1, &outDesc, sizeof(D3D11_TEXTURE2D_DESC));
		outDescP1.Width = m_playerFullResolution.Width;
		outDescP1.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		m_MidColorRes[0].D3D_Create(device, &outDescP1, device12);

		D3D11_TEXTURE2D_DESC outDescP2;
		memcpy(&outDescP2, &outDescP1, sizeof(D3D11_TEXTURE2D_DESC));
		if (m_playerCount != 2) {
			outDescP2.Width = 1;
			outDescP2.Height = 1;
		}
		m_MidColorRes[1].D3D_Create(device, &outDescP2, device12);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		if (m_MidColorRes[0].d11) device->CreateShaderResourceView(m_MidColorRes[0].d11, &srvDesc, &m_MidColorSRV[0]);
		if (m_MidColorRes[1].d11) device->CreateShaderResourceView(m_MidColorRes[1].d11, &srvDesc, &m_MidColorSRV[1]);

		// =================================================================

		if (DLSS_Level < 2) return;

		outDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_FGOutputInterpRes.D3D_Create(device, &outDesc, device12);
		m_FGInputBackupRes.D3D_Create(device, &outDesc, device12);
	}

	void AddPostProcess_t::Buffer_VKCreate(PslVulkanAPI deviceVK) {
		if (!deviceVK) return;

		m_RenderRes.VK_CreateFromD11(deviceVK);

		m_ColorRes[0].VK_CreateFromD11(deviceVK);
		m_ColorRes[1].VK_CreateFromD11(deviceVK);

		m_MotionVectorRes.VK_CreateFromD11(deviceVK);
		m_fgMVRes.VK_CreateFromD11(deviceVK);
		m_DepthRes[0].VK_CreateFromD11(deviceVK, VK_IMAGE_ASPECT_DEPTH_BIT);
		m_DepthRes[1].VK_CreateFromD11(deviceVK, VK_IMAGE_ASPECT_DEPTH_BIT);
		m_MidColorRes[0].VK_CreateFromD11(deviceVK);
		m_MidColorRes[1].VK_CreateFromD11(deviceVK);
		m_FGhudLessRes.VK_CreateFromD11(deviceVK);
		m_FGOutputInterpRes.VK_CreateFromD11(deviceVK);
		m_FGInputBackupRes.VK_CreateFromD11(deviceVK);
	}

	void AddPostProcess_t::Buffer_Release(PslVulkanAPI deviceVK) {
		D3DResourceCommonRelease((IUnknown**)&m_RenderUAV);
		m_RenderRes.Release(deviceVK);

		D3DResourceCommonRelease((IUnknown**)&m_ColorSRV[0]);
		D3DResourceCommonRelease((IUnknown**)&m_ColorUAV[0]);
		m_ColorRes[0].Release(deviceVK);
		D3DResourceCommonRelease((IUnknown**)&m_ColorSRV[1]);
		D3DResourceCommonRelease((IUnknown**)&m_ColorUAV[1]);
		m_ColorRes[1].Release(deviceVK);

		D3DResourceCommonRelease((IUnknown**)&m_MotionVectorUAV);
		m_MotionVectorRes.Release(deviceVK);
		D3DResourceCommonRelease((IUnknown**)&m_fgMVUAV);
		m_fgMVRes.Release(deviceVK);

		D3DResourceCommonRelease((IUnknown**)&m_DepthUAV[0]);
		m_DepthRes[0].Release(deviceVK);
		D3DResourceCommonRelease((IUnknown**)&m_DepthUAV[1]);
		m_DepthRes[1].Release(deviceVK);

		D3DResourceCommonRelease((IUnknown**)&m_MidColorSRV[0]);
		m_MidColorRes[0].Release(deviceVK);
		D3DResourceCommonRelease((IUnknown**)&m_MidColorSRV[1]);
		m_MidColorRes[1].Release(deviceVK);

		D3DResourceCommonRelease((IUnknown**)&m_FGhudLessUAV);
		m_FGhudLessRes.Release(deviceVK);

		m_FGOutputInterpRes.Release(deviceVK);
		m_FGInputBackupRes.Release(deviceVK);
	}

	void AddPostProcess_t::Execute(Pg_D3D11DeviceInfo pD3D, int playerIndex, const AddPostProcessRes_t& ThreadGroupCount, ID3D11ShaderResourceView** ppColor, ID3D11ShaderResourceView** ppDepth){
		auto context = pD3D->context;

		context->CSSetShaderResources(0, 1, ppColor);
		context->CSSetShaderResources(1, 1, ppDepth);
		context->CSSetShaderResources(9, 1, &m_LookupTable_SRV);
		context->CSSetSamplers(11, 1, &m_LUTSamplerLinear);

		context->CSSetShader(m_PostProcessCS, nullptr, 0);

		context->CSSetUnorderedAccessViews(0, 1, &m_ColorUAV[playerIndex], nullptr);
		context->CSSetUnorderedAccessViews(1, 1, &m_DepthUAV[playerIndex], nullptr);
		context->Dispatch(ThreadGroupCount.Width, ThreadGroupCount.Height, 1);

		pD3D->pCurrentCSShaderResourceView[0] = 0;
		pD3D->pCurrentCSShaderResourceView[1] = 0;
		pD3D->pCurrentCSShaderResourceView[9] = 0;
		pD3D->pCurrentCSShader = 0;
		pD3D->pCurrentCSSamplerState[11] = 0;
		pD3D->pCurrentCSUnorderedAccessViews[0] = 0;
		pD3D->pCurrentCSUnorderedAccessViews[1] = 0;

		ID3D11ShaderResourceView* nullSRV = nullptr;
		context->CSSetShaderResources(0, 1, &nullSRV);
		context->CSSetShaderResources(1, 1, &nullSRV);
		context->CSSetShaderResources(9, 1, &nullSRV);
		ID3D11SamplerState* nullSampler = nullptr;
		context->CSSetSamplers(11, 1, &nullSampler);
		ID3D11ComputeShader* nullCS = nullptr;
		context->CSSetShader(nullptr, nullptr, 0);
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
	}

	void AddPostProcess_t::CalculateMV(PG_XglSystemRenderer pRender, Pg_D3D11DeviceInfo pD3D, const AddPostProcessRes_t& ThreadGroupCount, bool clearMV) {

		auto context = pD3D->context;
		if(!clearMV){
			//auto current_cb0 = pRender->CB_xgl_system.m10->pBuffer;
			//context->CSSetConstantBuffers(0, 1, &current_cb0);
			context->CSSetConstantBuffers(2, 1, &CB_Previous_xgl_system);

			context->CSSetShader(m_MotionVectorCS, nullptr, 0);
			context->CSSetUnorderedAccessViews(0, 1, &m_MotionVectorUAV, nullptr);
			context->CSSetUnorderedAccessViews(1, 1, &m_fgMVUAV, nullptr);

			context->Dispatch(ThreadGroupCount.Width, ThreadGroupCount.Height, 1);

			pD3D->pCurrentCSUnorderedAccessViews[1] = 0;
			ID3D11UnorderedAccessView* nullUAV = nullptr;
			context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
		}
		else {
			FLOAT clearValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			context->ClearUnorderedAccessViewFloat(m_MotionVectorUAV, clearValues);
			context->ClearUnorderedAccessViewFloat(m_fgMVUAV, clearValues);
		}

		context->UpdateSubresource(CB_Previous_xgl_system, 0, nullptr, &pRender->xgl_system, 0, 0);
	}

	void AddPostProcess_t::CopyBuffer(Pg_D3D11DeviceInfo pD3D, const AddPostProcessRes_t& ThreadGroupCount, Pg_D3D_ResourceInfo p1color, Pg_D3D_ResourceInfo p2color) {
		auto context = pD3D->context;

		context->CSSetShaderResources(0, 1, &m_MidColorSRV[0]);
		context->CSSetShader(m_ToFGBufferCS, nullptr, 0);
		context->CSSetUnorderedAccessViews(0, 1, &m_FGhudLessUAV, nullptr);
		context->CSSetUnorderedAccessViews(1, 1, &p1color->pUAV, nullptr);

		context->Dispatch(ThreadGroupCount.Width, ThreadGroupCount.Height, 1);

		pD3D->pCurrentCSShader = 0;
		pD3D->pCurrentCSShaderResourceView[0] = 0;
		pD3D->pCurrentCSUnorderedAccessViews[0] = 0;
		pD3D->pCurrentCSUnorderedAccessViews[1] = 0;

		ID3D11ShaderResourceView* nullSRV = nullptr;
		context->CSSetShaderResources(0, 1, &nullSRV);
		ID3D11ComputeShader* nullCS = nullptr;
		context->CSSetShader(nullptr, nullptr, 0);
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);

		if (p2color) context->CopyResource(p2color->pTexture, m_MidColorRes[1].d11);

		//context->Flush();
	}

	void AddPostProcess_t::ToRenderBuffer(Pg_D3D11DeviceInfo pD3D, int playerCount, ID3D11UnorderedAccessView** outColor, int useMid) {
		auto context = pD3D->context;

		UINT threadX = m_playerFullResolution.Width;
		UINT threadY = m_playerFullResolution.Height;
		AddPostProcessRenderConstants_t cb{};
		if (playerCount == 2) {
			cb.m_offset = threadX;
		} else {
			cb.m_offset = 0;
		}

		context->UpdateSubresource(CB_RenderConstants, 0, nullptr, &cb, 0, 0);

		if (useMid) {
			context->CSSetShaderResources(0, 1, &m_MidColorSRV[0]);
			context->CSSetShaderResources(1, 1, &m_MidColorSRV[1]);
		} else {
			context->CSSetShaderResources(0, 1, &m_ColorSRV[0]);
			context->CSSetShaderResources(1, 1, &m_ColorSRV[1]);
		}

		context->CSSetShader(m_ToRenderCS, nullptr, 0);

		context->CSSetUnorderedAccessViews(0, 1, &m_FGhudLessUAV, nullptr);
		if (outColor) {
			context->CSSetUnorderedAccessViews(1, 1, outColor, nullptr);
		} else {
			context->CSSetUnorderedAccessViews(1, 1, &m_RenderUAV, nullptr);
		}

		context->CSSetConstantBuffers(2, 1, &CB_RenderConstants);

		threadX += 15;
		threadY += 15;
		context->Dispatch(threadX / 16, threadY / 16, 1);

		pD3D->pCurrentCSBuffer[2] = 0;
		pD3D->pCurrentCSShader = 0;
		pD3D->pCurrentCSShaderResourceView[0] = 0;
		pD3D->pCurrentCSShaderResourceView[1] = 0;
		pD3D->pCurrentCSUnorderedAccessViews[0] = 0;
		pD3D->pCurrentCSUnorderedAccessViews[1] = 0;

		ID3D11ShaderResourceView* nullSRV = nullptr;
		context->CSSetShaderResources(0, 1, &nullSRV);
		context->CSSetShaderResources(1, 1, &nullSRV);
		ID3D11ComputeShader* nullCS = nullptr;
		context->CSSetShader(nullptr, nullptr, 0);
		ID3D11Buffer* nullBuffer = nullptr;
		context->CSSetConstantBuffers(2, 1, &nullBuffer);
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		context->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);

		if (!outColor) {
			auto pDXGI = DXGI_GetGameDXGIRender();
			auto pTexBack = pDXGI->pTextureBackBuffer;
			auto pBuffer = pTexBack->pInfo;
			context->CopyResource(pBuffer->pTexture, m_RenderRes.d11);
		}

		m_DrawDone = 1111;
	}

	void AddPostProcess_t::LUTBuffer_Load(ID3D11Device* device) {
		D3D11_SAMPLER_DESC samp_desc = {};
		samp_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samp_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samp_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samp_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samp_desc.MaxAnisotropy = 1;
		samp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samp_desc.MinLOD = 0;
		samp_desc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT hr = device->CreateSamplerState(&samp_desc, &m_LUTSamplerLinear);

		// =====================================================================
		auto index = Config_PostProcessTexIndex;
		if (index > MAXPostProcessTexIndex) index = 0;

		m_LookupTable_SRV = LUTBuffer_Create(device, index);
	}

	void AddPostProcess_t::LUTBuffer_Reload(ID3D11Device* device){
		if (m_LookupTable_SRV_Backup) {
			m_LookupTable_SRV_Backup->Release();
			m_LookupTable_SRV_Backup = nullptr;
		}

		if (m_LUTindex > MAXPostProcessTexIndex) m_LUTindex = 0;

		m_LookupTable_SRV_Backup = m_LookupTable_SRV;
		m_LookupTable_SRV = LUTBuffer_Create(device, m_LUTindex);
	}

	ID3D11ShaderResourceView* AddPostProcess_t::LUTBuffer_Create(ID3D11Device* device, int index) {
		CriFileSystemGet_t getFile;
		auto pFile = getFile.Open(GetLUTFilePath(index, 0));

		ID3D11ShaderResourceView* out;
		auto hr = DirectX::CreateDDSTextureFromMemory(device, pFile->fs->data, pFile->fs->data_size, nullptr, &out);

		//auto debugText = std::format(L"CreateDDSTextureFromMemory result: {0}", hr);
		//MessageBoxW(NULL, debugText.c_str(), L"debug", MB_OK); 

		return out;
	}

	std::wstring AddPostProcess_t::GetLUTFilePath(int index, int slot) {
		if (!slot) {
			switch (index) {
			case 1:
				return L"app:/mod/system/LUT_DefaultEnhance1.dds";
			case 2:
				return L"app:/mod/system/LUT_DefaultEnhance2.dds";
			default:
				return L"app:/mod/system/LUT_DefaultEnhance0.dds";
			}
		}

		switch (index) {
		case EnvLookupTableTexture_NightVisionLowLight:
			return L"app:/mod/system/LUT_NightVisionLowLight.dds";
		case EnvLookupTableTexture_NightVisionInfrared:
			return L"app:/mod/system/LUT_NightVisionInfrared.dds";
		case EnvLookupTableTexture_HistoricalReproduction:
			return L"app:/mod/system/LUT_HistoricalReproduction.dds";
		default:
			return L"app:/mod/system/LUT_DefaultEnhance0.dds";
		}
		// end
	}


	PAddPostProcess __fastcall GetAddPostProcessPointer(){
		return g_AddPostProcess;
	}
	// end
}
