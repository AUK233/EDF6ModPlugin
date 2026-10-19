#pragma once
#include <d3d11.h>
#include "Base/g_DXresource.h"

namespace D3D {
	enum EnvLookupTableTexture {
		EnvLookupTableTexture_DefaultEnhance = 0,
		EnvLookupTableTexture_NightVisionLowLight = 1,
		EnvLookupTableTexture_NightVisionInfrared = 2,
		EnvLookupTableTexture_HistoricalReproduction = 3,
		EnvLookupTableTexture_LowSaturationHighContrast = 4,
		EnvLookupTableTexture_CoolNeutralTones = 5,
	};

	// constant buffer data structure
	typedef struct alignas(16) PostProcessConstants_t {
		float pp_brightness;
		float pp_blendFactor;
		float pp_pad[2];
	}*PPostProcessConstantsConstants;

	class D3DPostProcess_t {
	public:
		ID3D11Device* Device;
		ID3D11DeviceContext* Context;

		ID3D11Texture2D* OutColor[2];
		ID3D11UnorderedAccessView* OutUAV[2];
		ID3D11ComputeShader* PostProcessCS;
		ID3D11Texture2D* BlackMV;

		ID3D11SamplerState* LUTSamplerLinear;
		ID3D11ShaderResourceView* LookupTable_SRV;
		ID3D11ShaderResourceView* LookupTable_SRV_Backup; // temporarily reserve the LUT when switching

		ID3D11Buffer* xgl_user_param; // cb2
		PostProcessConstants_t CB2;
		int LUTindex;

		int PlayerCount;
		int CurrentTime[2];

		void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, DXGI_SWAP_CHAIN_DESC* pChainDesc);
		void LoadComputeShader();
		void ReleaseBuffer();
		void SetBuffer(UINT Width, UINT Height);
		void Execute(Pg_D3D11DeviceInfo pD3D, int playerIndex);

		void LoadLUTBuffer();
		ID3D11ShaderResourceView* CreateLUTBuffer(int index);
		void SetPostProcessInfo(int lutindex = 0, float blendFactor = 0.985, float brightness = 1);
		std::wstring GetLUTFilePath(int index);
	};

	typedef D3DPostProcess_t* PD3DPostProcess;
}