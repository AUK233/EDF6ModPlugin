#include "pch.h"

#include "Base/g_system.h"
#include "0SL_executor.h"
#include "0SL_wapper.h"

//#define hasDX12Debug

PFN_D3D12_CREATE_DEVICE fnD3D12CreateDevice = nullptr;
PFN_D3D12_GET_DEBUG_INTERFACE fnD3D12GetDebugInterface = nullptr;

D3D::PStreamLineProcessor g_StreamLineProcessor = nullptr;
D3D::PAddPostProcess g_AddPostProcess = nullptr;
// 0 is PostProcess, 1 is DLAA, 2 is DLSSFG
bool g_bPostProcess[4] = { false, true, true, true };

extern "C"{
	extern int Config_PostProcess;
	extern int Config_PostProcessTexIndex;
	extern int Config_DLSS;
	extern int Config_OnDX12;
	extern int Config_DLSSFG;
}

void __fastcall streamline_TriggerFailureResult(UINT32 slresult, void* sl) {
#if defined(SLDEBUG)
	std::wstring hrText = std::format(L"DLSS Error Code: {:X}.\n", slresult);
	MessageBoxW(NULL, std::to_wstring((UINT32)slresult).c_str(), L"error", MB_OK);
#endif

	if (sl) {
		auto pSL = (D3D::PStreamLineProcessor)sl;
		pSL->m_renderAPI = 0;
	}
}

void __fastcall streamline_InitializeSLPointers() {
	using namespace D3D;
	if (g_StreamLineProcessor) return;

	auto p = (PStreamLineProcessor)_aligned_malloc(sizeof(StreamLineProcessor_t), 16U);
	if (!p) return;

	ZeroMemory(p, sizeof(StreamLineProcessor_t));
	g_StreamLineProcessor = p;

	p->m_status = StreamLineProcessorStatus_t::eFailed;
	p->Reset();
}

bool __fastcall streamline_InitializeD3D12() {
	// check no dxvk
	if (std::filesystem::exists(L"./dxgi.dll")) return false;
	if (!g_StreamLineProcessor) return false;

	auto hmodD3D12 = LoadLibraryW(L"d3d12.dll");
	if (!hmodD3D12) return false;

	fnD3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(hmodD3D12, "D3D12CreateDevice");
	fnD3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(hmodD3D12, "D3D12GetDebugInterface");

	return true;
}

bool __fastcall streamline_InitializeVK(ID3D11Device* device){
	using namespace D3D;

	IDXGIVkInteropDevice* pVkInterop = nullptr;
	HRESULT hr = device->QueryInterface(IID_IDXGIVkInteropDevice, (void**)&pVkInterop);
	if (!pVkInterop) return false;

	VkDevice vkDevice; VkInstance vkInstance; VkPhysicalDevice vkPhysDevice;
	VkQueue vkQueue; uint32_t QueueFamilyIndex;

	pVkInterop->GetVulkanHandles(&vkInstance, &vkPhysDevice, &vkDevice);
	pVkInterop->GetSubmissionQueue(&vkQueue, &QueueFamilyIndex);
	pVkInterop->Release();

	if (!vkDevice) return false;

	auto p = (PslVulkanAPI)_aligned_malloc(sizeof(slVulkanAPI_t), 16U);
	if (!p) return false;

	ZeroMemory(p, sizeof(slVulkanAPI_t));

	p->Initialization();
	if (!p->vulkanModule) {
		_aligned_free(p);
		return false;
	}

	p->m_vkInterop = pVkInterop;
	p->m_vkDevice = vkDevice;
	p->m_vkInstance = vkInstance;
	p->m_vkPhysDevice = vkPhysDevice;
	p->m_vkQueue = vkQueue;
	p->m_QueueFamilyIndex = QueueFamilyIndex;
	p->CreateFence();

	auto pSL = g_StreamLineProcessor;
	pSL->vk = p;

	return true;
}


