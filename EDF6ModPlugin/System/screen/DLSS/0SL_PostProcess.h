#pragma once
#include "0SL_common.h"
#include "Base/g_DXresource.h"
#include "Base/g_system.h"

namespace D3D{
	constexpr int MAXPostProcessTexIndex = 2;

	enum EnvLookupTableTexture {
		EnvLookupTableTexture_NightVisionLowLight = 0,
		EnvLookupTableTexture_NightVisionInfrared = 1,
		EnvLookupTableTexture_HistoricalReproduction = 2,
	};

	typedef struct AddPostProcessRes_t {
		UINT Width, Height;
	} *PAddPostProcessRes;

	typedef struct alignas(16) AddPostProcessRenderConstants_t {
		int m_offset;
		float pp_pad[3];
	}*PAddPostProcessRenderConstants;

	class AddPostProcess_t {
	public:
		int m_DrawDone;
		bool m_bNeedReload;
		DXSharedTexture2D m_RenderRes; ID3D11UnorderedAccessView* m_RenderUAV;
		ID3D11Buffer* CB_RenderConstants; ID3D11ComputeShader* m_ToRenderCS;

		AddPostProcessRes_t m_playerResolution, m_playerFullResolution, m_fullResolution;

		DXSharedTexture2D m_ColorRes[2];
		ID3D11ShaderResourceView* m_ColorSRV[2];
		ID3D11UnorderedAccessView* m_ColorUAV[2];

		DXSharedTexture2D m_DepthRes[2]; ID3D11UnorderedAccessView* m_DepthUAV[2];

		ID3D11ComputeShader* m_PostProcessCS; ID3D11SamplerState* m_LUTSamplerLinear;
		ID3D11ShaderResourceView* m_LookupTable_SRV;
		ID3D11ShaderResourceView* m_LookupTable_SRV_Backup; // temporarily reserve the LUT when switching

		// DLSS
		DXSharedTexture2D m_MotionVectorRes; ID3D11UnorderedAccessView* m_MotionVectorUAV;
		DXSharedTexture2D m_fgMVRes; ID3D11UnorderedAccessView* m_fgMVUAV;
		DXSharedTexture2D m_MidColorRes[2]; ID3D11ShaderResourceView* m_MidColorSRV[2];
		ID3D11Buffer* CB_Previous_xgl_system; // this is used to calculate the motion vector
		ID3D11ComputeShader* m_MotionVectorCS;
		ID3D11ComputeShader* m_ToFGBufferCS; // FG
		// DLSS end

		// DX12 or VK
		DXSharedTexture2D m_FGhudLessRes; ID3D11UnorderedAccessView* m_FGhudLessUAV;
		DXSharedTexture2D m_FGOutputInterpRes, m_FGInputBackupRes;
		// DX12 or VK end

		int m_LUTindex, m_playerCount;
		int m_currentTime;

		void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

		void Buffer_Create(ID3D11Device* device, ID3D12Device* device12, UINT DLSS_Level);
		void Buffer_CreateFull(ID3D11Device* device, ID3D12Device* device12, UINT DLSS_Level);
		void Buffer_VKCreate(PslVulkanAPI deviceVK);
		void Buffer_Release(PslVulkanAPI deviceVK);

		void Execute(Pg_D3D11DeviceInfo pD3D, int playerIndex, const AddPostProcessRes_t& ThreadGroupCount, ID3D11ShaderResourceView** ppColor, ID3D11ShaderResourceView** ppDepth);
		void CalculateMV(PG_XglSystemRenderer pRender, Pg_D3D11DeviceInfo pD3D, const AddPostProcessRes_t& ThreadGroupCount, bool clearMV);
		void CopyBuffer(Pg_D3D11DeviceInfo pD3D, const AddPostProcessRes_t& ThreadGroupCount, Pg_D3D_ResourceInfo p1color, Pg_D3D_ResourceInfo p2color);
		void ToRenderBuffer(Pg_D3D11DeviceInfo pD3D, int playerCount, ID3D11UnorderedAccessView** outColor, int useMid);

		void LUTBuffer_Load(ID3D11Device* device);
		void LUTBuffer_Reload(ID3D11Device* device);
		ID3D11ShaderResourceView* LUTBuffer_Create(ID3D11Device* device, int index);
		std::wstring GetLUTFilePath(int index, int slot);
	};
	typedef AddPostProcess_t* PAddPostProcess;

	PAddPostProcess  __fastcall GetAddPostProcessPointer();
}
