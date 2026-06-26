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

#include "1SetPostProcess.h"

namespace D3D {
	void D3DPostProcess_t::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, DXGI_SWAP_CHAIN_DESC* pChainDesc) {

		Device = device;
		Context = context;

		CurrentTime[0] = -1;
		CurrentTime[1] = -1;

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

// end
}