void __fastcall streamline_CreateD3D12Device(){
	if (!streamline_InitializeD3D12()) return;

#if defined(hasDX12Debug)
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(fnD3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		//debugController->Release();
	}
#endif	

	auto pSL = g_StreamLineProcessor;
	//pSL->m_status = D3D::StreamLineProcessorStatus_t::eFailed;
	pSL->CreateD3D12Device();
}

void __fastcall streamline_InitializePostProcess(ID3D11Device* device, ID3D11DeviceContext* context) {
	using namespace D3D;
	if (g_AddPostProcess) return;

	auto p = (PAddPostProcess)_aligned_malloc(sizeof(AddPostProcess_t), 16U);
	if (!p) return;

	ZeroMemory(p, sizeof(AddPostProcess_t));
	g_AddPostProcess = p;

	p->Initialize(device, context);
	g_bPostProcess[0] = true;
}

void __fastcall streamline_Initialize(ID3D11Device* device, ID3D11DeviceContext* context){
	using namespace D3D;

	streamline_InitializePostProcess(device, context);
	auto pPP = g_AddPostProcess;
	if (!pPP) return;

	auto pSL = g_StreamLineProcessor;
	if (!pSL) return;


	if (pSL->m_status != D3D::StreamLineProcessorStatus_t::eD3D12) {
		if (!streamline_InitializeVK(device)) {
			pSL->m_status = D3D::StreamLineProcessorStatus_t::eD3D11;
		} else {
			pSL->m_status = D3D::StreamLineProcessorStatus_t::eVULKAN;
		}
	}

	static const char game_guid[] = "a0f57b54-1daf-4934-90ae-c4035c19df04";
	static const char game_version[] = "6.0";
#if defined(SLDEBUG)
	// 231313132 from ngx_dlss_demo
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
	std::filesystem::path slDir = exeDir / L"DLSSDebug";
	static auto dlss_path = slDir.wstring();

	const wchar_t* pluginPaths[] = { dlss_path.c_str() };

	NVSDK_NGX_FeatureCommonInfo featureCommonInfo = {};
	featureCommonInfo.PathListInfo.Path = pluginPaths;
	featureCommonInfo.PathListInfo.Length = 1;
	featureCommonInfo.LoggingInfo.DisableOtherLoggingSinks = false;
	featureCommonInfo.LoggingInfo.MinimumLoggingLevel = NVSDK_NGX_LOGGING_LEVEL_VERBOSE;

	static const wchar_t log_path[] = L"Z:\\TEMP";
	NVSDK_NGX_FeatureCommonInfo* pFeatureInfo = &featureCommonInfo;
#else
	static const wchar_t log_path[] = L".";
	NVSDK_NGX_FeatureCommonInfo* pFeatureInfo = nullptr;
#endif

	NVSDK_NGX_Result slresult;
	switch (pSL->m_status) {
	case StreamLineProcessorStatus_t::eD3D11:
		slresult = NVSDK_NGX_D3D11_Init_with_ProjectID(game_guid, NVSDK_NGX_ENGINE_TYPE_CUSTOM, game_version, log_path, device, pFeatureInfo);
		if (slresult != NVSDK_NGX_Result_Success) {
			streamline_TriggerFailureResult((UINT32)slresult + 0x100, pSL);
			return;
		}

		break;
	case StreamLineProcessorStatus_t::eD3D12:
		slresult = NVSDK_NGX_D3D12_Init_with_ProjectID(game_guid, NVSDK_NGX_ENGINE_TYPE_CUSTOM, game_version, log_path, pSL->m_d3d12Device, pFeatureInfo);
		if (slresult != NVSDK_NGX_Result_Success) {
			streamline_TriggerFailureResult((UINT32)slresult + 0x200, pSL);
			return;
		}

		break;
	case StreamLineProcessorStatus_t::eVULKAN: {
		auto vk = pSL->vk;
		slresult = NVSDK_NGX_VULKAN_Init_with_ProjectID(game_guid, NVSDK_NGX_ENGINE_TYPE_CUSTOM, game_version, log_path, vk->m_vkInstance, vk->m_vkPhysDevice, vk->m_vkDevice, nullptr, nullptr, pFeatureInfo);
		if (slresult != NVSDK_NGX_Result_Success) {
			streamline_TriggerFailureResult((UINT32)slresult + 0x300, pSL);
			return;
		}

		break;
	}
	default: break;
	}
}

