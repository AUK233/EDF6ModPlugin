#pragma once
#pragma comment(lib, "DXGI.lib")
#include "0SL_common.h"
#include "lib/nvsdk_ngx.h"
#include "lib/nvsdk_ngx_defs.h"
#include "lib/nvsdk_ngx_params.h"
#include "lib/nvsdk_ngx_helpers.h"
#include "lib/nvsdk_ngx_helpers_dlssg_d3d.h"
#include "0SL_PostProcess.h"

#include "System/0GDXGI.h"

constexpr auto D3D12WrappedBackBuffersCount = 3;
constexpr auto DLSSMaxMultiFrameCount = 5;

namespace D3D {
	enum class StreamLineProcessorStatus_t : int {
		eFailed = 0,
		eD3D11 = 1,
		eD3D12 = 2,
		eVULKAN = 3,
	};

	__declspec(align(16)) class StreamLineProcessor_t {
	public:
		union {
			StreamLineProcessorStatus_t m_status;
			int m_renderAPI;
		};
		bool m_bIsSplitScreen, m_bNeedFG, m_bNeednoSleep;
		PslVulkanAPI vk;

		// dlss =================================================
		NVSDK_NGX_Parameter* m_srParameters;
		NVSDK_NGX_Handle* m_srFeature;
		NVSDK_NGX_Parameter* m_fgParameters;
		NVSDK_NGX_Handle* m_fgFeature;
		int m_srReset, m_fgReset, m_multiFrameCount;
		int JitterIndex; float v_jitter[2];
		UINT m_inputResolution[2], m_outResolution[2];
		NVSDK_NGX_PerfQuality_Value m_DLSSMode;
		float m_sharpness;
		// dlss end =============================================

		// FG =====================================================
		ID3D11DeviceContext4* m_context4;
		DXSharedTexture2D g_gameSceneResource; ID3D11UnorderedAccessView* g_gameSceneResourceUAV;
		ID3D11Texture2D* g_D3D11BackBuffer; // DXVK is simulations D3D swap chain buffer
		// FG end =================================================

		// D3D12 =====================================================
		ID3D12Device* m_d3d12Device; ID3D12CommandQueue* m_commandQueue;
		IDXGIFactory7* m_dxgiFactory; IDXGISwapChain3* m_dxgiSwapChain3;
		ID3D12CommandAllocator* m_commandAllocator;
		ID3D12GraphicsCommandList* m_commandList;

		ID3D12Fence* m_fenceNative; UINT64 m_nativeFenceValue; HANDLE m_fenceEvent;
		ID3D11Fence* m_fence11; ID3D12Fence* m_fence12; UINT64 m_shareFenceValue;

		ID3D12Resource* v_d3d12BackBuffers[D3D12WrappedBackBuffersCount];
		// D3D12 End ===================================================



		void __fastcall CreateD3D12Device();

		HRESULT __fastcall SwapChainGetBuffer(IDXGISwapChain* pSwapChain, PGameDXGIRender pGameDXGI, ID3D11Texture2D** pOut);
		bool __fastcall SwapChainPresent(PGameDXGIRender pGameDXGI, UINT SyncInterval);


		void __fastcall D3D12GetBuffer();
		void __fastcall D3D12CommandBegin();
		void __fastcall D3D12CommandEnd();
		void __fastcall D3D12WaitFinish();
		void __fastcall D3D12SetPresentBuffer();

		void __fastcall GetDLSSMode(int in);
		void __fastcall GetDLSSResolution(UINT Width, UINT Height);
		void __fastcall SetDLSSPreset(NVSDK_NGX_DLSS_Hint_Render_Preset in);
		bool __fastcall GetParameters();
		void __fastcall SetFeature(ID3D11DeviceContext* context, UINT Width, UINT Height, int playerCount);
		void __fastcall Release();
		void __fastcall ReleaseFeature();
		void __fastcall Reset();

		float __fastcall Halton(int index, int base);
		void __fastcall GetJitter(int playerCount);

		// D3D11
		void __fastcall EvaluateSR(ID3D11DeviceContext* context, PAddPostProcess pPP, Pg_DeferredRenderSystem_GBufferData pRTV, NVSDK_NGX_D3D11_DLSS_Eval_Params& D3D11DlssEvalParams, int playerIndex);
		// D3D12
		void __fastcall EvaluateSR(PAddPostProcess pPP, NVSDK_NGX_D3D12_DLSS_Eval_Params& D3D12DlssEvalParams, int playerIndex);
		// VULKAN
		void __fastcall EvaluateSR(PAddPostProcess pPP, NVSDK_NGX_VK_DLSS_Eval_Params& D3DvkDlssEvalParams, int playerIndex);

		void* __fastcall GetPlayer(int index);
		PAddPostProcess __fastcall CheckNeedFG();

		bool __fastcall D3D12EvaluateFG(PGameDXGIRender pGameDXGI, UINT SyncInterval);
		// D3D12
		void EvaluateFG(NVSDK_NGX_D3D12_DLSSG_Eval_Params& FGParams, NVSDK_NGX_DLSSG_Opt_Eval_Params& OptEvalParams, PAddPostProcess pPP);

		bool __fastcall VKEvaluateFG(PGameDXGIRender pGameDXGI, UINT SyncInterval);
		// VULKAN
		void EvaluateFG(NVSDK_NGX_VK_DLSSG_Eval_Params& FGParams, NVSDK_NGX_DLSSG_Opt_Eval_Params& OptEvalParams, PAddPostProcess pPP);

		// because only VK supports the software frame sorting for driver.
		void VULKANUnlockMFG();
	};
	typedef StreamLineProcessor_t* PStreamLineProcessor;
}