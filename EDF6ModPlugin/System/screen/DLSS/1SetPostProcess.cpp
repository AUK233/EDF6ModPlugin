#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <format>
#include <stdexcept>
#include <list>
#include <vector>
#include <cstdlib>
#include <d3dcompiler.h>
// what!
#pragma comment(lib, "D3DCompiler.lib")
#include <wrl.h>

#include "DirectXTK/DDSTextureLoader.h"
#include "Base/g_criFS.h"

#include "1SetPostProcess.h"

namespace D3D {
	void D3DPostProcess_t::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, DXGI_SWAP_CHAIN_DESC* pChainDesc) {

		Device = device;
		Context = context;

		CurrentTime[0] = -1;
		CurrentTime[1] = -1;

		LUTSamplerLinear = 0;
		LookupTable_SRV = 0;
		LookupTable_SRV_Backup = 0;
		xgl_user_param = 0;

		//SetBuffer(pChainDesc->BufferDesc.Width, pChainDesc->BufferDesc.Height);
		LoadComputeShader();

	}

	void D3DPostProcess_t::LoadComputeShader() {
		Microsoft::WRL::ComPtr<ID3DBlob> cs_blob;
		Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
		//ID3DBlob* cs_blob = nullptr;
		//ID3DBlob* error_blob = nullptr;

		auto hr = D3DCompileFromFile(L"./subtitle/test.hlsl", nullptr, nullptr, "CS_main", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &cs_blob, &error_blob);
		if (hr == S_OK) {
			if (PostProcessCS) {
				PostProcessCS->Release();
				PostProcessCS = nullptr;
			}

			Device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &PostProcessCS);
		}
	}

	void D3DPostProcess_t::ReleaseBuffer() {
		if (OutUAV[0]) {
			OutUAV[0]->Release();
			OutUAV[0] = nullptr;
		}

		if (OutColor[0]) {
			OutColor[0]->Release();
			OutColor[0] = nullptr;
		}

		// ===========================

		if (OutUAV[1]) {
			OutUAV[1]->Release();
			OutUAV[1] = nullptr;
		}

		if (OutColor[1]) {
			OutColor[1]->Release();
			OutColor[1] = nullptr;
		}

		// ===========================

		if (BlackMV) {
			BlackMV->Release();
			BlackMV = nullptr;
		}

	}

	void D3DPostProcess_t::SetBuffer(UINT Width, UINT Height) {
		ReleaseBuffer();

		D3D11_TEXTURE2D_DESC outDesc = {};
		outDesc.Width = Width;
		outDesc.Height = Height;
		outDesc.MipLevels = 1;
		outDesc.ArraySize = 1;
		outDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		outDesc.SampleDesc.Count = 1;
		outDesc.SampleDesc.Quality = 0;
		outDesc.Usage = D3D11_USAGE_DEFAULT;
		outDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
		outDesc.CPUAccessFlags = 0;
		outDesc.MiscFlags = 0;
		Device->CreateTexture2D(&outDesc, 0, &OutColor[0]);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = outDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		Device->CreateUnorderedAccessView(OutColor[0], &uavDesc, &OutUAV[0]);

		if (PlayerCount == 2) {
			Device->CreateTexture2D(&outDesc, 0, &OutColor[1]);
			Device->CreateUnorderedAccessView(OutColor[1], &uavDesc, &OutUAV[1]);
		}

		// set black mv buffer
		outDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
		outDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		UINT pixelSize = Width * 4;  // 2 bytes (X) + 2 bytes (Y)
		UINT dataSize = Height * pixelSize;
		std::vector<BYTE> zeroData(dataSize, 0);
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = zeroData.data();
		initData.SysMemPitch = pixelSize;
		initData.SysMemSlicePitch = dataSize;
		Device->CreateTexture2D(&outDesc, &initData, &BlackMV);
	}

	void D3DPostProcess_t::Execute(Pg_D3D11DeviceInfo pD3D, int playerIndex) {
		ID3D11DeviceContext* pContext = pD3D->context;

		pD3D->pCurrentCSShaderResourceView[9] = &LookupTable_SRV;
		pContext->CSSetShaderResources(9, 1, &LookupTable_SRV);
		pD3D->pCurrentCSSamplerState[11] = &LUTSamplerLinear;
		pContext->CSSetSamplers(11, 1, &LUTSamplerLinear);

		pD3D->pCurrentCSBuffer[2] = &xgl_user_param;
		pContext->CSSetConstantBuffers(2, 1, &xgl_user_param);

		pD3D->pCurrentCSShader = PostProcessCS;
		pContext->CSSetShader(PostProcessCS, nullptr, 0);

		pD3D->pCurrentCSUnorderedAccessViews[0] = &OutUAV[playerIndex];
		pContext->CSSetUnorderedAccessViews(0, 1, &OutUAV[playerIndex], nullptr);
	}

	void D3DPostProcess_t::LoadLUTBuffer() {
		// create linear sampler
		D3D11_SAMPLER_DESC samp_desc = {};
		samp_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samp_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samp_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samp_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samp_desc.MaxAnisotropy = 1;
		samp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samp_desc.MinLOD = 0;
		samp_desc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT hr = Device->CreateSamplerState(&samp_desc, &LUTSamplerLinear);


		D3D11_BUFFER_DESC cb_desc2 = {};
		cb_desc2.Usage = D3D11_USAGE_DEFAULT;
		cb_desc2.ByteWidth = sizeof(PostProcessConstants_t);
		cb_desc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc2.CPUAccessFlags = 0;
		Device->CreateBuffer(&cb_desc2, nullptr, &xgl_user_param);

		LookupTable_SRV = CreateLUTBuffer(0);
	}

	ID3D11ShaderResourceView* D3DPostProcess_t::CreateLUTBuffer(int index) {
		CriFileSystemGet_t getFile;
		auto pFile = getFile.Open(GetLUTFilePath(index));

		ID3D11ShaderResourceView* out;
		DirectX::CreateDDSTextureFromMemory(Device, pFile->fs->data, pFile->fs->data_size, nullptr, &out);

		return out;
	}

	void D3DPostProcess_t::SetPostProcessInfo(int lutindex, float blendFactor, float brightness){
		CB2.pp_brightness = brightness;
		CB2.pp_blendFactor = blendFactor;

		Context->UpdateSubresource(xgl_user_param, 0, nullptr, &CB2, 0, 0);

		if (LookupTable_SRV_Backup) {
			LookupTable_SRV_Backup->Release();
			LookupTable_SRV_Backup = nullptr;
		}

		if (LUTindex == lutindex) return;

		LUTindex = lutindex;
		LookupTable_SRV_Backup = LookupTable_SRV;
		LookupTable_SRV = CreateLUTBuffer(lutindex);
	}

	std::wstring D3DPostProcess_t::GetLUTFilePath(int index){
		
		switch (index) {
		case EnvLookupTableTexture_NightVisionLowLight:
			return L"app:/mod/system/LUT_NightVisionLowLight.dds";
		case EnvLookupTableTexture_NightVisionInfrared:
			return L"app:/mod/system/LUT_NightVisionInfrared.dds";
		case EnvLookupTableTexture_HistoricalReproduction:
			return L"app:/mod/system/LUT_HistoricalReproduction.dds";
		case EnvLookupTableTexture_LowSaturationHighContrast:
			return L"app:/mod/system/LUT_LowSaturationHighContrast.dds";
		case EnvLookupTableTexture_CoolNeutralTones:
			return L"app:/mod/system/LUT_CoolNeutralTones.dds";
		default:
			return L"app:/mod/system/LUT_DefaultEnhance.dds";
		}
		// end
	}

// end
}