bool __fastcall streamline_CreateSwapChain(DXGI_SWAP_CHAIN_DESC* pChainDesc, IDXGISwapChain1** ppSwapChain, IDXGISwapChain1* pD3D11SwapChain){
	if (pD3D11SwapChain) pD3D11SwapChain->Release();

	auto pSL = g_StreamLineProcessor;
	if(!pSL) return false;
	if (pSL->m_status != D3D::StreamLineProcessorStatus_t::eD3D12) return false;

	if (pSL->m_dxgiSwapChain3) {
		pSL->m_dxgiSwapChain3->Release();
		pSL->m_dxgiSwapChain3 = nullptr;
	}


	int backBufferCount = D3D12WrappedBackBuffersCount;


	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = pChainDesc->BufferDesc.Width;
	swapChainDesc.Height = pChainDesc->BufferDesc.Height;
	swapChainDesc.Format = pChainDesc->BufferDesc.Format;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = pChainDesc->BufferUsage;
	swapChainDesc.BufferCount = backBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = pChainDesc->Flags;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
	fullScreenDesc.RefreshRate = pChainDesc->BufferDesc.RefreshRate;
	fullScreenDesc.ScanlineOrdering = pChainDesc->BufferDesc.ScanlineOrdering;
	fullScreenDesc.Scaling = pChainDesc->BufferDesc.Scaling;
	fullScreenDesc.Windowed = pChainDesc->Windowed;

	*ppSwapChain = nullptr;
	auto hr = pSL->m_dxgiFactory->CreateSwapChainForHwnd(pSL->m_commandQueue, pChainDesc->OutputWindow, &swapChainDesc, 0, 0, ppSwapChain);

	//std::wstring hrText = std::format(L"CreateSwapChainForHwnd result: {0}.\nold BuffersCount : {0}", hr, pChainDesc->BufferCount);
	//MessageBoxW(NULL, hrText.c_str(), L"debug", MB_OK);
	if (FAILED(hr)) return false;

	auto pSwapChain = *ppSwapChain;

	// get swap chain3
	pSwapChain->QueryInterface(IID_PPV_ARGS(&pSL->m_dxgiSwapChain3));
	pSL->D3D12GetBuffer();

	return true;
}

#if defined(hasDX12Debug)
ID3D11Texture2D* tempBuffer = 0;
ID3D11Texture2D* tempBuffer2 = 0;
#endif

HRESULT __fastcall streamline_SwapChainGetBuffer(IDXGISwapChain* pSwapChain, PGameDXGIRender pGameDXGI, ID3D11Texture2D** pOut) {
	auto pSL = g_StreamLineProcessor;
	if (!pSL) {
#if defined(hasDX12Debug)
		pSwapChain->GetBuffer(0, IID_PPV_ARGS(&tempBuffer));
		tempBuffer->Release();

		auto device = pGameDXGI->pD3D11Device;

		auto dufferDesc = &pGameDXGI->DXGISwapChainDesc.BufferDesc;
		D3D11_TEXTURE2D_DESC outDesc = {};
		outDesc.Width = dufferDesc->Width;
		outDesc.Height = dufferDesc->Height;
		outDesc.MipLevels = 1;
		outDesc.ArraySize = 1;
		outDesc.Format = dufferDesc->Format;
		outDesc.SampleDesc.Count = 1;
		outDesc.SampleDesc.Quality = 0;
		outDesc.Usage = D3D11_USAGE_DEFAULT;
		outDesc.BindFlags = D3D11_BIND_RENDER_TARGET; // | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		outDesc.CPUAccessFlags = 0;
		outDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED;
		auto hr1 = device->CreateTexture2D(&outDesc, 0, &tempBuffer2);

		*pOut = tempBuffer2;
		tempBuffer2->AddRef();
		return hr1;
#else
		auto hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(pOut));
		return hr;
#endif
	}

	auto hr = pSL->SwapChainGetBuffer(pSwapChain, pGameDXGI, pOut);
	return hr;
}

bool __fastcall streamline_SwapChainPresent(PGameDXGIRender pGameDXGI, UINT SyncInterval) {
	auto pSL = g_StreamLineProcessor;
	if (!pSL) {
#if defined(hasDX12Debug)
		auto context = pGameDXGI->pD3D11DeviceContext;
		context->CopyResource(tempBuffer, tempBuffer2);
#endif
		return false;
	}

	auto presentDone = pSL->SwapChainPresent(pGameDXGI, SyncInterval);
	return presentDone;
}

HRESULT __fastcall streamline_SwapChainResizeBuffer(PGameDXGIRender pGameDXGI, UINT Width, UINT Height) {

	auto pSwapChain = pGameDXGI->pDXGISwapChain;
	auto swapChainFlags = pGameDXGI->DXGISwapChainDesc.Flags;

	UINT pRes[2] = { Width, Height };
	streamline_SetFeature(-1, pRes);

	auto pSL = g_StreamLineProcessor;
	if (!pSL || pSL->m_status != D3D::StreamLineProcessorStatus_t::eD3D12) {
		return pSwapChain->ResizeBuffers(1, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, swapChainFlags);
	}

	pSL->D3D12WaitFinish();

	UINT creationNodeMask[D3D12WrappedBackBuffersCount] = { 1, 1, 1 };
	auto queue = pSL->m_commandQueue;
	IUnknown* presentQueues[D3D12WrappedBackBuffersCount] = { queue, queue, queue };

	auto hr = pSL->m_dxgiSwapChain3->ResizeBuffers1(D3D12WrappedBackBuffersCount, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, swapChainFlags, creationNodeMask, presentQueues);

	if (SUCCEEDED(hr))
		pSL->D3D12GetBuffer();

	return hr;
}

void __fastcall streamline_Release(IDXGISwapChain* pDXGISwapChain) {
	using namespace D3D;
	auto pPP = g_AddPostProcess;
	auto pSL = g_StreamLineProcessor;

	if (pPP) {
		PslVulkanAPI deviceVK = 0;
		if (pSL) deviceVK = pSL->vk;

		pPP->Buffer_Release(deviceVK);
		_aligned_free(pPP);
		g_AddPostProcess = nullptr;
	}

	if (pSL) {
		pSL->Release();
		_aligned_free(pSL);
		g_StreamLineProcessor = nullptr;
	}

	if (pDXGISwapChain) {
		pDXGISwapChain->Release();
	}
}

void __fastcall streamline_Reset() {
	auto pSL = g_StreamLineProcessor;
	if (pSL) {
		pSL->Reset();
	}
}

void __fastcall streamline_SetFeature(int playerCount, UINT* pRes) {
	using namespace D3D;
	auto pPP = g_AddPostProcess;
	if (!pPP) return;

	if (playerCount != -1) {
		if (!pPP->m_bNeedReload && pPP->m_playerCount == playerCount) return;
		pPP->m_playerCount = playerCount;
	} else {
		playerCount = pPP->m_playerCount;
	}

	pPP->m_bNeedReload = false;

	auto pSL = g_StreamLineProcessor;
	ID3D12Device* device12 = 0;
	PslVulkanAPI deviceVK = 0;
	if (pSL) {
		if (pSL->m_status == StreamLineProcessorStatus_t::eD3D12)
			device12 = pSL->m_d3d12Device;
		else if (pSL->m_status == StreamLineProcessorStatus_t::eVULKAN)
			deviceVK = pSL->vk;
	}

	AddPostProcessRes_t res;
	auto pDXGI = DXGI_GetGameDXGIRender();
	if (pRes) {
		pPP->m_fullResolution.Width = pRes[0];
		pPP->m_fullResolution.Height = pRes[1];
	} else {
		// this always returns a resolution of 16:9.
		/*
		auto pTexBack = pDXGI->pTextureBackBuffer;
		auto pBuffer = pTexBack->pInfo;

		res_full.Width = pBuffer->width;
		res_full.Height = pBuffer->height;*/
		pPP->m_fullResolution.Width = pDXGI->DXGISwapChainDesc.BufferDesc.Width;
		pPP->m_fullResolution.Height = pDXGI->DXGISwapChainDesc.BufferDesc.Height;
	}

	res = pPP->m_fullResolution;
	if (playerCount == 2) {
		res.Width /= 2;
	}

	UINT DLSS_Level = 0;
	if (pSL){
		if (pSL->m_renderAPI > 1) {
			DLSS_Level = 2;
		} else {
			DLSS_Level = 1;
		}

		pSL->SetFeature(pDXGI->pD3D11DeviceContext, res.Width, res.Height, playerCount);

		res.Width = pSL->m_inputResolution[0];
		res.Height = pSL->m_inputResolution[1];
	}

	// Because we must first obtain DLSS input resolution
	pPP->m_playerResolution = res;
	pPP->Buffer_Release(deviceVK);
	pPP->Buffer_Create(pDXGI->pD3D11Device, device12, DLSS_Level);
	pPP->Buffer_CreateFull(pDXGI->pD3D11Device, device12, DLSS_Level);
	pPP->Buffer_VKCreate(deviceVK);
	// end
}

void* __fastcall streamline_ExecuteSR(Pg_D3D11DeviceInfo pD3D, int OutOffset, void* saveRAX) {
	using namespace D3D;
	auto pPP = g_AddPostProcess;
	auto pSL = g_StreamLineProcessor;
	if (!pPP) return saveRAX;
	if (OutOffset) {
		if (pPP->m_DrawDone) {
			return 0;
		} else {
			return saveRAX;
		}
	}

	pPP->m_DrawDone = 0;
	auto pPlayer = XGS_GetSystemPlayerPointer(XGS_GetXGSSystemPointer(), 0);
	auto pCamera = pPlayer->pCamera;
	if (pCamera) {
		auto pGameObject = (int*)(pCamera->pGameObject);
		int currentTime = pGameObject[0x310 / 4]; // it's a float second, but we use int to check if it has changed.
		if (currentTime != pPP->m_currentTime) {
			pPP->m_currentTime = currentTime;
		} else {
			return saveRAX;
		}
		// end
	} else { return saveRAX; }


	int hasDLSS = 0;
	ID3D11UnorderedAccessView** outColor = 0;
	if (pSL) {
		if (pSL->m_srFeature && g_bPostProcess[1]) hasDLSS = 1;

		if (pSL->g_gameSceneResourceUAV) outColor = &pSL->g_gameSceneResourceUAV;
	}

	auto playerCount = pPP->m_playerCount;

	auto pSys = XGS_GetXGSSystemPointer();
	PXGS_System_Player player[2];
	player[0] = &pSys->player[0];
	player[1] = &pSys->player[1];

	AddPostProcessRes_t threadGroupCount;
	threadGroupCount.Width = (pPP->m_playerResolution.Width + 15) / 16;
	threadGroupCount.Height = (pPP->m_playerResolution.Height + 15) / 16;

	ID3D11DeviceContext* context = pD3D->context;

	for (int i = 0; i < playerCount; i++) {
		auto pColorRes = (Pg_D3D_ResourceInfo)player[i]->pDrawColorInfo;
		auto pDSVInfo = (Pg_D3D_ResourceInfo)player[i]->pDSVInfo;

		if (g_bPostProcess[0]) {
			pPP->Execute(pD3D, i, threadGroupCount, &pColorRes->pSRV, &pDSVInfo->pSRV);
		} else if (hasDLSS) {
			context->CopyResource(pPP->m_ColorRes[i].d11, pColorRes->pTexture);
			context->CopyResource(pPP->m_DepthRes[i].d11, pDSVInfo->pTexture);
		} else { return saveRAX; }
	}

	if (!hasDLSS) {
		if (g_bPostProcess[0]) {

			pPP->ToRenderBuffer(pD3D, playerCount, outColor, 0);
			return 0;
		}

		return saveRAX;
	}

	// compute motion vector
	// Needs to be repositioned edf.dll+1130A30
	//bool isClearMV = (pSL->m_srReset || playerCount != 1);
	//pPP->CalculateMV(pD3D, player[0], threadGroupCount, isClearMV);

	int srReset = pSL->m_srReset;
	pSL->GetJitter(playerCount);

	// execute DLSS
	if (pSL->m_status == StreamLineProcessorStatus_t::eD3D11) {
		NVSDK_NGX_D3D11_DLSS_Eval_Params D3D11DlssEvalParams;
		memset(&D3D11DlssEvalParams, 0, sizeof(D3D11DlssEvalParams));

		D3D11DlssEvalParams.InReset = srReset;

		for (int i = 0; i < playerCount; i++) {
			pSL->EvaluateSR(context, pPP, player[i]->pRTV, D3D11DlssEvalParams, i);
		}
		// end
	} else if (pSL->m_status == StreamLineProcessorStatus_t::eD3D12) {
		/**/
		NVSDK_NGX_D3D12_DLSS_Eval_Params D3D12DlssEvalParams;
		memset(&D3D12DlssEvalParams, 0, sizeof(D3D12DlssEvalParams));
		D3D12DlssEvalParams.InReset = srReset;

		pSL->D3D12CommandBegin();
		for (int i = 0; i < playerCount; i++) {
			pSL->EvaluateSR(pPP, D3D12DlssEvalParams, i);
		}
		pSL->D3D12CommandEnd();

		pSL->D3D12WaitFinish();
		// end
	}
	else if (pSL->m_status == StreamLineProcessorStatus_t::eVULKAN) {
		NVSDK_NGX_VK_DLSS_Eval_Params D3DvkDlssEvalParams;
		memset(&D3DvkDlssEvalParams, 0, sizeof(D3DvkDlssEvalParams));

		D3DvkDlssEvalParams.InReset = srReset;

		pSL->vk->VK_Enter();
		for (int i = 0; i < playerCount; i++) {
			pSL->EvaluateSR(pPP, D3DvkDlssEvalParams, i);
		}
		pSL->vk->VK_Leave(VK_NULL_HANDLE);
		// end
	}

	pPP->ToRenderBuffer(pD3D, playerCount, outColor, 0x190);
	return 0;
}

void __fastcall streamline_SwitchStatus(int index) {
	switch (index) {
	case 0: {
		if (!Config_PostProcess) return;

		g_bPostProcess[0] = !g_bPostProcess[0];

		if (!g_bPostProcess[0]) {
			auto index = Config_PostProcessTexIndex;
			//if (index > 3) index = 0;

			auto pDXGI = DXGI_GetGameDXGIRender();
			g_AddPostProcess->m_LUTindex = index;
			g_AddPostProcess->LUTBuffer_Reload(pDXGI->pD3D11Device);
		}
		return;
	}
	case 1:
		if (!Config_DLSS) return;

		g_bPostProcess[1] = !g_bPostProcess[1];
		return;
	case 2:
		if (!Config_DLSSFG) return;

		g_bPostProcess[2] = !g_bPostProcess[2];

		if (g_StreamLineProcessor) {

			g_StreamLineProcessor->m_multiFrameCount = 1;
			g_StreamLineProcessor->m_bNeedFG = g_bPostProcess[2];
		}

		return;
	case 3: {
		g_AddPostProcess->m_LUTindex++;
		auto pDXGI = DXGI_GetGameDXGIRender();
		g_AddPostProcess->LUTBuffer_Reload(pDXGI->pD3D11Device);
		return;
	}
	case 4: {
		if (!g_StreamLineProcessor) return;

		auto mfg = g_StreamLineProcessor->m_multiFrameCount + 1;
		if (mfg > DLSSMaxMultiFrameCount) mfg = 1;

		g_StreamLineProcessor->m_multiFrameCount = mfg;
		return;
	}
	case 5: {
		if (!g_StreamLineProcessor) return;
		if (!g_AddPostProcess) return;

		UINT dlssMode = Config_DLSS + 1;
		if (dlssMode > 5) dlssMode = 1;

		Config_DLSS = dlssMode;
		g_AddPostProcess->m_bNeedReload = true;

		return;
	}
	default: return;
	}
	// end
}
